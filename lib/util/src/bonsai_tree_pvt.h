/* SPDX-License-Identifier: Apache-2.0 */
/*
 * Copyright (C) 2015-2020 Micron Technology, Inc.  All rights reserved.
 */

#ifndef HSE_BONSAI_TREE_PVT_H
#define HSE_BONSAI_TREE_PVT_H

#include <hse_util/string.h>
#include <hse_util/alloc.h>
#include <hse_util/slab.h>
#include <hse_util/barrier.h>
#include <hse_util/bonsai_tree.h>
#include <hse_util/assert.h>

enum bonsai_update_lr {
    B_UPDATE_L = 0,
    B_UPDATE_R = 1,
};

enum bonsai_match_type {
    B_MATCH_EQ = 0,
    B_MATCH_GE = 1,
    B_MATCH_LE = 2,
};

/**
 * bn_node_alloc() - allocate and initialize a node plus key and value
 * @tree:    bonsai tree instance
 * @skey:
 * @sval:
 *
 * Return: new node at height 1 with nil left/right child ptrs
 */
struct bonsai_node *
bn_kvnode_alloc(
    struct bonsai_root       *tree,
    const struct bonsai_skey *skey,
    const struct bonsai_sval *sval);

/**
 * bn_val_alloc() - allocate and initialize a value
 * @tree:       bonsai tree instance
 * @sval:
 * @deepcopy:   value must be copied into bonsai tree if true
 *
 * Return:
 */
struct bonsai_val *
bn_val_alloc(struct bonsai_root *tree, const struct bonsai_sval *sval, bool deepcopy);

/**
 * bn_node_dup() - allocate and initialize a new node copied from src
 * @tree: bonsai tree instance
 * @src:  bonsai_node to be dup'ed
 *
 * Return: new node (duplicated from 'node') at height of src node
 */
struct bonsai_node *
bn_node_dup(struct bonsai_root *tree, struct bonsai_node *src);

/**
 * bn_node_dup_ext() - allocate and initialize a new node copied from src
 * @tree:  bonsai tree instance
 * @node:  bonsai_node to be dup'ed
 * @left:  left child
 * @right: right child
 *
 * Similar to bn_node_dup() but installs new left and right children.
 *
 * Return: new node (duplicated from 'node') at max height of left/right
 */
struct bonsai_node *
bn_node_dup_ext(
    struct bonsai_root *tree,
    struct bonsai_node *src,
    struct bonsai_node *left,
    struct bonsai_node *right);

/**
 * bn_balance_tree() -
 * @tree:    bonsai tree instance
 * @node:
 * @left:    left child
 * @right:   right child
 * @key_imm: key immediate
 * @key:
 *
 * Return:
 */
struct bonsai_node *
bn_balance_tree(
    struct bonsai_root *        tree,
    struct bonsai_node *        node,
    struct bonsai_node *        left,
    struct bonsai_node *        right,
    const struct key_immediate *key_imm,
    const void *                key,
    enum bonsai_update_lr       lr);

/**
 * bn_height_max() -
 * @a:
 * @b:
 *
 * Return:
 */
static HSE_ALWAYS_INLINE int
bn_height_max(int a, int b)
{
    return (a > b) ? a : b;
}

/**
 * bn_height_get() -
 * @node:
 *
 * Return:
 */
static HSE_ALWAYS_INLINE int
bn_height_get(const struct bonsai_node *node)
{
    return node ? node->bn_height : 0;
}

/**
 * bn_height_update() -
 * @node:
 *
 * Return:
 */
static HSE_ALWAYS_INLINE void
bn_height_update(struct bonsai_node *node)
{
    node->bn_height = bn_height_max(bn_height_get(node->bn_left), bn_height_get(node->bn_right));

    node->bn_height++;
}

#endif /* HSE_BONSAI_TREE_PVT_H */