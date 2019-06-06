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

#ifndef __KERNEL_TRACEPOINT_H
#define __KERNEL_TRACEPOINT_H

#include <compiler.h>
#include <list.h>

__BEGIN_CDECLS

struct tracepoint {
    const char *name;
    int state;
    void **funcs;
} __attribute__((aligned(8)));

#define TPPROTO(args...)    args
#define TPARGS(args...)     args

#if WITH_KERNEL_TRACEPOINT

#define __DO_TRACE(tp, proto, args)                                             \
    do {                                                                        \
        void **it_func;                                                         \
                                                                                \
        it_func = (tp)->funcs;                                                  \
        if (it_func) {                                                          \
            do {                                                                \
                ((void(*)(proto))(*it_func))(args);                             \
            } while (*(++it_func));                                             \
        }                                                                       \
    } while (0)

#define DEFINE_TRACE(name, proto, args)                                         \
    static inline void trace_##name(proto)                                      \
    {                                                                           \
        static const char __tpstrtab_##name[]                                   \
        __attribute__((section("__tracepoints_strings"))) = #name;              \
                                                                                \
        static struct tracepoint __tracepoint_##name                            \
        __attribute__((section("__tracepoints"), aligned(8))) =                 \
        { __tpstrtab_##name, 0, NULL };                                         \
        if (unlikely(__tracepoint_##name.state))                                \
             __DO_TRACE(&__tracepoint_##name,                                   \
                 TPPROTO(proto), TPARGS(args));                                 \
    }                                                                           \
    static inline int register_trace_##name(void (*probe)(proto), int state)    \
    {                                                                           \
        return tracepoint_probe_register(#name, (void *)probe, state);          \
    }

#else // !WITH_KERNEL_TRACEPOINT

#define DEFINE_TRACE(name, proto, args)                                         \
    static inline void trace_##name(proto) { }                                  \
    static inline int register_trace_##name(void (*probe)(proto), int state)    \
    {                                                                           \
        return -1;                                                              \
    }

#endif

extern int tracepoint_probe_register(const char *name, void *probe, int state);

__END_CDECLS

#endif
