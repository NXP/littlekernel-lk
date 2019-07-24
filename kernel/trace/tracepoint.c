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

#include <kernel/trace/tracepoint.h>

#include <err.h>
#include <hashtable.h>
#include <lk/init.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern struct tracepoint __start___tracepoints[];
extern struct tracepoint __stop___tracepoints[];

#define TRACEPOINT_HASH_BITS    6

static DEFINE_HASHTABLE(tracepoint_table, TRACEPOINT_HASH_BITS);

struct tracepoint_entry {
    struct list_node node;
    void **funcs;
    int state;
    char name[0];
};

static struct tracepoint_entry *get_tracepoint(const char *name)
{
    struct tracepoint_entry *e;

    hash_for_each_possible(tracepoint_table, e, struct tracepoint_entry, node, name) {
        if (!strcmp(name, e->name))
            return e;
    }
    return NULL;
}

static void set_tracepoint(struct tracepoint_entry *entry,
                           struct tracepoint *elem)
{
    elem->funcs = entry->funcs;
    elem->state = 1;
}

static void disable_tracepoint(struct tracepoint *elem)
{
    elem->state = 0;
}

static void tracepoint_update_probes(void)
{
    struct tracepoint *b = __start___tracepoints;
    struct tracepoint *e = __stop___tracepoints;
    struct tracepoint *iter;
    struct tracepoint_entry *entry;

    for (iter = b; iter < e; iter++) {
        entry = get_tracepoint(iter->name);
        if (entry && entry->state)
            set_tracepoint(entry, iter);
        else
            disable_tracepoint(iter);
    }
}

static status_t tracepoint_set_status(const char *name, unsigned int state)
{
    struct tracepoint_entry *e;

    e = get_tracepoint(name);
    if (!e)
        return ERR_NOT_FOUND;

    e->state = !!state;

    tracepoint_update_probes();

    return NO_ERROR;
}

static struct tracepoint_entry *add_tracepoint(const char *name, int state)
{
    struct tracepoint_entry *e;
    size_t name_len = strlen(name) + 1;

    hash_for_each_possible(tracepoint_table, e, struct tracepoint_entry, node, name) {
        if (!strcmp(name, e->name)) {
            printf("Tracepoint %s already existing\n", name);
            return NULL;
        }
    }

    e = malloc(sizeof(struct tracepoint_entry) + name_len);
    if (!e) {
        printf("Failed to allocate memory\n");
        return NULL;
    }
    memcpy(&e->name[0], name, name_len);
    e->funcs = NULL;
    e->state = !!state;

    hash_add(tracepoint_table, &e->node, name);

    return e;
}

static status_t tracepoint_entry_add_probe(struct tracepoint_entry *entry, void *probe)
{
    int nr_probes = 0;
    void **old, **new;

    old = entry->funcs;
    if (old) {
        for (nr_probes = 0; old[nr_probes]; nr_probes++)
            if (old[nr_probes] == probe) {
                printf("Probe already existing\n");
                return ERR_ALREADY_EXISTS;
            }
    }

    new = calloc((nr_probes + 2), sizeof(void *));
    if (!new) {
        printf("Failed to allocate memory\n");
        return ERR_NO_MEMORY;
    }

    if (old)
        memcpy(new, old, nr_probes * sizeof(void *));
    new[nr_probes] = probe;
    entry->funcs = new;

    return NO_ERROR;
}

int lk_tracepoint_probe_register(const char *name, void *probe, int state)
{
    struct tracepoint_entry *entry;
    status_t ret;

    entry = get_tracepoint(name);
    if (!entry) {
        entry = add_tracepoint(name, state);
        if (!entry)
            return ERR_GENERIC;
    }

    ret = tracepoint_entry_add_probe(entry, probe);
    if (ret)
        return ret;

    tracepoint_update_probes();

    return NO_ERROR;
}

static void tracepoint_init(uint level)
{
    hash_init(tracepoint_table);
}
LK_INIT_HOOK(trace_tracepoint, &tracepoint_init, LK_INIT_LEVEL_TARGET_EARLY);

#if WITH_LIB_CONSOLE

#include <lib/console.h>

static void cmd_do_list(void)
{
    struct tracepoint_entry *e;
    unsigned int i, nr_probes;

    hash_for_each(tracepoint_table, i, e, struct tracepoint_entry, node) {
            printf("[%s] active: %d\n", e->name, e->state);

            for (nr_probes = 0; e->funcs[nr_probes]; nr_probes++)
                printf("\tprobe %d : %p\n", nr_probes, e->funcs[nr_probes]);
    }
}

static status_t cmd_do_set(const char *name, unsigned int state)
{
    int ret;

    ret = tracepoint_set_status(name, state);
    if (ret < 0)
        printf("Error when setting state %d for tracepoint %s\n", state, name);
    else
        printf("[%s] tracepoint %s\n", name, state ? "enabled" : "disabled");

    return ret;

}

static int cmd_trace(int argc, const cmd_args *argv)
{
    int ret = NO_ERROR;

    if (argc < 2) {
usage:
        printf("%s list: list tracepoints\n", argv[0].str);
        printf("%s enable <tracepoint>: enable tracepoint\n", argv[0].str);
        printf("%s disable <tracepoint>: disable tracepoint\n", argv[0].str);
        return ERR_GENERIC;
    }

    if (!strcmp(argv[1].str, "list")) {
        cmd_do_list();
    } else if (!strcmp(argv[1].str, "enable")) {
        if (argc < 3) goto usage;
        ret = cmd_do_set(argv[2].str, 1);
    } else if (!strcmp(argv[1].str, "disable")) {
        if (argc < 3) goto usage;
        ret = cmd_do_set(argv[2].str, 0);
    } else {
        printf("Command unknown\n");
        goto usage;
    }

    return ret;
}

STATIC_COMMAND_START
STATIC_COMMAND("trace", "commands for manipulating tracepoints", &cmd_trace)
STATIC_COMMAND_END(trace);

#endif // WITH_LIB_CONSOLE

