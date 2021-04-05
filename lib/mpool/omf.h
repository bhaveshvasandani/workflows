/* SPDX-License-Identifier: Apache-2.0 */
/*
 * Copyright (C) 2015-2021 Micron Technology, Inc.  All rights reserved.
 */

#ifndef MPOOL_OMF_H
#define MPOOL_OMF_H

#include <hse_util/omf.h>
#include <hse_util/hse_err.h>

struct mdc_loghdr;
struct mdc_rechdr;
struct mblock_metahdr;
struct mblock_filehdr;

/*
 * MDC OMF
 */

struct mdc_loghdr_omf {
    __le32 lh_vers;
    __le32 lh_magic;
    __le32 lh_rsvd;
    __le64 lh_gen;
    __le32 lh_crc;
} __packed;

/* Define set/get methods for mdc_loghdr_omf */
OMF_SETGET(struct mdc_loghdr_omf, lh_vers, 32);
OMF_SETGET(struct mdc_loghdr_omf, lh_magic, 32);
OMF_SETGET(struct mdc_loghdr_omf, lh_rsvd, 32);
OMF_SETGET(struct mdc_loghdr_omf, lh_gen, 64);
OMF_SETGET(struct mdc_loghdr_omf, lh_crc, 32);


struct mdc_rechdr_omf {
    __le32 rh_crc;
    __le64 rh_size;
} __packed;

/* Define set/get methods for mdc_rechdr_omf */
OMF_SETGET(struct mdc_rechdr_omf, rh_crc, 32);
OMF_SETGET(struct mdc_rechdr_omf, rh_size, 64);

merr_t
omf_mdc_loghdr_pack_htole(struct mdc_loghdr *lh, char *outbuf);

merr_t
omf_mdc_loghdr_unpack_letoh(struct mdc_loghdr *lh, const char *inbuf);

size_t
omf_mdc_loghdr_len(void);

void
omf_mdc_rechdr_unpack_letoh(struct mdc_rechdr *rh, const char *inbuf);

size_t
omf_mdc_rechdr_len(void);

/*
 * Mblock Meta OMF
 */

struct mblock_metahdr_omf {
    __le32 mh_vers;
    __le32 mh_magic;
    u8     mh_mcid;
    u8     mh_fcnt;
    u8     mh_blkbits;
    u8     mh_mcbits;
} __packed;

/* Define set/get methods for mblock_metahdr_omf */
OMF_SETGET(struct mblock_metahdr_omf, mh_vers, 32);
OMF_SETGET(struct mblock_metahdr_omf, mh_magic, 32);
OMF_SETGET(struct mblock_metahdr_omf, mh_mcid, 8);
OMF_SETGET(struct mblock_metahdr_omf, mh_fcnt, 8);
OMF_SETGET(struct mblock_metahdr_omf, mh_blkbits, 8);
OMF_SETGET(struct mblock_metahdr_omf, mh_mcbits, 8);


struct mblock_filehdr_omf {
    u8     fh_fileid;
    u8     fh_rsvd1;
    __le16 fh_rsvd2;
    __le32 fh_rsvd3;
} __packed;

/* Define set/get methods for mblock_filehdr_omf */
OMF_SETGET(struct mblock_filehdr_omf, fh_fileid, 8);
OMF_SETGET(struct mblock_filehdr_omf, fh_rsvd1, 8);
OMF_SETGET(struct mblock_filehdr_omf, fh_rsvd2, 16);
OMF_SETGET(struct mblock_filehdr_omf, fh_rsvd3, 32);


struct mblock_oid_omf {
    __le64 mblk_id;
} __packed;

/* Define set/get methods for mblock_oid_omf */
OMF_SETGET(struct mblock_oid_omf, mblk_id, 64);

void
omf_mblock_metahdr_pack_htole(struct mblock_metahdr *mh, char *outbuf);

void
omf_mblock_metahdr_unpack_letoh(struct mblock_metahdr *mh, const char *inbuf);

void
omf_mblock_filehdr_pack_htole(struct mblock_filehdr *fh, char *outbuf);

void
omf_mblock_filehdr_unpack_letoh(struct mblock_filehdr *fh, const char *inbuf);
#endif /* MPOOL_OMF_H */
