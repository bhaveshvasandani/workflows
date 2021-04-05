/* SPDX-License-Identifier: Apache-2.0 */
/*
 * Copyright (C) 2015-2021 Micron Technology, Inc.  All rights reserved.
 */

#define _LARGEFILE64_SOURCE

#ifndef IOV_MAX
#include <sys/uio.h>

#ifdef __IOV_MAX
#define IOV_MAX __IOV_MAX
#endif
#endif

#ifndef IOV_MAX
/* This is for IOV_MAX */
#define __need_IOV_MAX /* This way stdio_lim.h defines IOV_MAX */
#include <bits/stdio_lim.h>
#endif

#ifndef IOV_MAX
#error "Neither __IOV_MAX nor IOV_MAX is defined"
#endif

#include <hse_util/event_counter.h>
#include <hse_util/minmax.h>

#include "io.h"

static size_t
iolen(const struct iovec *iov, int cnt)
{
    size_t len = 0;

    while (cnt-- > 0)
        len += iov[cnt].iov_len;

    return len;
}

merr_t
io_sync_read(int fd, off_t off, const struct iovec *iov, int iovcnt, int flags)
{
    const struct iovec *curiov;

    int left;

    curiov = iov;
    left = iovcnt;

    while (left > 0) {
        size_t cc, len;
        int cnt;

        cnt = min_t(int, left, IOV_MAX);

        len = iolen(curiov, cnt);

        /* Pass flags to preadv2(). Not available on fc25. */
        cc = preadv(fd, curiov, cnt, off);
        if (ev(cc != len))
            return merr((cc == -1) ? errno : EIO);

        off += cc;

        left -= cnt;
        curiov += cnt;
    }

    return 0;
}

merr_t
io_sync_write(int fd, off_t off, const struct iovec *iov, int iovcnt, int flags)
{
    const struct iovec *curiov;

    int left;

    curiov = iov;
    left = iovcnt;

    while (left > 0) {
        size_t cc, len;
        int cnt;

        cnt = min_t(int, left, IOV_MAX);

        len = iolen(curiov, cnt);

        /* Pass flags to pwritev2(). Not available on fc25. */
        cc = pwritev(fd, curiov, cnt, off);
        if (ev(cc != len))
            return merr((cc == -1) ? errno : EIO);

        off += cc;

        left -= cnt;
        curiov += cnt;
    }

    return 0;
}

const struct io_ops io_sync_ops = {
    .read = io_sync_read,
    .write = io_sync_write,
};
