/* SPDX-License-Identifier: Apache-2.0 */
/*
 * Copyright (C) 2015-2021 Micron Technology, Inc.  All rights reserved.
 */

#ifndef MPOOL_MDC_H
#define MPOOL_MDC_H

#include <hse_util/mutex.h>

struct media_class;
struct mdc_file;
struct io_ops;

struct mpool_mdc {
	struct mutex           lock;
	struct mdc_file       *mfp1;
	struct mdc_file       *mfp2;
	struct mdc_file       *mfpa;

	struct media_class    *mc;
	struct mpool          *mp;
};

struct media_class *
mdc_mclass_get(struct mpool_mdc *mdc);

#endif /* MPOOL_MDC_H */
