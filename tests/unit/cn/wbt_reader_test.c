/* SPDX-License-Identifier: Apache-2.0 */
/*
 * Copyright (C) 2015-2020 Micron Technology, Inc.  All rights reserved.
 */

#include <hse_ut/framework.h>
#include <hse_util/logging.h>
#include <hse_util/alloc.h>
#include <hse_util/slab.h>
#include <hse_util/page.h>

#include <cn/omf.h>
#include <cn/wbt_reader.h>
#include <cn/wbt_internal.h>
#include <cn/kblock_reader.h>

#include <mocks/mock_mpool.h>

#include <stdlib.h>

char data_path[PATH_MAX];

char *kblock_files[] = {
    "simple_500b.kb3_w3_b3.xz",
    "simple_500c.kb5_w5_b4.xz",
};

int
test_collection_setup(struct mtf_test_info *info)
{
    struct mtf_test_coll_info *coll_info = info->ti_coll;
    int                        len;

    hse_openlog("wbt_reader_test", 1);

    if (coll_info->tci_argc != 2) {
        hse_log(HSE_ERR "Usage:  %s <mblock_image_dir>", coll_info->tci_argv[0]);
        return -1;
    }

    len = strlen(coll_info->tci_argv[1]);
    if (coll_info->tci_argv[1][len - 1] == '/')
        coll_info->tci_argv[1][len - 1] = 0;
    strncpy(data_path, coll_info->tci_argv[1], sizeof(data_path) - 1);

    mock_mpool_set();
    return 0;
}

int
test_collection_teardown(struct mtf_test_info *info)
{
    mock_mpool_unset();
    return 0;
}

void
t_wbtr_read_vref_helper(struct mtf_test_info *lcl_ti, const char *kblock_image_file)
{
    merr_t                err;
    struct mpool *        mp_ds = (void *)-1;
    struct wbt_desc       desc = {};
    void *                wbt_hdr;
    char                  keybuf[100];
    struct kvs_ktuple     ktuple;
    enum key_lookup_res   lookup_res;
    struct kvs_vtuple_ref vref;
    int                   i, nkeys;
    char                  filename[PATH_MAX];
    struct kvs_mblk_desc  blkdesc = {};
    u64                   seqno, blkid;
    int                   rc;

    rc = snprintf(filename, sizeof(filename), "%s/%s", data_path, kblock_image_file);
    ASSERT_LT(rc, sizeof(filename));

    hse_log(HSE_DEBUG "Testing with kblock %s", filename);

    err = mpm_mblock_alloc_file(&blkid, filename);
    ASSERT_EQ(err, 0);
    blkdesc.mb_id = blkid;
    blkdesc.map_idx = 0;

    err = mpool_mcache_mmap(mp_ds, 1, &blkdesc.mb_id, &blkdesc.map);
    ASSERT_EQ(err, 0);

    blkdesc.map_base = mpool_mcache_getbase(blkdesc.map, blkdesc.map_idx);
    ASSERT_NE(blkdesc.map_base, NULL);

    wbt_hdr = blkdesc.map_base + omf_kbh_wbt_hoff(blkdesc.map_base);

    desc.wbd_first_page = omf_kbh_wbt_doff_pg(blkdesc.map_base);
    desc.wbd_n_pages = omf_kbh_wbt_dlen_pg(blkdesc.map_base);

    err = kbr_read_wbt_region_desc_mem(wbt_hdr, &desc);
    ASSERT_EQ(err, 0);

    nkeys = 500;
    seqno = U64_MAX; /* read latest value of each key */
    for (i = 0; i < nkeys + 10; ++i) {

        /* Format key to match what simple_client tool uses */
        snprintf(keybuf, sizeof(keybuf), "key.%09d", i);
        ktuple.kt_hash = 0;
        ktuple.kt_len = strlen(keybuf);
        ktuple.kt_data = keybuf;

        wbtr_read_vref(&blkdesc, &desc, &ktuple, 0, seqno, &lookup_res, &vref);
        if (i < nkeys)
            ASSERT_EQ(FOUND_VAL, lookup_res);
        else
            ASSERT_EQ(NOT_FOUND, lookup_res);
    }

    struct wbti *wbti;
    const void * kdata;
    uint         klen;
    const void * kmd;

    snprintf(keybuf, sizeof(keybuf), "key");
    ktuple.kt_hash = 0;
    ktuple.kt_len = -1 * strlen(keybuf);
    ktuple.kt_data = keybuf;
    err = wbti_create(&wbti, &blkdesc, &desc, &ktuple, false, false);
    ASSERT_EQ(0, err);

    int cnt = 0;

    while (wbti_next(wbti, &kdata, &klen, &kmd))
        cnt++;

    ASSERT_EQ(nkeys, cnt);
    wbti_destroy(wbti);

    err = mpool_mcache_munmap(blkdesc.map);
    ASSERT_EQ(err, 0);
}

MTF_BEGIN_UTEST_COLLECTION_PREPOST(
    wbt_reader_test,
    test_collection_setup,
    test_collection_teardown);

MTF_DEFINE_UTEST(wbt_reader_test, t_wbtr_read_vref)
{
    int i;

    for (i = 0; i < NELEM(kblock_files); i++)
        t_wbtr_read_vref_helper(lcl_ti, kblock_files[i]);
}

MTF_END_UTEST_COLLECTION(wbt_reader_test)
