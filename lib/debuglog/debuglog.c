// Copyright 2016 The Fuchsia Authors
// Copyright 2018 NXP
//
// Use of this source code is governed by a MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT

#include <kernel/spinlock.h>
#include <kernel/thread.h>

#include <lib/debuglog.h>

#include <err.h>
#include <lib/io.h>
#include <lib/version.h>
#include <lk/init.h>
#include <platform.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#ifndef CASCFG_DLOG_SIZE_IN_KB
#define DLOG_SIZE (128u * 1024u)
#else
#define DLOG_SIZE (CASCFG_DLOG_SIZE_IN_KB * 1024u)
#endif

#define DLOG_MASK (DLOG_SIZE - 1u)

static uint8_t DLOG_DATA[DLOG_SIZE];

static dlog_t DLOG = {
    .lock = SPIN_LOCK_INITIAL_VALUE,
    .head = 0,
    .tail = 0,
    .data = DLOG_DATA,
    .panic = false,
    .event = EVENT_INITIAL_VALUE(DLOG.event, 0, EVENT_FLAG_AUTOUNSIGNAL),

    .readers_lock = MUTEX_INITIAL_VALUE(DLOG.readers_lock),
    .readers = LIST_INITIAL_VALUE(DLOG.readers),
};

static thread_t* notifier_thread;
static thread_t* dumper_thread;

// Used to request that notifier and dumper threads terminate.

// dlog_bypass_ will directly write to console. It also has the side effect of
// disabling uart Tx interrupts. So all serial console writes are polling.
static bool dlog_bypass_ = false;

// We need to preserve the compile time switch (ENABLE_KERNEL_LL_DEBUG), even
// though we add a kernel cmdline (kernel.bypass-debuglog), to bypass the debuglog.
// This is to allow very early prints in the kernel to go to the serial console.

// Called first thing in init, so very early printfs can go to serial console.
void dlog_bypass_init_early(void) {
#if ((defined ENABLE_KERNEL_LL_DEBUG) && (ENABLE_KERNEL_LL_DEBUG))
    dlog_bypass_ = true;
#endif
}

bool dlog_bypass(void) {
    return dlog_bypass_;
}

void dlog_panic(void)
{
    DLOG.panic = true;
    dlog_bypass_ = true;
}

// The debug log maintains a circular buffer of debug log records,
// consisting of a common header (dlog_header_t) followed by up
// to 224 bytes of textual log message.  Records are aligned on
// uint32_t boundaries, so the header word which indicates the
// true size of the record and the space it takes in the fifo
// can always be read with a single uint32_t* read (the header
// or body may wrap but the initial header word never does).
//
// The ring buffer position is maintained by continuously incrementing
// head and tail pointers (type size_t, so uint64_t on 64bit systems),
//
// This allows readers to trivial compute if their local tail
// pointer has "fallen out" of the fifo (an entire fifo's worth
// of messages were written since they last tried to read) and then
// they can snap their tail to the global tail and restart
//
//
// Tail indicates the oldest message in the debug log to read
// from, Head indicates the next space in the debug log to write
// a new message to.  They are clipped to the actual buffer by
// DLOG_MASK.
//
//       T                     T
//  [....XXXX....]  [XX........XX]
//           H         H

#define ALIGN4(n) (((n) + 3) & (~3))

status_t dlog_write(uint32_t flags, const void* data_ptr, size_t len) {
    const uint8_t* ptr = (const uint8_t*)data_ptr;
    dlog_t* log = &DLOG;

    if (len > DLOG_MAX_DATA) {
        return ERR_OUT_OF_RANGE;
    }

    if (log->panic) {
        return ERR_BAD_STATE;
    }

    // Our size "on the wire" must be a multiple of 4, so we know
    // that worst case there will be room for a header skipping
    // the last n bytes when the fifo wraps
    size_t wiresize = DLOG_MIN_RECORD + ALIGN4(len);

    // Prepare the record header before taking the lock
    dlog_header_t hdr;
    hdr.header = (uint32_t)(DLOG_HDR_SET(wiresize, DLOG_MIN_RECORD + len));
    hdr.datalen = (uint16_t)(len);
    hdr.flags = (uint16_t)(flags);
    hdr.timestamp = current_time_hires();
    thread_t* t = get_current_thread();
    if (t) {
        hdr.tid = (uint64_t) t;
    } else {
        hdr.tid = 0;
    }

    spin_lock_saved_state_t state;
    spin_lock_irqsave(&log->lock, state);

    // Discard records at tail until there is enough
    // space for the new record.
    while ((log->head - log->tail) > (DLOG_SIZE - wiresize)) {
        uint32_t header = *(uint32_t*)(log->data + (log->tail & DLOG_MASK));
        log->tail += DLOG_HDR_GET_FIFOLEN(header);
    }

    size_t offset = (log->head & DLOG_MASK);

    size_t fifospace = DLOG_SIZE - offset;

    if (fifospace >= wiresize) {
        // everything fits in one write, simple case!
        memcpy(log->data + offset, &hdr, sizeof(hdr));
        memcpy(log->data + offset + sizeof(hdr), ptr, len);
    } else if (fifospace < sizeof(hdr)) {
        // the wrap happens in the header
        memcpy(log->data + offset, &hdr, fifospace);
        memcpy(log->data, (uint8_t*)(&hdr) + fifospace, sizeof(hdr) - fifospace);
        memcpy(log->data + (sizeof(hdr) - fifospace), ptr, len);
    } else {
        // the wrap happens in the data
        memcpy(log->data + offset, &hdr, sizeof(hdr));
        offset += sizeof(hdr);
        fifospace -= sizeof(hdr);
        memcpy(log->data + offset, ptr, fifospace);
        memcpy(log->data, ptr + fifospace, len - fifospace);
    }
    log->head += wiresize;

    spin_unlock_irqrestore(&log->lock, state);

    event_signal(&log->event, false);

    return 0;
}

// TODO: support reading multiple messages at a time
// TODO: filter with flags
status_t dlog_read(dlog_reader_t* rdr, uint32_t flags, void* data_ptr,
                      size_t len, size_t* _actual) {
    uint8_t* ptr = (uint8_t*)(data_ptr);
    // must be room for worst-case read
    if (len < DLOG_MAX_RECORD) {
        return ERR_NOT_ENOUGH_BUFFER;
    }

    dlog_t* log = rdr->log;
    status_t status = ERR_BUSY;

    spin_lock_saved_state_t state;
    spin_lock_irqsave(&log->lock, state);

    size_t rtail = rdr->tail;

    // If the read-tail is not within the range of log-tail..log-head
    // this reader has been lapped by a writer and we reset our read-tail
    // to the current log-tail.
    //
    if ((log->head - log->tail) < (log->head - rtail)) {
        rtail = log->tail;
    }

    if (rtail != log->head) {
        size_t offset = (rtail & DLOG_MASK);
        uint32_t header = *(uint32_t*)(log->data + offset);

        size_t actual = DLOG_HDR_GET_READLEN(header);
        size_t fifospace = DLOG_SIZE - offset;

        if (fifospace >= actual) {
            memcpy(ptr, log->data + offset, actual);
        } else {
            memcpy(ptr, log->data + offset, fifospace);
            memcpy(ptr + fifospace, log->data, actual - fifospace);
        }

        *_actual = actual;
        status = 0;

        rtail += DLOG_HDR_GET_FIFOLEN(header);
    }

    rdr->tail = rtail;

    spin_unlock_irqrestore(&log->lock, state);

    return status;
}

void dlog_reader_init(dlog_reader_t* rdr, void (*notify)(void*), void* cookie) {
    dlog_t* log = &DLOG;

    rdr->log = log;
    rdr->notify = notify;
    rdr->cookie = cookie;

    mutex_acquire(&log->readers_lock);
    list_add_tail(&log->readers, &rdr->node);

    bool do_notify = false;

    spin_lock_saved_state_t state;
    spin_lock_irqsave(&log->lock, state);
    rdr->tail = log->tail;
    do_notify = (log->tail != log->head);
    spin_unlock_irqrestore(&log->lock, state);

    // simulate notify callback for events that arrived
    // before we were initialized
    if (do_notify && notify) {
        notify(cookie);
    }

    mutex_release(&log->readers_lock);
}

void dlog_reader_destroy(dlog_reader_t* rdr) {
    dlog_t* log = rdr->log;

    mutex_acquire(&log->readers_lock);
    list_delete(&rdr->node);
    mutex_release(&log->readers_lock);
}

// The debuglog notifier thread observes when the debuglog is
// written and calls the notify callback on any readers that
// have one so they can process new log messages.
static int debuglog_notifier(void* arg) {
    dlog_t* log = &DLOG;

    for (;;) {
        event_wait(&log->event);

        // notify readers that new log items were posted
        mutex_acquire(&log->readers_lock);
        dlog_reader_t* rdr;
        list_for_every_entry (&log->readers, rdr, dlog_reader_t, node) {
            if (rdr->notify) {
                rdr->notify(rdr->cookie);
            }
        }
        mutex_release(&log->readers_lock);
    }
    return 0;
}

// Common bottleneck between sys_debug_write() and debuglog_dumper()
// to reduce interleaved messages between the serial console and the
// debuglog drainer.
static mutex_t lock = MUTEX_INITIAL_VALUE(lock);
void dlog_serial_write(const char* data, size_t len) {
    if (dlog_bypass_ == true) {
        // If LL DEBUG is enabled we take this path which uses a spinlock
        // and prevents the direct writes from the kernel from interleaving
        // with our output
        __kernel_serial_write(data, len);
    } else {
        // Otherwise we can use a mutex and avoid time under spinlock
        mutex_acquire(&lock);
        platform_dputs_thread(data, len);
        mutex_release(&lock);
    }
}

// The debuglog dumper thread creates a reader to observe
// debuglog writes and dump them to the kernel consoles
// and kernel serial console.
static void debuglog_dumper_notify(void* cookie) {
    event_t* event = (event_t*)(cookie);
    event_signal(event, false);
}

static event_t dumper_event = EVENT_INITIAL_VALUE(dumper_event, 0, EVENT_FLAG_AUTOUNSIGNAL);

static dlog_reader_t reader;
// assembly buffer with room for log text plus header text
static char dlog_buffer[DLOG_MAX_DATA + 128];

static bool dlog_get_item(char **buffer, size_t *len)
{
    size_t actual;
    status_t ret;

    struct {
        dlog_header_t hdr;
        char data[DLOG_MAX_DATA + 1];
    } rec;


    ret = dlog_read(&reader, 0, &rec, DLOG_MAX_RECORD, &actual);
    if (ret != 0)
        return false;

    if (rec.hdr.datalen && (rec.data[rec.hdr.datalen - 1] == '\n')) {
        rec.data[rec.hdr.datalen - 1] = 0;
    } else {
        rec.data[rec.hdr.datalen] = 0;
    }
    int n;
    const char *name = "unknown";
    if (rec.hdr.tid)
        name = ((thread_t*)(rec.hdr.tid))->name;
    n = snprintf(dlog_buffer, sizeof(dlog_buffer), "[%05d.%06d] %s > %s\n",
                 (int)(rec.hdr.timestamp / 1000000ULL),
                 (int)((rec.hdr.timestamp) % 1000000ULL),
                 name, rec.data);
    if (n > (int)sizeof(dlog_buffer)) {
        n = sizeof(dlog_buffer);
    }

    *len = n;
    *buffer = dlog_buffer;

    return true;
}

void dlog_flush(void)
{
    char *tmp;
    size_t n;

    while (true) {
        bool has_data = dlog_get_item(&tmp, &n);
        if (has_data == false)
            break;
        size_t n = strlen(tmp);
        __kernel_console_write(tmp, n);
        dlog_serial_write(tmp, n);
    }
}


static int debuglog_dumper(void* arg) {

    dlog_reader_init(&reader, debuglog_dumper_notify, &dumper_event);

    for (;;) {
        event_wait(&dumper_event);
        dlog_flush();
    }

    return 0;
}

static void dlog_init_hook(uint level) {
    DEBUG_ASSERT(notifier_thread == NULL);
    DEBUG_ASSERT(dumper_thread == NULL);

    if ((notifier_thread = thread_create("debuglog-notifier", debuglog_notifier, NULL,
                                         HIGH_PRIORITY - 1, DEFAULT_STACK_SIZE)) != NULL) {
        thread_resume(notifier_thread);
    }

    if ((dumper_thread = thread_create("debuglog-dumper", debuglog_dumper, NULL,
                                       HIGH_PRIORITY - 2, DEFAULT_STACK_SIZE)) != NULL) {
        thread_resume(dumper_thread);
    }
}

LK_INIT_HOOK(debuglog, dlog_init_hook, LK_INIT_LEVEL_THREADING - 1);
