/* SPDX-License-Identifier: Apache-2.0 */
/*
 * Copyright (C) 2015-2021 Micron Technology, Inc.  All rights reserved.
 */

#ifndef HSE_KVDB_PERFC_API_H
#define HSE_KVDB_PERFC_API_H

/* To reduce minor page faults, sort each enumeration such that the hottest
 * counters have the lowest enumerators.
 */

enum kvdb_perfc_sidx_kvdbop {

    PERFC_RA_KVDBOP_KVS_GET,
    PERFC_RA_KVDBOP_KVS_GETB,

    PERFC_RA_KVDBOP_KVS_PUT,
    PERFC_RA_KVDBOP_KVS_PUTB,

    PERFC_RA_KVDBOP_KVDB_TXN_BEGIN,
    PERFC_RA_KVDBOP_KVDB_TXN_COMMIT,
    PERFC_RA_KVDBOP_KVDB_TXN_ABORT,
    PERFC_RA_KVDBOP_KVDB_TXN_ALLOC,
    PERFC_RA_KVDBOP_KVDB_TXN_FREE,
    PERFC_RA_KVDBOP_KVDB_TXN_GET_STATE,

    PERFC_RA_KVDBOP_KVS_CURSOR_CREATE,
    PERFC_RA_KVDBOP_KVS_CURSOR_UPDATE,
    PERFC_RA_KVDBOP_KVS_CURSOR_SEEK,
    PERFC_RA_KVDBOP_KVS_CURSOR_READ,
    PERFC_RA_KVDBOP_KVS_CURSOR_DESTROY,

    PERFC_RA_KVDBOP_KVS_DEL,
    PERFC_RA_KVDBOP_KVS_DELB,

    PERFC_RA_KVDBOP_KVS_PFX_DEL,
    PERFC_RA_KVDBOP_KVS_PFX_DELB,

    PERFC_RA_KVDBOP_KVS_PFXPROBE,

    PERFC_RA_KVDBOP_KVDB_SYNC,

    PERFC_RA_KVDBOP_KVDB_CREATE,
    PERFC_RA_KVDBOP_KVDB_OPEN,
    PERFC_RA_KVDBOP_KVDB_CLOSE,
    PERFC_RA_KVDBOP_KVDB_GET_NAMES,
    PERFC_RA_KVDBOP_KVDB_FREE_NAMES,
    PERFC_RA_KVDBOP_KVDB_KVS_OPEN,
    PERFC_RA_KVDBOP_KVDB_KVS_CLOSE,
    PERFC_RA_KVDBOP_KVDB_KVS_CREATE,
    PERFC_RA_KVDBOP_KVDB_KVS_DROP,
    PERFC_EN_KVDBOP
};

enum kvdb_perfc_sidx_c0skop {
    PERFC_RA_C0SKOP_GET,
    PERFC_LT_C0SKOP_GET,
    PERFC_RA_C0SKOP_PUT,
    PERFC_LT_C0SKOP_PUT,
    PERFC_RA_C0SKOP_DEL,
    PERFC_LT_C0SKOP_DEL,
    PERFC_EN_C0SKOP
};

enum kvdb_perfc_sidx_c0sking {
    PERFC_BA_C0SKING_QLEN,
    PERFC_BA_C0SKING_WIDTH,
    PERFC_DI_C0SKING_KVMSDSIZE,
    PERFC_DI_C0SKING_PREP,
    PERFC_DI_C0SKING_FIN,
    PERFC_DI_C0SKING_THRSR,
    PERFC_DI_C0SKING_MEM,
    PERFC_EN_C0SKING
};

enum kvdb_perfc_sidx_c0metrics { PERFC_BA_C0METRICS_KVMS_CNT, PERFC_EN_C0METRICS };

enum kvdb_perfc_sidx_ctxnop {
    PERFC_BA_CTXNOP_ACTIVE,
    PERFC_RA_CTXNOP_ALLOC,
    PERFC_RA_CTXNOP_BEGIN,
    PERFC_RA_CTXNOP_COMMIT,
    PERFC_LT_CTXNOP_COMMIT,
    PERFC_RA_CTXNOP_ABORT,
    PERFC_RA_CTXNOP_LOCKFAIL,
    PERFC_RA_CTXNOP_FREE,
    PERFC_EN_CTXNOP
};

enum kvdb_perfc_sidx_sp3 {
    PERFC_BA_SP3_SAMP,
    PERFC_BA_SP3_REDUCE,
    PERFC_BA_SP3_LGOOD_CURR,
    PERFC_BA_SP3_LGOOD_TARG,
    PERFC_BA_SP3_LSIZE_CURR,
    PERFC_BA_SP3_LSIZE_TARG,
    PERFC_BA_SP3_RSIZE_CURR,
    PERFC_BA_SP3_RSIZE_TARG,
    PERFC_EN_SP3
};

enum kvdb_perfc_sidx_cnget {
    PERFC_RA_CNGET_GET = 0,
    PERFC_LT_CNGET_GET = 1,
    /* The enum values PERFC_LT_CNGET_GET_L0 to L5 must be sequential */
    PERFC_LT_CNGET_GET_L0 = 2,
    PERFC_LT_CNGET_GET_L1 = 3,
    PERFC_LT_CNGET_GET_L2 = 4,
    PERFC_LT_CNGET_GET_L3 = 5,
    PERFC_LT_CNGET_GET_L4 = 6,
    PERFC_LT_CNGET_GET_L5 = 7,
    PERFC_LT_CNGET_PROBEPFX,
    PERFC_DI_CNGET_DEPTH,
    PERFC_DI_CNGET_NKVSET,
    PERFC_RA_CNGET_MISS,
    PERFC_LT_CNGET_MISS,
    PERFC_RA_CNGET_TOMB,
    PERFC_EN_CNGET
};

enum kvdb_perfc_sidx_kvdb_metrics {
    PERFC_BA_KVDBMETRICS_SEQNO,
    PERFC_BA_KVDBMETRICS_CURHORIZON,
    PERFC_BA_KVDBMETRICS_HORIZON,
    PERFC_BA_KVDBMETRICS_CURCNT,
    PERFC_RA_KVDBMETRICS_CURRETIRED,
    PERFC_RA_KVDBMETRICS_CUREVICTED,
    PERFC_DI_KVDBMETRICS_THROTTLE,
    PERFC_EN_KVDBMETRICS
};

enum kvdb_perfc_sidx_api_throttle {
    PERFC_DI_THSR_CSCHED,
    PERFC_DI_THSR_C0SK,
    PERFC_DI_THSR_MAX,
    PERFC_DI_THSR_MAVG,
    PERFC_EN_THSR
};

enum kvdb_perfc_sidx_throttle_sleep { PERFC_DI_THR_SVAL, PERFC_EN_THR_MAX };

#define CNGET_LMAX PERFC_LT_CNGET_GET_L5

enum kvdb_perfc_compact {
    PERFC_BA_CNCOMP_START,
    PERFC_BA_CNCOMP_FINISH,
    PERFC_RA_CNCOMP_RREQS,
    PERFC_RA_CNCOMP_RBYTES,
    PERFC_RA_CNCOMP_WREQS,
    PERFC_RA_CNCOMP_WBYTES,
    PERFC_DI_CNCOMP_VBCNT,
    PERFC_DI_CNCOMP_VBUTIL,
    PERFC_DI_CNCOMP_VBDEAD,
    PERFC_LT_CNCOMP_TOTAL,
    PERFC_DI_CNCOMP_VGET,
    PERFC_EN_CNCOMP
};

enum kvdb_perfc_cnshape {
    PERFC_BA_CNSHAPE_NODES,
    PERFC_BA_CNSHAPE_AVGLEN,
    PERFC_BA_CNSHAPE_AVGSIZE,
    PERFC_BA_CNSHAPE_MAXLEN,
    PERFC_BA_CNSHAPE_MAXSIZE,
    PERFC_EN_CNSHAPE,
};

enum kvdb_perfc_cncapped {
    PERFC_BA_CNCAPPED_DEPTH,
    PERFC_BA_CNCAPPED_PTSEQ,
    PERFC_BA_CNCAPPED_ACTIVE,
    PERFC_BA_CNCAPPED_NEW,
    PERFC_BA_CNCAPPED_OLD,
    PERFC_EN_CNCAPPED,
};

enum kvdb_perfc_cnmclass {
    PERFC_BA_CNMCLASS_SYNCK_STAGING,
    PERFC_BA_CNMCLASS_SYNCK_CAPACITY,
    PERFC_BA_CNMCLASS_SYNCV_STAGING,
    PERFC_BA_CNMCLASS_SYNCV_CAPACITY,
    PERFC_BA_CNMCLASS_ROOTK_STAGING,
    PERFC_BA_CNMCLASS_ROOTK_CAPACITY,
    PERFC_BA_CNMCLASS_ROOTV_STAGING,
    PERFC_BA_CNMCLASS_ROOTV_CAPACITY,
    PERFC_BA_CNMCLASS_INTK_STAGING,
    PERFC_BA_CNMCLASS_INTK_CAPACITY,
    PERFC_BA_CNMCLASS_INTV_STAGING,
    PERFC_BA_CNMCLASS_INTV_CAPACITY,
    PERFC_BA_CNMCLASS_LEAFK_STAGING,
    PERFC_BA_CNMCLASS_LEAFK_CAPACITY,
    PERFC_BA_CNMCLASS_LEAFV_STAGING,
    PERFC_BA_CNMCLASS_LEAFV_CAPACITY,
    PERFC_EN_CNMCLASS,
};

enum kvdb_perfc_sidx_cursorcache {
    PERFC_RA_CC_HIT,
    PERFC_RA_CC_MISS,
    PERFC_RA_CC_SAVE,
    PERFC_RA_CC_SAVEFAIL,
    PERFC_RA_CC_RESTFAIL,
    PERFC_RA_CC_UPDATE,
    PERFC_BA_CC_EAGAIN_C0,
    PERFC_BA_CC_EAGAIN_CN,
    PERFC_BA_CC_INIT_CREATE_C0,
    PERFC_BA_CC_INIT_UPDATE_C0,
    PERFC_BA_CC_INIT_CREATE_CN,
    PERFC_BA_CC_INIT_UPDATE_CN,
    PERFC_BA_CC_UPDATED_C0,
    PERFC_BA_CC_UPDATED_CN,
    PERFC_EN_CC
};

enum kvdb_perfc_sidx_cursordist {
    PERFC_LT_CD_SAVE,
    PERFC_LT_CD_RESTORE,
    PERFC_LT_CD_CREATE_CN,
    PERFC_LT_CD_UPDATE_CN,
    PERFC_LT_CD_CREATE_C0,
    PERFC_LT_CD_UPDATE_C0,
    PERFC_DI_CD_READPERSEEK,
    PERFC_DI_CD_TOMBSPERPROBE,
    PERFC_DI_CD_ACTIVEKVSETS_CN,
    PERFC_EN_CD
};

/* "PKVSL" stands for Public KVS interface Latencies" */
enum kvdb_perfc_sidx_pkvsl {
    PERFC_LT_PKVSL_KVS_PUT,
    PERFC_LT_PKVSL_KVS_GET,
    PERFC_LT_PKVSL_KVS_DEL,

    PERFC_LT_PKVSL_KVS_PFX_PROBE,
    PERFC_LT_PKVSL_KVS_PFX_DEL,

    PERFC_LT_PKVSL_KVS_CURSOR_CREATE,
    PERFC_LT_PKVSL_KVS_CURSOR_UPDATE,
    PERFC_LT_PKVSL_KVS_CURSOR_SEEK,
    PERFC_LT_PKVSL_KVS_CURSOR_READFWD,
    PERFC_LT_PKVSL_KVS_CURSOR_READREV,
    PERFC_LT_PKVSL_KVS_CURSOR_DESTROY,
    PERFC_LT_PKVSL_KVS_CURSOR_FULL,
    PERFC_LT_PKVSL_KVS_CURSOR_INIT,

    PERFC_EN_PKVSL,
};

/* "PKVDBL" stands for Public KVDB interface Latencies" */
enum kvdb_perfc_sidx_pkvdbl {
    PERFC_LT_PKVDBL_KVDB_TXN_BEGIN,
    PERFC_LT_PKVDBL_KVDB_TXN_COMMIT,
    PERFC_LT_PKVDBL_KVDB_TXN_ABORT,

    PERFC_SL_PKVDBL_KVDB_SYNC,

    PERFC_LT_PKVDBL_KVDB_MAKE,
    PERFC_LT_PKVDBL_KVDB_DROP,
    PERFC_LT_PKVDBL_KVDB_OPEN,
    PERFC_LT_PKVDBL_KVS_OPEN,

    PERFC_EN_PKVDBL,
};

/* STS Queues */
enum kvdb_perfc_sidx_sts_queues {

    PERFC_BA_STS_QDEPTH,

    PERFC_RA_STS_JOBS,
    PERFC_BA_STS_JOBS_RUN,

    PERFC_BA_STS_WORKERS,
    PERFC_BA_STS_WORKERS_IDLE,

    PERFC_EN_STS,
};

extern struct perfc_set kvdb_metrics_pc;

#endif /* HSE_KVDB_PERFC_API_H */
