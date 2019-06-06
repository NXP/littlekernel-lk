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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define RPMSG_PATH  "/dev/lktraces"
#define PAGE_SIZE   4096

int main(int argc, char *argv[])
{
    int fd_r, fd_w;
    ssize_t count = 0;
#ifdef DEBUG
    ssize_t tot = 0;
#endif
    char buf[PAGE_SIZE];

    fd_r = open(RPMSG_PATH, O_RDONLY);
    if (fd_r < 0)
        exit(EXIT_FAILURE);

    fd_w = open(argv[1], O_WRONLY | O_CREAT);
    if (fd_w < 0)
        exit(EXIT_FAILURE);

    while (1) {
        count = read(fd_r, buf, PAGE_SIZE);
        if (!count) {
#ifdef DEBUG
            tot = 0;
#endif
	    continue;
        }

#ifdef DEBUG
        tot += count;

        printf("count: %ld tot: %ld\n", count, tot);
#endif

        if (write(fd_w, buf, count) < 0)
            exit(EXIT_FAILURE);
    }

    return(EXIT_SUCCESS);
}
