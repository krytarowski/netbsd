/****************************************************************************
 *                                                                          *
 *  (C) DIGITAL EQUIPMENT CORPORATION 1988, 1989, 1990, 1991                *
 */
/* =======================================================================
 *   (c) Copyright Hewlett-Packard Development Company, L.P., 2008
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of version 2 the GNU General Public License as
 *   published by the Free Software Foundation.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * =======================================================================
 */
/*
 * @(#)$RCSfile: bs_stg.h,v $ $Revision: 1.1.35.2 $ (DEC) $Date: 2006/08/28 17:41:07 $
 */

#ifndef _BS_STG_
#define _BS_STG_

#include <fs/msfs/bs_public.h>

/*****************************************
 *
 * The following #define will cause stress on the RBMT
 * by disabling the storage reservtion code and forcing
 * the BMT to be extended by a single page at a time.
 *
 * Beware that existing domains may end up with a
 * less than optimal BMT that can not be removed if much
 * storage is added.
 */

/* #define ADVFS_RBMT_STRESS */

int
stg_add_stg(
    ftxHT ftxH,			/* in */
    struct bfAccess * bfap,	/* in */
    unsigned long bfPageOffset,	/* in */
    unsigned long bfPageCnt,	/* in */
    int allowOverlapFlag,	/* in */
    uint32_t * allocPageCnt	/* out */
);

int
stg_add_stg_no_cow(
    ftxHT ftxH,			/* in */
    struct bfAccess * bfap,	/* in */
    unsigned long bfPageOffset,	/* in */
    unsigned long bfPageCnt,	/* in */
    int allowOverlapFlag,	/* in */
    uint32_t * allocPageCnt	/* out */
);

int
stg_set_alloc_disk(
    struct bfAccess * bfap,	/* in */
    vdIndexT curVdIndex,	/* in */
    uint32_t pageCntNeeded,	/* in */
    int forceFlag		/* in */
);

int
stg_alloc_from_svc_class(
    bfAccessT * bfAccess,	/* in */
    serviceClassT reqServices,	/* in */
    serviceClassT optServices,	/* in */
    bfTagT bfSetTag,		/* in */
    bfTagT bfTag,		/* in */
    uint32_t bfPageOffset,	/* in */
    uint32_t bfPageCnt,		/* in */
    int bfPageSize,		/* in */
    bsInMemXtntMapT * xtntMap,	/* in */
    bsAllocHintT * alloc_hint,	/* in/out */
    ftxHT parentFtx,		/* in */
    uint32_t * allocPageCnt	/* out */
);

int
cp_stg_alloc_from_svc_class(
    bfAccessT * bfap,		/* in */
    uint32_t bfPageOffset,	/* in */
    uint32_t bfPageCnt,		/* in */
    int32_t stg_type,
    bsInMemXtntMapT * xtntMap,	/* in */
    uint32_t startPage,		/* in */
    bsAllocHintT alloc_hint,	/* in */
    uint32_t * copyXferSize	/* out */
);

int
stg_alloc_from_one_disk(
    bfAccessT * bfAccess,	/* in */
    vdIndexT bfVdIndex,		/* in */
    bfTagT bfSetTag,		/* in */
    bfTagT bfTag,		/* in */
    uint32_t bfPageOffset,	/* in */
    uint32_t bfPageCnt,		/* in */
    int bfPageSize,		/* in */
    bsInMemXtntMapT * xtntMap,	/* in */
    bsAllocHintT * alloc_hint,	/* in/out */
    ftxHT parentFtx,		/* in */
    uint32_t * allocPageCnt	/* out */
);

int
cp_stg_alloc_from_one_disk(
    bfAccessT * bfap,		/* in */
    vdIndexT bfVdIndex,		/* in */
    uint32_t bfPageOffset,	/* in */
    uint32_t bfPageCnt,		/* in */
    int32_t stg_type,
    bsInMemXtntMapT * xtntMap,	/* in */
    uint32_t startPage,		/* in */
    uint32_t * allocPageCnt,	/* out */
    uint64_t dstBlkOffset,	/* in */
    bsAllocHintT alloc_hint	/* in */
);

int
xfer_stg(
    bfAccessT * bfap,		/* file to add storage to */
    u_long bfPageOffset,	/* offset to add storage */
    u_long bfPageCnt,		/* number of pages to add */
    uint32_t startblk,		/* storage in hand to add */
    vdIndexT vdIndex,		/* storage is on this disk */
    ftxHT parentFtx
);

int
stg_alloc_new_mcell(
    bfAccessT * bfAccess,	/* in */
    bfTagT bfSetTag,		/* in */
    bfTagT bfTag,		/* in */
    vdIndexT newVdIndex,	/* in */
    ftxHT parentFtx,		/* in */
    bfMCIdT * newMcellId,	/* out */
    int forceFlag		/* in */
);

int
stg_remove_stg_start(
    bfAccessT * bfAccess,	/* in */
    uint32_t pageOffset,		/* in */
    uint32_t pageCnt,		/* in */
    int relQuota,		/* in */
    ftxHT parentFtx,		/* in */
    uint32_t * delCnt,		/* out */
    void **delList,		/* out */
    int32_t doCow		/* in */
);

void
stg_remove_stg_finish(
    domainT * dmnp,		/* in */
    uint32_t delCnt,		/* in */
    void *delList		/* in */
);

int
page_is_mapped_local(
    bfAccessT * bfap,
    uint32_t pg,
    uint32_t * nextPage,		/* out */
    int permHoleFlg,		/* in */
    vdIndexT * vdIndex,		/* out */
    uint32_t * vdBlk,		/* out */
    uint32_t * start_pg,		/* out */
    int load_map
);

#endif				/* BS_STG */
