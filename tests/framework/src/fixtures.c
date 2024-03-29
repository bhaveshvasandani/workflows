/* SPDX-License-Identifier: Apache-2.0 */
/*
 * Copyright (C) 2021 Micron Technology, Inc. All rights reserved.
 */

#include <hse_ut/fixtures.h>

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>

extern char home[PATH_MAX];

static struct hse_kvdb *mtf_kvdb_handle;

/* By default, the UT framework will try to plow ahead in the presence
 * of failures.  Unfortunately, that often leads to red herrings due
 * to insufficent cleanup of the failing test (e.g., pthreads) such
 * that things go confusingly awry during hse_init() of the subsequent
 * subtest.  Instead, we now we abort the entire test at the first sign
 * of trouble.  Individual tests can override this debug hook if need
 * be, but it is not recommended.
 */
HSE_WEAK
void
mtf_debug_hook(void)
{
    abort();
}

void
mtf_print_errinfo(
    hse_err_t               err,
    const char             *fmt,
    ...)
{
    va_list     ap;
    char        msgbuf[256];

    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    if (err) {
        hse_strerror(err, msgbuf, sizeof(msgbuf));
        fprintf(stderr, "\nError detail: %s\n", msgbuf);
    }
}

void
mtf_print_err(
    const char             *fmt,
    ...)
{
    va_list     ap;

    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
}

static int
mtf_kvs_drop_all_warn(
    struct hse_kvdb    *kvdb,
    bool                warn)
{
    int       rc;
    hse_err_t err;
    size_t    kvs_count;
    char    **kvs_list;

    err = hse_kvdb_kvs_names_get(kvdb, &kvs_count, &kvs_list);
    if (err) {
        mtf_print_errinfo(err, "%s: hse_kvdb_kvs_names_get failed\n", __func__);
        return -1;
    }

    rc = 0;

    for (unsigned int i = 0; i < kvs_count; i++) {
        const char *kvs_name = kvs_list[i];

        if (warn)
            mtf_print_err("warning: deleting leftover kvs %s\n", kvs_name);

        err = hse_kvdb_kvs_drop(mtf_kvdb_handle, kvs_name);
        if (err) {
            mtf_print_errinfo(err, "unable to delete kvs %s\n", kvs_name);
            rc = -1;
        }
    }

    hse_kvdb_kvs_names_free(kvdb, kvs_list);
    return rc;
}


int
mtf_kvdb_kvs_drop_all(
    struct hse_kvdb        *kvdb)
{
    return mtf_kvs_drop_all_warn(kvdb, true);
}


int
mtf_kvdb_setupv(
    struct mtf_test_info   *lcl_ti,
    struct hse_kvdb       **kvdb_out,
    va_list                 ap)
{
    hse_err_t           err;
    const char         *param;

    if (!lcl_ti) {
        mtf_print_err("%s: missing required parameter: lcl_ti\n", __func__);
        goto fail;
    }

    if (!kvdb_out) {
        mtf_print_err("%s: missing required parameter: kvdb_out\n", __func__);
        goto fail;
    }

    param = va_arg(ap, const char *);
    if (param) {
        mtf_print_err("%s: improperly terminated var args list\n", __func__);
        goto fail;
    }

    if (mtf_kvdb_handle) {
        mtf_print_err("%s: mtf_kvdb_setup should only be called once.\n", __func__);
        goto fail;
    }

    err = hse_kvdb_create(home, 0, NULL);
    if (err) {
        mtf_print_errinfo(err, "Cannot create KVDB '%s'\n", home);
        goto fail;
    }

    err = hse_kvdb_open(home, 0, NULL, &mtf_kvdb_handle);
    if (err) {
        mtf_print_errinfo(err, "Cannot open KVDB '%s'\n", home);
        goto fail;
    }

    if (mtf_kvdb_kvs_drop_all(mtf_kvdb_handle))
        goto fail;

    *kvdb_out = mtf_kvdb_handle;
    return 0;

  fail:
    if (mtf_kvdb_handle) {
        hse_kvdb_close(mtf_kvdb_handle);
        hse_kvdb_drop(home, 0, NULL);
        mtf_kvdb_handle = 0;
    }

    return -1;
}

int
mtf_kvdb_setup(
    struct mtf_test_info   *lcl_ti,
    struct hse_kvdb       **kvdb,
    ...)
{
    int rc;
    va_list ap;

    va_start(ap, kvdb);
    rc = mtf_kvdb_setupv(lcl_ti, kvdb, ap);
    va_end(ap);

    return rc;
}

int
mtf_kvdb_teardown(
    struct mtf_test_info   *lcl_ti)
{
    hse_err_t err = 0;
    int rc = 0;

    if (mtf_kvdb_handle) {
        rc = mtf_kvs_drop_all_warn(mtf_kvdb_handle, false);
        err = hse_kvdb_close(mtf_kvdb_handle);
        if (err) {
            mtf_print_errinfo(err, "%s: hse_kvdb_close failed\n", __func__);
        }
        err = hse_kvdb_drop(home, 0, NULL);
        if (err) {
            mtf_print_errinfo(err, "%s: hse_kvdb_drop failed\n", __func__);
        }
    }

    return (rc || err) ? -1 : 0;
}
