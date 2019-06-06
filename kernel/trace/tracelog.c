/*
 * Copyright 2019 - NXP
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

#include <err.h>
#include <lk/init.h>
#include <kernel/timer.h>
#include <platform.h>
#include <string.h>
#include <stdlib.h>

#include <lib/cbuf.h>

#include <kernel/trace/tracelog.h>
#include <kernel/trace/tracelog_str.h>
#include <kernel/trace/tracelog_bin.h>
#include <kernel/trace/tracelog_kernel.h>

#include <ivshmem-endpoint.h>
#include <cipc.h>
#include <trace.h>

//#define CHECK_PAYLOAD
#define LOCAL_TRACE 0

#ifdef CHECK_PAYLOAD
#define CBUF_SIZE   (1 << 20)
#else
#define CBUF_SIZE   (1 << 23)
#endif

#define CPUS_NUM    (3)

static struct {
    cbuf_t cbuf;
    uint8_t store[CBUF_SIZE];
    spin_lock_t lock;
} ring[CPUS_NUM];

static struct tracelog_hooks hooks[] = {
    [TRACELOG_TYPE_STR]     = { tracelog_str_print, tracelog_str_store, NULL },
    [TRACELOG_TYPE_KERNEL]  = { tracelog_kernel_print, tracelog_kernel_store, tracelog_kernel_no_ipc_trace },
    [TRACELOG_TYPE_BINARY]  = { tracelog_bin_print, tracelog_bin_store, NULL },
};

static uint8_t flush_store[CBUF_SIZE];

static void tracelog_write_cbuf(struct tracelog_entry_header *header)
{
    struct tracelog_entry_header h;
    unsigned int cpu;
    size_t len;

    cpu = header->cpu_id;

    if (thread_lock_held() && spin_lock_held(&ring[cpu].lock))
        return;

    len = sizeof(struct tracelog_entry_header) + header->len;

    spin_lock_saved_state_t state;
    spin_lock_irqsave(&ring[cpu].lock, state);

    while (cbuf_space_avail(&ring[cpu].cbuf) < len) {
        cbuf_read(&ring[cpu].cbuf, &h, sizeof(struct tracelog_entry_header), 0);
        if (h.len)
            cbuf_read(&ring[cpu].cbuf, NULL, h.len, 0);
    }
    cbuf_write(&ring[cpu].cbuf, header, len, 0);

    spin_unlock_irqrestore(&ring[cpu].lock, state);
}

void tracelog_write(unsigned int type, void *arg0, void *arg1)
{
    char buf[TRACELOG_ENTRY_MAX_SIZE] = { 0 };
    struct tracelog_entry_header *header;
    unsigned int t;

    t = TRACELOG_TYPE(type);

    if (t >= TRACELOG_TYPE_NUM)
        return;

    header = (struct tracelog_entry_header *) buf;

    header->magic = TRACELOG_MAGIC;
    header->type = (uint8_t) type;
    header->cpu_id = (uint8_t) arch_curr_cpu_num();
    header->timestamp = current_time_hires();

    if (hooks[t].no_trace && hooks[t].no_trace(header, arg0, arg1))
        return;

    hooks[t].store(header, arg0, arg1);

    tracelog_write_cbuf(header);
}

void tracelog_flush(void)
{
    unsigned int cpu;
    size_t count;

    for (cpu = 0; cpu < CPUS_NUM; cpu++) {
        count = cbuf_read(&ring[cpu].cbuf, flush_store, CBUF_SIZE, 0);
        cipc_write_buf(IVSHM_EP_ID_LKTRACES, flush_store, count);

        LTRACEF("cpu %d: Send %ld bytes\n", cpu, count);
#ifdef CHECK_PAYLOAD
        {
            size_t off = 0;
            struct tracelog_entry_header *header;

            printf("Sent %ld bytes\n", count);

            while (off < count) {
                header = (struct tracelog_entry_header *) &flush_store[off];
                if (header->magic != TRACELOG_MAGIC)
                    printf("Wrong magic!\n");
                off += sizeof(struct tracelog_entry_header) + header->len;
            }
        }
#endif
    }
}

static int tracelog_flush_thread(void *arg)
{
    while (1) {
        thread_sleep(50);
        tracelog_flush();
    }

    return 0;
}

static void tracelog_init(uint level)
{
    unsigned int cpu;

    for (cpu = 0; cpu < CPUS_NUM; cpu++) {
        cbuf_initialize_etc(&ring[cpu].cbuf, CBUF_SIZE, ring[cpu].store);
        ring[cpu].cbuf.no_event = true;
    }
}
LK_INIT_HOOK(trace_tracelog, &tracelog_init, LK_INIT_LEVEL_KERNEL);

#if WITH_LIB_CONSOLE

#include <lib/console.h>

static void cmd_do_flush(void)
{
    tracelog_flush();
}

static void cmd_do_start_flush(void)
{
    thread_t *t;

    t = thread_create("tracelog-flush",
                      &tracelog_flush_thread,
                      NULL,
                      LOW_PRIORITY,
                      DEFAULT_STACK_SIZE);
    thread_detach_and_resume(t);
}

static void cmd_do_list(void)
{
    struct tracelog_entry_header *header;
    unsigned int t, cpu;
    iovec_t regions[2];
    size_t count, off;

    for (cpu = 0; cpu < CPUS_NUM; cpu++) {
        spin_lock_saved_state_t state;
        spin_lock_irqsave(&ring[cpu].lock, state);

        count = cbuf_peek(&ring[cpu].cbuf, regions);
        iovec_to_membuf(flush_store, count, regions, 2, 0);

        spin_unlock_irqrestore(&ring[cpu].lock, state);

        for (off = 0; off < count; off += sizeof(struct tracelog_entry_header) + header->len) {
            header = (struct tracelog_entry_header *) &flush_store[off];

            if (header->magic != TRACELOG_MAGIC) {
                printf("Wrong magic!\n");
                return;
            }

            t = TRACELOG_TYPE(header->type);

            if (t >= TRACELOG_TYPE_NUM)
                continue;

            printf("[ %llu.%d | type: %d | subtype: %d ]: ", header->timestamp, header->cpu_id,
                    TRACELOG_TYPE(header->type), TRACELOG_SUBTYPE(header->type));
            hooks[t].print(header, &flush_store[off + sizeof(struct tracelog_entry_header)]);
        }
    }
}

static int cmd_tracelog(int argc, const cmd_args *argv)
{
    if (argc < 2) {
usage:
        printf("%s list: view tracepoint events list\n", argv[0].str);
        printf("%s start_flush: start flush thread\n", argv[0].str);
        printf("%s flush: Flush tracelog buffer\n", argv[0].str);

        return ERR_GENERIC;
    }

    if (!strcmp(argv[1].str, "list")) {
        cmd_do_list();
    } else if (!strcmp(argv[1].str, "start_flush")) {
        cmd_do_start_flush();
    } else if (!strcmp(argv[1].str, "flush")) {
        cmd_do_flush();
    } else {
        printf("Command unknown\n");
        goto usage;
    }

    return NO_ERROR;
}

STATIC_COMMAND_START
STATIC_COMMAND("tracelog", "commands for manipulating tracepoints log", &cmd_tracelog)
STATIC_COMMAND_END(tracelog);

#endif // WITH_LIB_CONSOLE

