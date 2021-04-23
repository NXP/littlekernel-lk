// Copyright 2016 The Fuchsia Authors
//
// Use of this source code is governed by a MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT

#pragma once

#include <compiler.h>
#include <sys/types.h>
#include <kernel/event.h>
#include <kernel/mutex.h>
#include <list.h>
#include <stdint.h>

__BEGIN_CDECLS

typedef struct dlog dlog_t;
typedef struct dlog_header dlog_header_t;
typedef struct dlog_record dlog_record_t;
typedef struct dlog_reader dlog_reader_t;

struct dlog {
    spin_lock_t lock;

    size_t head;
    size_t tail;

    uint8_t* data;

    bool panic;

    event_t event;

    mutex_t readers_lock;
    struct list_node readers;
};

struct dlog_reader {
    struct list_node node;

    dlog_t* log;
    size_t tail;

    void (*notify)(void* cookie);
    void *cookie;
};

#define DLOG_HDR_SET(fifosize, readsize) \
    ((((readsize) & 0xFFF) << 12) | ((fifosize) & 0xFFF))

#define DLOG_HDR_GET_FIFOLEN(n)   ((n) & 0xFFF)
#define DLOG_HDR_GET_READLEN(n)  (((n) >> 12) & 0xFFF)

#define DLOG_MIN_RECORD          (32u)
#define DLOG_MAX_DATA            (224u)
#define DLOG_MAX_RECORD          (DLOG_MIN_RECORD + DLOG_MAX_DATA)

struct dlog_header {
    uint32_t header;
    uint16_t datalen;
    uint16_t flags;
    uint64_t timestamp;
    uint64_t tid;
};

struct dlog_record {
    dlog_header_t hdr;
    char data[DLOG_MAX_DATA];
};

void dlog_reader_init(dlog_reader_t* rdr, void (*notify)(void*), void* cookie);
void dlog_reader_destroy(dlog_reader_t* rdr);
status_t dlog_write(uint32_t flags, const void* ptr, size_t len);
status_t dlog_read(dlog_reader_t* rdr, uint32_t flags, void* ptr, size_t len, size_t* actual);

void dlog_bypass_init_early(void);
void dlog_bypass_init(void);
bool dlog_bypass(void);

__END_CDECLS
