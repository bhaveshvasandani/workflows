/* SPDX-License-Identifier: Apache-2.0 */
/*
 * Copyright (C) 2021 Micron Technology, Inc.  All rights reserved.
 */
#ifndef WAL_INTERNAL_H
#define WAL_INTERNAL_H

#include <hse_ikvdb/wal.h>
#include <hse_ikvdb/limits.h>

#define WAL_DUR_MS_MAX         (1000)
#define WAL_DUR_MS_MIN         (10)

#define WAL_DUR_BYTES_MAX      (100 << 20)
#define WAL_DUR_BYTES_MIN      (16 << 20)

#define WAL_MDC_CAPACITY       (1 << 30)
#define WAL_MAGIC              (0xabcdabcd)
#define WAL_FILE_SIZE_BYTES    (((HSE_C0_INGEST_SZ_MAX * 4) / 10) << 20)

#define MSEC_TO_NSEC(_ms)      (NSEC_PER_SEC / MSEC_PER_SEC * (_ms))
#define NSEC_TO_MSEC(_ns)      ((_ns) / (NSEC_PER_SEC / MSEC_PER_SEC))

#define WAL_NODE_MAX           (4)
#define WAL_BPN_MAX            (2)
#define WAL_BUF_MAX            (WAL_NODE_MAX * WAL_BPN_MAX)

struct wal_minmax_info {
    u64 min_seqno;
    u64 max_seqno;
    u64 min_gen;
    u64 max_gen;
    u64 min_txid;
    u64 max_txid;
};

struct wal_iocb {
    void *cbarg;
    void (*iocb)(void *cbarg, merr_t err);
};

struct wal;
struct mpool;

void
wal_dur_params_get(struct wal *wal, uint32_t *dur_ms, uint32_t *dur_bytes);

void
wal_dur_params_set(struct wal *wal, uint32_t dur_ms, uint32_t dur_bytes);

uint64_t
wal_reclaim_gen_get(struct wal *wal);

void
wal_reclaim_gen_set(struct wal *wal, uint64_t rgen);

uint32_t
wal_version_get(struct wal *wal);

void
wal_version_set(struct wal *wal, uint32_t version);

struct mpool *
wal_mpool_get(struct wal *wal);

#endif /* WAL_INTERNAL_H */
