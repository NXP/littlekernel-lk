/*
 * Copyright (c) 2013-2015 Travis Geiselbrecht
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
#if WITH_LIB_OPENLIBM

#include <stdio.h>
#include <math.h>
#include <lib/console.h>
#include <app/tests.h>

static const char *usage =  "Usage: math_tests <verbose: [0..3]>\n";

static int math_tests(int argc, const cmd_args *argv)
{
    int ret, verdict = 0;
    printf("Math library test:\n");
    int verbose = 0;
    if (argc == 2) {
        verbose = argv[2].i;
    }
    ret = math_float_test(verbose);
    if (ret)
        printf("Math float test failed!\n");
    verdict += ret;

    ret = math_double_test(verbose);
    if (ret)
        printf("Math double test failed!\n");
    verdict += ret;

    return verdict;

}

STATIC_COMMAND_START
STATIC_COMMAND("math_tests", "math library test", (console_cmd)&math_tests)
STATIC_COMMAND_END(math_tests);

#endif // WITH_LIB_OPENLIBM
