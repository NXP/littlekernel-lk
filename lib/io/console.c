/*
 * Copyright (c) 2008-2015 Travis Geiselbrecht
 * Copyright 2018-2020 NXP
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include <lib/io.h>

#include <err.h>
#include <ctype.h>
#include <debug.h>
#include <assert.h>
#include <list.h>
#include <string.h>
#include <lib/cbuf.h>
#include <arch/ops.h>
#include <platform.h>
#include <platform/debug.h>
#include <kernel/thread.h>
#include <kernel/vm.h>
#include <lk/init.h>

#if WITH_LIB_DEBUGLOG
#include <lib/debuglog.h>
#endif

/* routines for dealing with main console io */

#if WITH_LIB_SM
#define PRINT_LOCK_FLAGS SPIN_LOCK_FLAG_IRQ_FIQ
#else
#define PRINT_LOCK_FLAGS SPIN_LOCK_FLAG_INTERRUPTS
#endif

static spin_lock_t print_spin_lock;
static struct list_node print_callbacks = LIST_INITIAL_VALUE(print_callbacks);

#if CONSOLE_HAS_INPUT_BUFFER
#ifndef CONSOLE_BUF_LEN
#define CONSOLE_BUF_LEN 256
#endif

/* global input circular buffer */
cbuf_t console_input_cbuf;
static uint8_t console_cbuf_buf[CONSOLE_BUF_LEN];
#endif // CONSOLE_HAS_INPUT_BUFFER

void __kernel_console_write(const char* str, size_t len)
{
    print_callback_t *cb;

    /* print to any registered loggers */
    if (!list_is_empty(&print_callbacks)) {
        spin_lock_saved_state_t state;
        spin_lock_save(&print_spin_lock, &state, PRINT_LOCK_FLAGS);

        list_for_every_entry(&print_callbacks, cb, print_callback_t, entry) {
            if (cb->print)
                cb->print(cb, str, len);
        }

        spin_unlock_restore(&print_spin_lock, state, PRINT_LOCK_FLAGS);
    }
}


void __kernel_serial_write(const char* str, size_t len)
{
#if WITH_LIB_DEBUGLOG
    static spin_lock_t dputc_spin_lock;

    arch_spin_lock_init(&dputc_spin_lock);

    spin_lock_saved_state_t state;
    spin_lock_irqsave(&dputc_spin_lock, state);

    /* write out the serial port */
    platform_dputs_irq(str, len);

    spin_unlock_irqrestore(&dputc_spin_lock, state);
#else
    platform_dputs(str, len);
#endif
}

void register_print_callback(print_callback_t *cb)
{
    spin_lock_saved_state_t state;
    spin_lock_save(&print_spin_lock, &state, PRINT_LOCK_FLAGS);

    list_add_head(&print_callbacks, &cb->entry);

    spin_unlock_restore(&print_spin_lock, state, PRINT_LOCK_FLAGS);
}

void unregister_print_callback(print_callback_t *cb)
{
    spin_lock_saved_state_t state;
    spin_lock_save(&print_spin_lock, &state, PRINT_LOCK_FLAGS);

    list_delete(&cb->entry);

    spin_unlock_restore(&print_spin_lock, state, PRINT_LOCK_FLAGS);
}

static void __kernel_stdout_write(const char *s, size_t len)
{

#if WITH_LIB_DEBUGLOG
    if (dlog_bypass() == false) {
        if (dlog_write(0, s, len) == 0)
            return;
    }
#endif

    __kernel_console_write(s, len);
    __kernel_serial_write(s, len);

}

#if WITH_DEBUG_LINEBUFFER
static struct s_linebuffer {
    char buf[THREAD_LINEBUFFER_LENGTH];
    size_t pos;
} core_linebuffer[SMP_MAX_CPUS];


static void __kernel_stdout_write_buffered(const char* str, size_t len)
{
    thread_t* t = get_current_thread();

    char* buf;
    size_t pos;
    size_t *ppos;

    if (unlikely(t == NULL)) {
        unsigned core = arch_curr_cpu_num();
        buf = core_linebuffer[core].buf;
        pos = core_linebuffer[core].pos;
        ppos = &core_linebuffer[core].pos;
    } else {
        buf = t->linebuffer;
        pos = t->linebuffer_pos;
        ppos = &t->linebuffer_pos;
    }

    // look for corruption and don't continue
    if (unlikely(!is_kernel_address((uintptr_t)buf)) || (pos >= (THREAD_LINEBUFFER_LENGTH - 1))) {
        const char* str = "<linebuffer corruption>\n";
        __kernel_stdout_write(str, strlen(str));
        return;
    }

    while (len-- > 0) {
        char c = *str++;
        buf[pos++] = c;
        if (c == '\n') {
            __kernel_stdout_write(buf, pos);
            pos = 0;
            continue;
        }
        if (pos == (THREAD_LINEBUFFER_LENGTH - 1)) {
            buf[pos++] = '\n';
            __kernel_stdout_write(buf, pos);
            pos = 0;
            continue;
        }
    }
    *ppos = pos;
}
#endif

static ssize_t __debug_stdio_write(io_handle_t *io, const char *s, size_t len)
{
#if WITH_DEBUG_LINEBUFFER
    __kernel_stdout_write_buffered(s, len);
#else
    __kernel_stdout_write(s, len);
#endif
    return len;
}

static ssize_t __debug_stdio_read(io_handle_t *io, char *s, size_t len)
{
    if (len == 0)
        return 0;

#if CONSOLE_HAS_INPUT_BUFFER
    ssize_t err = cbuf_read(&console_input_cbuf, s, len, true);
    return err;
#else
    int err = platform_dgetc(s, true);
    if (err < 0)
        return err;

    return 1;
#endif
}

#if CONSOLE_HAS_INPUT_BUFFER
void console_init_hook(uint level)
{
    arch_spin_lock_init(&print_spin_lock);
    cbuf_initialize_etc(&console_input_cbuf, sizeof(console_cbuf_buf), console_cbuf_buf);
}

LK_INIT_HOOK(console, console_init_hook, LK_INIT_LEVEL_PLATFORM_EARLY - 1);
#endif

/* global console io handle */
static const io_handle_hooks_t console_io_hooks = {
    .write  = __debug_stdio_write,
    .read   = __debug_stdio_read,
};

io_handle_t console_io = IO_HANDLE_INITIAL_VALUE(&console_io_hooks);

