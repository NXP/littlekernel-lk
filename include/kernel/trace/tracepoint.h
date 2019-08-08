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

#define LK_PARAMS(params...)    params

#if WITH_KERNEL_TRACEPOINT

#define LK_TP(tp, proto, params)                                                            \
    static inline void lk_trace_##tp(proto)                                                 \
    {                                                                                       \
        static const char __lk_tp_str_##tp[]                                                \
        __attribute__((section("__lk_tp_strings"))) = #tp;                                  \
                                                                                            \
        static struct tracepoint __lk_tp_##tp                                               \
        __attribute__((section("__lk_tp"), aligned(8))) = { __lk_tp_str_##tp, 0, NULL };    \
                                                                                            \
        if (__lk_tp_##tp.state) {                                                           \
            void **f = (&__lk_tp_##tp)->funcs;                                              \
                                                                                            \
            while (f && (*f)) {                                                             \
                ((void (*)(proto))(*(f++)))(params);                                        \
            };                                                                              \
        }                                                                                   \
    }                                                                                       \
    static inline int lk_register_trace_##tp(void (*probe)(proto), int state)               \
    {                                                                                       \
        return lk_tracepoint_probe_register(#tp, (void *)probe, state);                     \
    }

#else // !WITH_KERNEL_TRACEPOINT

#define LK_TP(tp, proto, params)                                                            \
    static inline void lk_trace_##tp(proto) { }                                             \
    static inline int lk_register_trace_##tp(void (*probe)(proto), int state)               \
    {                                                                                       \
        return -1;                                                                          \
    }

#endif

extern int lk_tracepoint_probe_register(const char *name, void *probe, int state);

__END_CDECLS

#endif
