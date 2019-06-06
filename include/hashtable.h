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

#ifndef __HASHTABLE_H
#define __HASHTABLE_H

#include <list.h>
#include <pow2.h>
#include <jhash.h>

#define DEFINE_HASHTABLE(name, bits)                                                    \
    struct list_node name[1 << (bits)] =                                                \
        { [0 ... ((1 << (bits)) - 1)] = LIST_INITIAL_CLEARED_VALUE }

#define DECLARE_HASHTABLE(name, bits)                                                   \
    struct list_node name[1 << (bits)]

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x)           (sizeof(x) / sizeof((x)[0]))
#endif

#define HASH_SIZE(name)         (ARRAY_SIZE(name))
#define HASH_BITS(name)         (log2_uint(HASH_SIZE(name)))

#define hash(val, name)         (jhash(val, strlen(val), 0) & (HASH_SIZE(name) - 1))

static inline void __hash_init(struct list_node *ht, unsigned int sz)
{
    unsigned int i;

    for (i = 0; i < sz; i++)
        list_initialize(&ht[i]);
}

#define hash_init(hashtable)    __hash_init(hashtable, HASH_SIZE(hashtable))

#define hash_add(hashtable, node, key)                                                  \
    list_add_head(&hashtable[hash(key, hashtable)], node)

static inline bool hash_hashed(struct list_node *node)
{
    return list_in_list(node);
}

static inline bool __hash_empty(struct list_node *ht, unsigned int sz)
{
    unsigned int i;

    for (i = 0; i < sz; i++)
        if (!list_is_empty(&ht[i]))
            return false;

    return true;
}

#define hash_empty(hashtable)   __hash_empty(hashtable, HASH_SIZE(hashtable))

static inline void hash_del(struct list_node *node)
{
    list_delete(node);
}

#define hash_for_each(name, bkt, obj, type, member)                                     \
    for ((bkt) = 0, obj = NULL; (bkt) < HASH_SIZE(name); (bkt)++)                       \
        list_for_every_entry(&name[bkt], obj, type, member)

#define hash_for_each_possible(name, obj, type, member, key)                            \
        list_for_every_entry(&name[hash(key, name)], obj, type, member)

#endif
