/* SPDX-License-Identifier: Apache-2.0 */
/*
 * Copyright (C) 2015-2020 Micron Technology, Inc.  All rights reserved.
 */

#include <fcntl.h>

#include <hse_util/event_counter.h>
#include <hse_util/hse_err.h>
#include <hse_util/logging.h>

#include "mpool.h"
#include "mdc.h"
#include "mdc_file.h"

struct media_class *
mdc_mclass_get(struct mpool_mdc *mdc)
{
    return mdc->mc;
}

merr_t
mpool_mdc_alloc(
	struct mpool           *mp,
    uint32_t                magic,
    size_t                  capacity,
	enum mp_media_classp    mclass,
    uint64_t               *logid1,
    uint64_t               *logid2)
{
    enum mclass_id mcid;
	merr_t err;
    uint64_t id[2];
    int i, dirfd, flags, mode;

    if (ev(!mp || mclass >= MP_MED_COUNT || capacity < MDC_LOGHDR_LEN))
        return merr(EINVAL);

    dirfd = mclass_dirfd(mpool_mclass_handle(mp, mclass));
    flags = O_RDWR | O_CREAT | O_EXCL;
    mode = S_IRUSR | S_IWUSR;

    mcid = mclass_to_mcid(mclass);
    for (i = 0; i < 2; i++) {
        id[i] = logid_make(i, mcid, magic);

        err = mdc_file_create(dirfd, id[i], flags, mode, capacity);
        if (ev(err)) {
            if (i != 0)
                mdc_file_destroy(dirfd, id[0]);
            return err;
        }
    }

    *logid1 = id[0];
    *logid2 = id[1];

    return 0;
}

merr_t
mpool_mdc_commit(struct mpool *mp, uint64_t logid1, uint64_t logid2)
{
    enum mclass_id mcid;
	merr_t err;
    int dirfd, i;
    uint64_t id[] = {logid1, logid2};;

    if (ev(!mp || !logids_valid(logid1, logid2)))
        return merr(EINVAL);

    mcid = logid_mcid(logid1);
    dirfd = mclass_dirfd(mpool_mclass_handle(mp, mcid_to_mclass(mcid)));

    for (i = 0; i < 2; i++) {
        err = mdc_file_commit(dirfd, id[i]);
        if (ev(err)) {
            while (i >= 0)
                mdc_file_destroy(dirfd, id[i--]);
            return merr(err);
        }
    }

	return 0;
}

merr_t
mpool_mdc_delete(struct mpool *mp, uint64_t logid1, uint64_t logid2)
{
    enum mclass_id mcid;
    merr_t err, rval=0;
    int dirfd, i;
    uint64_t id[] = {logid1, logid2};;

    if (ev(!mp || !logids_valid(logid1, logid2)))
        return merr(EINVAL);

    mcid = logid_mcid(logid1);
    dirfd = mclass_dirfd(mpool_mclass_handle(mp, mcid_to_mclass(mcid)));

    for (i = 0; i < 2; i++) {
        err = mdc_file_destroy(dirfd, id[i]);
        if (ev(err))
            rval = err;
    }

    return rval;
}

merr_t
mpool_mdc_abort(struct mpool *mp, uint64_t logid1, uint64_t logid2)
{
    return mpool_mdc_delete(mp, logid1, logid2);
}

merr_t
mpool_mdc_open(
    struct mpool        *mp,
    uint64_t             logid1,
    uint64_t             logid2,
    struct mpool_mdc   **handle)
{
    struct mdc_file   *mfp[2] = {};
    struct mpool_mdc  *mdc;

    enum mclass_id mcid;
    merr_t  err, err1, err2;
    uint64_t gen1, gen2;

    if (ev(!mp || !handle || logid1 == logid2))
        return merr(EINVAL);

    mdc = calloc(1, sizeof(*mdc));
    if (ev(!mdc))
        return merr(ENOMEM);

    mdc->mp = mp;
    mcid = logid_mcid(logid1);
    mdc->mc = mpool_mclass_handle(mp, mcid_to_mclass(mcid));

    err1 = mdc_file_open(mdc, logid1, &gen1, &mfp[0]);
    err2 = mdc_file_open(mdc, logid2, &gen2, &mfp[1]);

    err = err1 ? err1 : err2;

    if (err || (!err && gen1 && gen1 == gen2)) {
        err = merr(EINVAL);
        hse_log(HSE_ERR "MDC (%lu:%lu) bad pair gen (%lu, %lu)",
                 logid1, logid2, gen1, gen2);
    } else {
        /* active log is valid log with smallest gen */
        if (gen2 < gen1) {
            mdc->mfpa = mfp[1];

            /**
             * Unconditionally erase the passive log.
             * This handles the following crash scenarios:
             * 1. Crash after mdc_cstart but before mdc_cend()
             * 2. Crash after gen update and during passive log erase
             * 3. Crash after passive log erase
             */
            err = mdc_file_erase(mfp[0], gen2 + 1);
            if (err)
                hse_elog(HSE_ERR "mdc file1 logid %lu erase failed: @@e", err, logid1);
        } else {
            mdc->mfpa = mfp[0];

            err = mdc_file_erase(mfp[1], gen1 + 1);
            if (err)
                hse_elog(HSE_ERR "mdc file2 logid %lu erase failed: @@e", err, logid2);
        }
    }

    if (!err) {
        mdc->mfp1 = mfp[0];
        mdc->mfp2 = mfp[1];
        mutex_init(&mdc->lock);

        *handle = mdc;
    } else {
        if (mfp[0])
            err1 = mdc_file_close(mfp[0]);
        if (mfp[1])
            err2 = mdc_file_close(mfp[1]);
    }

    if (err)
        free(mdc);

    return err;
}

merr_t
mpool_mdc_close(struct mpool_mdc *mdc)
{
    merr_t err, rval = 0;

    if (ev(!mdc))
        return merr(EINVAL);

    mutex_lock(&mdc->lock);

    err = mdc_file_close(mdc->mfp1);
    if (err) {
        hse_elog(HSE_ERR "mdc close failed, mfp1 %p: @@e", err, mdc->mfp1);
        rval = err;
    }

    err = mdc_file_close(mdc->mfp2);
    if (err) {
        hse_elog(HSE_ERR "mdc close failed, mfp2 %p: @@e", err, mdc->mfp2);
        rval = err;
    }

    mutex_unlock(&mdc->lock);

    free(mdc);

    return rval;
}

merr_t
mpool_mdc_cstart(struct mpool_mdc *mdc)
{
    struct mdc_file *tgth;
    merr_t err;

    if (ev(!mdc))
        return merr(EINVAL);

    mutex_lock(&mdc->lock);

    if (mdc->mfpa == mdc->mfp1)
        tgth = mdc->mfp2;
    else
        tgth = mdc->mfp1;

    err = mdc_file_sync(tgth);
    if (ev(err)) {
        mutex_unlock(&mdc->lock);
        mpool_mdc_close(mdc);

        return err;
    }

    mdc->mfpa = tgth;

    mutex_unlock(&mdc->lock);

    return 0;
}

merr_t
mpool_mdc_cend(struct mpool_mdc *mdc)
{
    struct mdc_file  *srch, *tgth;
    merr_t err;
    uint64_t gentgt = 0;

    if (ev(!mdc))
        return merr(EINVAL);

    mutex_lock(&mdc->lock);

    if (mdc->mfpa == mdc->mfp1) {
        tgth = mdc->mfp1;
        srch = mdc->mfp2;
    } else {
        tgth = mdc->mfp2;
        srch = mdc->mfp1;
    }

    err = mdc_file_sync(tgth);
    if (!err) {
        err = mdc_file_gen(tgth, &gentgt);
        if (!err)
            err = mdc_file_erase(srch, gentgt + 1);
    }

    if (ev(err)) {
        mutex_unlock(&mdc->lock);
        mpool_mdc_close(mdc);

        return err;
    }

    mutex_unlock(&mdc->lock);

    return err;
}

merr_t
mpool_mdc_rootid_get(struct mpool *mp, uint64_t *logid1, uint64_t *logid2)
{
    if (ev(!logid1 || !logid2))
        return merr(EINVAL);

    *logid1 = logid_make(0, MCID_CAPACITY, MDC_ROOT_MAGIC);
    *logid2 = logid_make(1, MCID_CAPACITY, MDC_ROOT_MAGIC);

    return 0;
}

static merr_t
mdc_exists(
    struct mpool        *mp,
    uint64_t             logid1,
    uint64_t             logid2,
    bool                *exist)
{
    enum mclass_id mcid;
    int dirfd;
    merr_t err;

    if (!mp || logid1 == logid2 || !exist)
        return merr(EINVAL);

    mcid = logid_mcid(logid1);
    dirfd = mclass_dirfd(mpool_mclass_handle(mp, mcid_to_mclass(mcid)));

    err = mdc_file_exists(dirfd, logid1, logid2, exist);
    if (ev(err))
        return err;

    return 0;
}

merr_t
mpool_mdc_root_init(struct mpool *mp)
{
    uint64_t id[2];
    merr_t err;
    bool exist;

    err = mpool_mdc_rootid_get(mp, &id[0], &id[1]);
    if (ev(err))
        return err;

    err = mdc_exists(mp, id[0], id[1], &exist);
    if (!err && !exist) {
        err = mpool_mdc_alloc(mp, MDC_ROOT_MAGIC, MPOOL_ROOT_LOG_CAP,
                               MP_MED_CAPACITY, &id[0], &id[1]);
        if (ev(err))
            return err;

        err = mpool_mdc_commit(mp, id[0], id[1]);
        if (ev(err))
            return err;
    }

    return err;
}

merr_t
mpool_mdc_root_destroy(struct mpool *mp)
{
    uint64_t id[2];
    merr_t err;

    err = mpool_mdc_rootid_get(mp, &id[0], &id[1]);
    if (ev(err))
        return err;

    err = mpool_mdc_delete(mp, id[0], id[1]);
    if (ev(err))
        return err;

    return 0;
}

merr_t
mpool_mdc_sync2(struct mpool_mdc *mdc)
{
    merr_t err;

    if (ev(!mdc))
        return merr(EINVAL);

    mutex_lock(&mdc->lock);

    err = mdc_file_sync(mdc->mfpa);
    if (err)
        hse_elog(HSE_ERR "mdc %p sync failed, mdc file %p: @@e", err, mdc, mdc->mfpa);

    mutex_unlock(&mdc->lock);

    return err;
}

merr_t
mpool_mdc_rewind(struct mpool_mdc *mdc)
{
    merr_t err;

    if (ev(!mdc))
        return merr(EINVAL);

    mutex_lock(&mdc->lock);

    err = mdc_file_rewind(mdc->mfpa);
    if (err)
        hse_elog(HSE_ERR "mdc %p rewind failed, mdc file %p: @@e", err, mdc, mdc->mfpa);

    mutex_unlock(&mdc->lock);

    return err;
}

merr_t
mpool_mdc_read(struct mpool_mdc *mdc, void *data, size_t len, size_t *rdlen)
{
    merr_t err;
    bool verify = false;

    if (!mdc || !data)
        return merr(EINVAL);

    mutex_lock(&mdc->lock);

    err = mdc_file_read(mdc->mfpa, data, len, rdlen, verify);
    if (err && (merr_errno(err) != EOVERFLOW))
        hse_elog(HSE_ERR "mdc %p read failed, mdc file %p len %lu: @@e",
              err, mdc, mdc->mfpa, len);

    mutex_unlock(&mdc->lock);

    return err;
}

merr_t
mpool_mdc_append(struct mpool_mdc *mdc, void *data, size_t len, bool sync)
{
    merr_t err;

    if (!mdc || !data)
        return merr(EINVAL);

    mutex_lock(&mdc->lock);

    err = mdc_file_append(mdc->mfpa, data, len, sync);
    if (err)
        hse_elog(HSE_ERR "mdc %p append failed, mdc file %p, len %lu sync %d: @@e",
              err, mdc, mdc->mfpa, len, sync);

    mutex_unlock(&mdc->lock);

    return err;
}

merr_t
mpool_mdc_usage(struct mpool_mdc *mdc, size_t *usage)
{
    merr_t err;

    if (!mdc || !usage)
        return merr(EINVAL);

    mutex_lock(&mdc->lock);

    err = mdc_file_usage(mdc->mfpa, usage);
    if (err)
        hse_elog(HSE_ERR "mdc %p usage failed, mdc file %p: @@e", err, mdc, mdc->mfpa);

    mutex_unlock(&mdc->lock);

    return err;
}
