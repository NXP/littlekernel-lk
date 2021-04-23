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

#ifndef __TRACE_TRACELOG_H
#define __TRACE_TRACELOG_H

#include <sys/types.h>

__BEGIN_CDECLS

#define TRACELOG_MAGIC              0xDEADBEEF

#define TRACELOG_SET_TYPE(t, s)     (((t) & 0xf) | (((s) & 0xf) << 4))
#define TRACELOG_TYPE(t)            ((t) & 0xf)
#define TRACELOG_SUBTYPE(s)         (((s) >> 4) & 0xf)

enum tracelog_entry_type {
    TRACELOG_TYPE_STR,
    TRACELOG_TYPE_KERNEL,
    TRACELOG_TYPE_BINARY,
    TRACELOG_TYPE_NUM,
};

struct tracelog_entry_header {
    uint32_t magic;
    lk_bigtime_t timestamp;
    uint8_t type;
    uint8_t cpu_id;
    uint16_t len;
    char data[0];
} __PACKED;

#define TRACELOG_ENTRY_MAX_SIZE     (256)
#define TRACELOG_DATA_MAX_SIZE      (TRACELOG_ENTRY_MAX_SIZE - sizeof(struct tracelog_entry_header) + 1)

struct tracelog_hooks {
    void (*print)(struct tracelog_entry_header *header, void *buf);
    void (*store)(struct tracelog_entry_header *header, void *arg0, void *arg1);
    bool (*no_trace)(struct tracelog_entry_header *header, void *arg0, void *arg1);
};

#if WITH_KERNEL_TRACEPOINT

extern void tracelog_write(unsigned int type, void *arg0, void *arg1);

#else // !WITH_KERNEL_TRACEPOINT

static inline void tracelog_write(unsigned int type, void *arg0, void *arg1) { }

#endif

__END_CDECLS

#endif
