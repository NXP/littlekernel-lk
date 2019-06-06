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

#include <lk/init.h>
#include <stdio.h>

#include <kernel/trace/tp_printf.h>
#include <kernel/trace/tracelog.h>

void probe_subsys_global_printf(const char *str)
{
    tracelog_write(TRACELOG_SET_TYPE(TRACELOG_TYPE_STR, 0), (void *) str, NULL);
}

static void probes_install(uint level)
{
    register_trace_subsys_global_printf(probe_subsys_global_printf, false);
}
LK_INIT_HOOK(trace_probes, &probes_install, LK_INIT_LEVEL_KERNEL);
