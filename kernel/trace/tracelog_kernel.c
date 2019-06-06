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

#include <kernel/thread.h>
#include <kernel/debug.h>
#include <stdio.h>
#include <string.h>

#include <kernel/trace/tracelog.h>

#define TID_OF(t) ((uintptr_t)(t) & 0xffffffff)

struct tracelog_kernel_switch {
    uint8_t prev_comm[32];
    uint8_t next_comm[32];
    uint32_t prev_tid;
    uint32_t next_tid;
    uint32_t prev_prio;
    uint32_t next_prio;
} __PACKED;

struct tracelog_kernel_irq {
    uint8_t num;
} __PACKED;

struct tracelog_kernel_preempt {
    uint8_t comm[32];
    uint32_t tid;
    uint32_t prio;
} __PACKED;

struct tracelog_kernel_timer_call {
    uintptr_t callback;
    uintptr_t arg;
} __PACKED;

static void timer_call_print(struct tracelog_entry_header *header, void *buf)
{
    struct tracelog_kernel_timer_call *t_call;

    t_call = (struct tracelog_kernel_timer_call *) buf;

    printf("Timer call callback: %p arg: %p\n", (void *) t_call->callback, (void *) t_call->arg);
}

static void timer_call_store(struct tracelog_entry_header *header, void *arg0, void *arg1)
{
    struct tracelog_kernel_timer_call *t_call;

    t_call = (struct tracelog_kernel_timer_call *) &header->data[0];

    t_call->callback = (uintptr_t) arg0;
    t_call->arg = (uintptr_t) arg1;

    header->len = sizeof(struct tracelog_kernel_timer_call);
}

static void timer_tick_print(struct tracelog_entry_header *header, void *buf)
{
    printf("Timer tick\n");
}

static void timer_tick_store(struct tracelog_entry_header *header, void *arg0, void *arg1)
{
    header->len = 0;
}

static void preempt_print(struct tracelog_entry_header *header, void *buf)
{
    struct tracelog_kernel_preempt *t_preempt;

    t_preempt = (struct tracelog_kernel_preempt *) buf;

    printf("Thread \"%s\" [TID: %d, prio: %d] preempted\n", t_preempt->comm, t_preempt->tid, t_preempt->prio);
}

static void preempt_store(struct tracelog_entry_header *header, void *arg0, void *arg1)
{
    struct tracelog_kernel_preempt *t_preempt;
    thread_t *t0;

    t_preempt = (struct tracelog_kernel_preempt *) &header->data[0];

    t0 = (thread_t *) arg0;

    memcpy(&t_preempt->comm, t0->name, strlen(t0->name) + 1);
    t_preempt->prio = t0->priority;
    t_preempt->tid = TID_OF(t0);

    header->len = sizeof(struct tracelog_kernel_preempt);
}

static void irq_print(struct tracelog_entry_header *header, void *buf)
{
    struct tracelog_kernel_irq *t_irq;
    int is_enter;

    is_enter = (TRACELOG_SUBTYPE(header->type) == KERNEL_EVLOG_IRQ_ENTER);
    t_irq = (struct tracelog_kernel_irq *) buf;

    printf("%s handler IRQ #%d\n", is_enter ? "Enter" : "Exit", t_irq->num);
}

static void irq_store(struct tracelog_entry_header *header, void *arg0, void *arg1)
{
    struct tracelog_kernel_irq *t_irq;

    t_irq = (struct tracelog_kernel_irq *) &header->data[0];

    t_irq->num = (uint8_t) ((uintptr_t) arg0 & 0xff);

    header->len = sizeof(struct tracelog_kernel_irq);
}

static void context_switch_print(struct tracelog_entry_header *header, void *buf)
{
    struct tracelog_kernel_switch *t_switch;

    t_switch = (struct tracelog_kernel_switch *) buf;

    printf("Context switch from \"%s\" [TID: %d, prio: %d] to \"%s\" [TID: %d, prio: %d]\n",
            t_switch->prev_comm, t_switch->prev_tid, t_switch->prev_prio,
            t_switch->next_comm, t_switch->next_tid, t_switch->next_prio);
}

static void context_switch_store(struct tracelog_entry_header *header, void *arg0, void *arg1)
{
    struct tracelog_kernel_switch *t_switch;
    thread_t *t0, *t1;

    t_switch = (struct tracelog_kernel_switch *) &header->data[0];

    t0 = (thread_t *) arg0;
    t1 = (thread_t *) arg1;

    memcpy(&t_switch->prev_comm, t0->name, strlen(t0->name) + 1);
    memcpy(&t_switch->next_comm, t1->name, strlen(t1->name) + 1);

    t_switch->prev_prio = t0->priority;
    t_switch->next_prio = t1->priority;

    t_switch->prev_tid = TID_OF(t0);
    t_switch->next_tid = TID_OF(t1);

    header->len = sizeof(struct tracelog_kernel_switch);
}

#define NO_TRACE_THRD_IVSHMEM   "ivshm-"
#define NO_TRACE_THRD_BINARY    "binary-"
#define NO_TRACE_THRD_TRACELOG  "tracelog-"

static bool no_ipc_trace(thread_t *t)
{
    return (!strncmp(t->name, NO_TRACE_THRD_IVSHMEM, strlen(NO_TRACE_THRD_IVSHMEM)) ||
            !strncmp(t->name, NO_TRACE_THRD_BINARY, strlen(NO_TRACE_THRD_BINARY)) ||
            !strncmp(t->name, NO_TRACE_THRD_TRACELOG, strlen(NO_TRACE_THRD_TRACELOG)));
}

static bool context_switch_no_trace(struct tracelog_entry_header *header, void *arg0, void *arg1)
{
    return no_ipc_trace((thread_t *) arg1);
}

static struct tracelog_hooks hooks[] = {
    [KERNEL_EVLOG_CONTEXT_SWITCH]   = { context_switch_print, context_switch_store, context_switch_no_trace },
    [KERNEL_EVLOG_PREEMPT]          = { preempt_print, preempt_store, NULL },
    [KERNEL_EVLOG_TIMER_TICK]       = { timer_tick_print, timer_tick_store, NULL },
    [KERNEL_EVLOG_TIMER_CALL]       = { timer_call_print, timer_call_store, NULL },
    [KERNEL_EVLOG_IRQ_ENTER]        = { irq_print, irq_store, NULL },
    [KERNEL_EVLOG_IRQ_EXIT]         = { irq_print, irq_store, NULL },
};

bool tracelog_kernel_no_ipc_trace(struct tracelog_entry_header *header, void *arg0, void *arg1)
{
    int t = TRACELOG_SUBTYPE(header->type);
    bool filter_out = false;

    if (t > KERNEL_EVLOG_IRQ_EXIT)
        return false;

    if (hooks[t].no_trace)
        filter_out = hooks[t].no_trace(header, arg0, arg1);

    return no_ipc_trace(get_current_thread()) || filter_out;
}

void tracelog_kernel_print(struct tracelog_entry_header *header, void *buf)
{
    int t = TRACELOG_SUBTYPE(header->type);

    if (t > KERNEL_EVLOG_IRQ_EXIT)
        return;

    hooks[t].print(header, buf);
}

void tracelog_kernel_store(struct tracelog_entry_header *header, void *arg0, void *arg1)
{
    int t = TRACELOG_SUBTYPE(header->type);

    if (t > KERNEL_EVLOG_IRQ_EXIT)
        return;

    hooks[t].store(header, arg0, arg1);
}

