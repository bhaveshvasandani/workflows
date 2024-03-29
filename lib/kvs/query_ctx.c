/* SPDX-License-Identifier: Apache-2.0 */
/*
 * Copyright (C) 2015-2020 Micron Technology, Inc.  All rights reserved.
 */

#include <hse_util/page.h>
#include <hse_util/hash.h>
#include <hse_util/slab.h>
#include <hse_util/keycmp.h>
#include <hse_util/byteorder.h>
#include <hse_util/assert.h>
#include <hse_util/event_counter.h>

#include <hse_ikvdb/query_ctx.h>

pthread_key_t tomb_thread_key;

/**
 * struct te_mem - Memory for tomb elems
 * @pglist:  list of all pages that constitute this memory.
 * @curr_pg: current page in use.
 */
struct te_mem {
    void *pglist;
    void *curr_pg;
};

/**
 * struct te_page_hdr - Header at the beginning of each page in the memory
 *                      region's page list
 * @next: pointer to the next page header.
 */
struct te_page_hdr {
    struct te_page_hdr *next;
};

void
qctx_thread_dtor(void *mem)
{
    struct te_mem *     tem = mem;
    struct te_page_hdr *hdr;

    hdr = tem->pglist;
    while (hdr) {
        struct te_page_hdr *next = hdr->next;

        hse_page_free(hdr);
        hdr = next;
    }

    free(tem);
}

static struct tomb_elem *
alloc_tomb_mem(struct query_ctx *qctx, size_t bytes)
{
    struct tomb_elem *  te;
    size_t              sz = sizeof(*te) + bytes;
    struct te_mem *     ptr;
    struct te_page_hdr *hdr;
    unsigned int        min_offset = (sizeof(*hdr) + 0x0f) & (~0x0e);

    ptr = pthread_getspecific(tomb_thread_key);

    if (qctx->pos < min_offset)
        qctx->pos = min_offset;

    if (!ptr) {
        int rc;

        ptr = malloc(sizeof(*ptr));
        if (!ptr)
            return 0;

        ptr->pglist = hse_page_alloc();
        if (!ptr->pglist) {
            free(ptr);
            return 0;
        }

        ptr->curr_pg = ptr->pglist;
        hdr = ptr->curr_pg;
        hdr->next = 0;
        qctx->pos = min_offset;

        rc = pthread_setspecific(tomb_thread_key, ptr);
        if (ev(rc)) {
            hse_page_free(ptr->pglist);
            free(ptr);
            return 0;
        }

    } else if (qctx->pos + sz > PAGE_SIZE) {
        hdr = ptr->curr_pg;
        if (hdr->next) {
            /* There is a cached page that can be used */
            hdr = hdr->next;
        } else {
            /* No more cached pages. Allocate and use a new page */
            void *mem;

            mem = hse_page_alloc();
            if (!mem)
                return 0;

            hdr->next = mem;
            hdr = mem;
            hdr->next = 0;
        }

        ptr->curr_pg = hdr;
        qctx->pos = min_offset;
    }

    te = (void *)(ptr->curr_pg + qctx->pos);
    qctx->pos += (sz + 0x08) & ~0x07;

    return te;
}

merr_t
qctx_tomb_insert(struct query_ctx *qctx, const void *sfx, size_t sfx_len)
{
    struct rb_node * parent, **link;
    struct rb_root * root;
    u64              hash = hse_hash64(sfx, sfx_len);

    assert(sfx_len > 0);
    root = &qctx->tomb_tree;
    link = &root->rb_node;
    parent = NULL;

    while (*link) {
        struct tomb_elem *te;

        __builtin_prefetch(*link);

        parent = *link;
        te = rb_entry(*link, typeof(*te), node);

        if (hash > te->hash)
            link = &(*link)->rb_right;
        else if (hash < te->hash)
            link = &(*link)->rb_left;
        else
            break;
    }

    if (*link) {
        /* duplicate. Do nothing */
    } else {
        struct tomb_elem *te;

        te = alloc_tomb_mem(qctx, sfx_len);
        if (ev(!te))
            return merr(ENOMEM);

        te->hash = hash;

        rb_link_node(&te->node, parent, link);
        rb_insert_color(&te->node, root);
        ++qctx->ntombs;
    }

    return 0;
}

bool
qctx_tomb_seen(struct query_ctx *qctx, const void *sfx, size_t sfx_len)
{
    struct tomb_elem *te = 0;
    struct rb_node  **link;
    struct rb_root   *root;
    u64               hash;

    assert(sfx_len > 0);
    if (!qctx->ntombs)
        return false;

    hash = hse_hash64(sfx, sfx_len);
    root = &qctx->tomb_tree;
    link = &root->rb_node;

    while (*link) {
        __builtin_prefetch(*link);

        te = rb_entry(*link, typeof(*te), node);

        if (hash > te->hash)
            link = &(*link)->rb_right;
        else if (hash < te->hash)
            link = &(*link)->rb_left;
        else
            break;
    }

    if (*link)
        return true;

    return false;
}

merr_t
qctx_te_mem_init(void)
{
    int rc;

    rc = pthread_key_create(&tomb_thread_key, &qctx_thread_dtor);
    return merr(ev(rc));
}

void
qctx_te_mem_reset(void)
{
    struct te_mem *ptr;

    /* reset current page pointer for use by the next call */
    ptr = pthread_getspecific(tomb_thread_key);
    if (ptr)
        ptr->curr_pg = ptr->pglist;
}
