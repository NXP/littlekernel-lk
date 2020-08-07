/*
 * Copyright 2020 - NXP
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

#include <debug.h>
#include <stdlib.h>
#include <string.h>
#include "kernel/thread.h"

extern struct s_dyndbg  __dyndbg_start;
extern struct s_dyndbg  __dyndbg_end;
static unsigned int dyndbg_level = AF_LK_LOGLEVEL;

void dyndbg_dump(void)
{
    struct s_dyndbg *dyndbg;
    for (dyndbg = &__dyndbg_start; dyndbg != &__dyndbg_end; dyndbg++) {
        printf("level: %d - func: %s @ %s:%d -%sabled\n",
            dyndbg->level,
            dyndbg->func,
            dyndbg->fname,
            dyndbg->line,
            (dyndbg->flags & DYNDBG_PRINT) ? "en": "dis"
        );
        /* Required, otherwise prints getting overwritten in intermediate buffer */
        thread_sleep(10);
    }
}

void dyndbg_active(struct s_dyndbg *dyndbg, bool enable)
{
    if (enable) {
        dyndbg->flags |= DYNDBG_PRINT;
    } else {
        /* Disable only if level is more verbose than current log lvl */
        if (dyndbg->level > dyndbg_level)
            dyndbg->flags &= ~DYNDBG_PRINT;
    }
}

void dyndbg_set_by_func(const char *func, int line, bool enable)
{
    struct s_dyndbg *dyndbg;
    size_t len = strlen(func);
    for (dyndbg = &__dyndbg_start; dyndbg != &__dyndbg_end; dyndbg++) {
        if (dyndbg->func && !strncmp(dyndbg->func, func, len)
                && ((line == -1) || (((unsigned int) line) == dyndbg->line)))
            dyndbg_active(dyndbg, enable);
    }
}

void dyndbg_set_by_file(const char *fname, int line, bool enable)
{
    struct s_dyndbg *dyndbg;
    size_t len = strlen(fname);
    for (dyndbg = &__dyndbg_start; dyndbg != &__dyndbg_end; dyndbg++) {
        if (dyndbg->fname && !strncmp(dyndbg->fname, fname, len)
                && ((line == -1) || (((unsigned int) line) == dyndbg->line)))
            dyndbg_active(dyndbg, enable);
    }
}

void dyndbg_set_loglevel(int lvl)
{
    struct s_dyndbg *dyndbg;
    dyndbg_level = lvl;
    for (dyndbg = &__dyndbg_start; dyndbg != &__dyndbg_end; dyndbg++) {
        if (dyndbg->level <= lvl)
            dyndbg_active(dyndbg, true);
        else
            dyndbg_active(dyndbg, false);
    }

}

#if WITH_LIB_CONSOLE
#include <stdio.h>
#include <lib/console.h>
#include <ctype.h>
#include <getopt.h>
#include <err.h>

static void usage(void)
{
    printf("dyndbg_cmd -[Dhed] -f FILE -l LINE -n NAME : \n");
    printf("Enable or disable a print dynamically\n");
    printf("-h\t\t\tPrint this helps\n");
    printf("-D\t\t\tDump all print parameters on console\n");
    printf("-L\tDEBUG\tSet global log level to DEBUG[0-7]\n");
    printf("-e\t\t\tEnable the print specified later\n");
    printf("-d\t\t\tDisable the print specified later\n");
    printf("-f\tFILE\tSpecify the print through its file location\n");
    printf("-l\tLINE\t Specify the print through its line location\n");
    printf("-n\tFUNCNAME\tSpecify the prints through its function location\n");

}

static int cmd_dyndbg(int argc, const cmd_args *argv)
{
    const char * nargv[argc];
    int i, c;
    for (i = 0; i < argc; i++)
        nargv[i] = argv[i].str;
    int enable = -1, line = -1;
    const char *file = NULL;
    const char *name = NULL;

    while ((c = getopt(argc, (char * const *)nargv, "Dhedf:l:n:L:")) != -1) {
        switch (c) {
        case 'h':
            usage();
            return 0;
        case 'D':
            dyndbg_dump();
            return 0;
        case 'L':
            dyndbg_set_loglevel(atoi(optarg));
            return 0;
        case 'e':
            enable = 1;
            break;
        case 'd':
            enable = 0;
            break;
        case 'f':
            file = optarg;
            break;
        case 'n':
            name = optarg;
            break;
        case 'l':
            line = atoi(optarg);
            break;
        case '?':
            if ((optopt == 'f') || (optopt == 'l') || (optopt == 'n'))
                printf("Option -%c requires an argument\n", optopt);
            else if (isprint(optopt))
                printf("Unknown option -%c\n", optopt);
            else
                printf("Unknown option character %x\n", optopt);
            return ERR_GENERIC;
        default:
            panic("getopt unexpected behaviour\n");
        }
    }

    printf("Enable: %d, file: %s func: %s line: %d\n", enable, file, name, line);
    if (name != NULL)
        dyndbg_set_by_func(name, line, enable);

    if (file != NULL)
        dyndbg_set_by_file(file, line, enable);

    return 0;
}

static int cmd_dyndbg_test(int argc, const cmd_args *argv)
{
    printlk(LK_ERR, "This is a error msg %d\n", argc);
    printlk(LK_WARNING, "This is a warning msg\n");
    printlk(LK_NOTICE, "This is a notice msg: (%d) arguments\n", argc);
    printlk(LK_VERBOSE, "This is a verbose msg at line %d: (%d) arguments\n", __LINE__, argc);
    printlk(LK_DEBUG, "This is a debug msg: (%d) arguments\n", argc);
    printlk(LK_VERBOSE, "This is a verbose msg: (%d) arguments\n", argc);
    printlk(LK_VERBOSE, "This is a verbose msg at line %d: (%d) arguments\n", __LINE__, argc);

    dyndbg_dump();
    return 0;
}

STATIC_COMMAND_START
STATIC_COMMAND("dyndbg", "commands for manipulating kernel dynamic debug log", &cmd_dyndbg)
STATIC_COMMAND("dyndbg_test", "dynamic debug test", &cmd_dyndbg_test)
STATIC_COMMAND_END(dyndbg);


#endif // WITH_LIB_CONSOLE
