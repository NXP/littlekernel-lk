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

#include <stdio.h>
#include <string.h>

#include <kernel/trace/tracelog.h>

static void tracelog_bin_print(struct tracelog_entry_header *header, void *buf)
{
    printf("Binary data size: %d bytes\n", header->len);
}

static void tracelog_bin_store(struct tracelog_entry_header *header, void *arg0, void *arg1)
{
    header->len = (uint16_t) ((uintptr_t) arg1 & 0xffffffff);

    memcpy(&header->data[0], arg0, header->len);
}

TRACELOG_START(bin, TRACELOG_TYPE_BINARY)
	.print = tracelog_bin_print,
	.store = tracelog_bin_store,
	.no_trace = NULL,
TRACELOG_END
