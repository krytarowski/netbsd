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
 * @(#)$RCSfile: bs_inmem_map.h,v $ $Revision: 1.1.32.3 $ (DEC) $Date: 2006/08/29 16:35:42 $
 */

#ifndef _BS_INMEM_MAP_
#define _BS_INMEM_MAP_

typedef struct {
	uint32_t pageOffset;
	uint32_t pageCnt;
	vdIndexT volIndex;
	uint32_t blkOffset;
}      bsXtntDescT;
/*
 * bsInMemXtntDescIdT - Identifies an extent descriptor in an in-memory extent
 * map.  This is used when sequentially fetching extent descriptors from an
 * in-mem extent map.
 */

typedef struct bsInMemXtntDescId {
	uint32_t subXtntMapIndex;
	uint32_t xtntDescIndex;
}                 bsInMemXtntDescIdT;
statusT
imm_create_xtnt_map(
    uint32_t blksPerPage,	/* in */
    domainT * domain,		/* in */
    uint32_t maxCnt,		/* in */
    uint32_t termPage,		/* in */
    vdIndexT termVdIndex,	/* in */
    bsInMemXtntMapT ** newXtntMap	/* out */
);

statusT
imm_init_xtnt_map(
    bsInMemXtntMapT * xtntMap,	/* in */
    uint32_t blksPerPage,	/* in */
    domainT * domain,		/* in */
    uint32_t maxCnt,		/* in */
    uint32_t termPage,		/* in */
    vdIndexT termVdIndex	/* in */
);

statusT
imm_extend_xtnt_map(
    bsInMemXtntMapT * xtntMap
);
statusT
imm_get_xtnt_map_size(
    bsInMemXtntMapT * xtntMap,	/* in */
    int *size			/* out */
);

statusT
imm_compress_xtnt_map(
    uint32_t segmentSize,	/* in */
    uint32_t cnt,		/* in */
    bsInMemXtntMapT * xtntMap,	/* in */
    bsXtntMapTypeT bsXtntMapType,	/* in */
    bsInMemXtntMapT ** newXtntMap	/* out */
);
void
imm_get_first_xtnt_desc(
    bsInMemXtntMapT * xtntMap,	/* in */
    bsInMemXtntDescIdT * xtntDescId,	/* out */
    bsXtntDescT * xtntDesc	/* out */
);

void
imm_get_xtnt_desc(
    uint32_t pageOffset,		/* in */
    bsInMemXtntMapT * xtntMap,	/* in */
    int updateFlg,		/* in */
    bsInMemXtntDescIdT * xtntDescId,	/* out */
    bsXtntDescT * xtntDesc	/* out */
);

void
imm_get_next_xtnt_desc(
    bsInMemXtntMapT * xtntMap,	/* in */
    bsInMemXtntDescIdT * xtntDescId,	/* in/out */
    bsXtntDescT * xtntDesc	/* out */
);

void
imm_delete_xtnt_map(
    bsInMemXtntMapT * xtntMap	/* in */
);

void
imm_delete_sub_xtnt_maps(
    bsInMemXtntMapT * xtntMap	/* in */
);

statusT
imm_copy_sub_xtnt_map(
    bfAccessT * bfap,		/* in */
    bsInMemXtntMapT * xtntMap,	/* in */
    bsInMemSubXtntMapT * oldSubXtntMap,	/* in */
    bsInMemSubXtntMapT * newSubXtntMap	/* in */
);

statusT
imm_init_sub_xtnt_map(
    bsInMemSubXtntMapT * subXtntMap,	/* in */
    uint32_t pageOffset,		/* in */
    uint32_t pageCnt,		/* in */
    vdIndexT vdIndex,		/* in */
    bfMCIdT mcellId,		/* in */
    uint32_t type,		/* in */
    uint32_t onDiskMaxCnt,	/* in */
    uint32_t maxCnt		/* in */
);

statusT
imm_extend_sub_xtnt_map(
    bsInMemSubXtntMapT * subXtntMap	/* in */
);

statusT
imm_load_sub_xtnt_map(
    bfAccessT * bfap,		/* in */
    bsInMemXtntMapT * xtntMap,	/* in */
    bsInMemSubXtntMapT * subXtntMap	/* in */
);

void
imm_unload_sub_xtnt_map(
    bsInMemSubXtntMapT * subXtntMap	/* in */
);

statusT
imm_copy_xtnt_descs(
    vdIndexT copyVdIndex,	/* in */
    bsXtntT * copyBsXA,		/* in */
    uint32_t copyCnt,		/* in */
    int copyHoleFlag,		/* in */
    bsInMemXtntMapT * xtntMap,	/* in */
    bsXtntMapTypeT bsXtntMapType/* in */
);

void
imm_split_desc(
    uint32_t pageOffset,		/* in */
    uint32_t pageSize,		/* in */
    bsXtntT * srcDesc,		/* in */
    bsXtntT * part1Desc,	/* in/modified */
    bsXtntT * part2Desc		/* in/modified */
);

statusT
imm_replace_xtnt_desc(
    uint32_t pageSize,		/* in */
    bsInMemXtntMapT * xtntMap,	/* in */
    bsInMemXtntMapT * subXtntMap/* in */
);

statusT
imm_get_alloc_page_cnt(
    bsInMemXtntMapT * xtntMap,	/* in */
    uint32_t bfPageOffset,	/* in */
    uint32_t bfPageCnt,		/* in */
    uint32_t * allocPageCnt	/* out */
);

uint32_t
imm_get_next_page(
    bsInMemXtntT * xtnts	/* in */
);

uint32_t
imm_get_hole_size(
    uint32_t pageOffset,		/* in */
    bsInMemXtntT * xtnts	/* in */
);

statusT
imm_get_page_type(
    bsInMemXtntT * xtnts,	/* in */
    uint32_t pageOffset		/* in */
);

void
imm_set_next_valid_copy_page(
    bsInMemXtntT * xtnts,	/* in */
    uint32_t bfPageOffset	/* in */
);

statusT
imm_page_to_sub_xtnt_map(
    uint32_t pageOffset,		/* in */
    bsInMemXtntMapT * xtntMap,	/* in */
    int updateFlg,		/* in */
    uint32_t * index		/* out */
);

statusT
imm_page_to_xtnt(
    uint32_t pageOffset,		/* in */
    bsInMemSubXtntMapT * subXtntMap,	/* in */
    int updateFlg,		/* in */
    int permHoleFlg,		/* in */
    uint32_t * index		/* out */
);

statusT
imm_merge_xtnt_map(
    bfAccessT * bfap,		/* in */
    bsInMemXtntMapT * src1XtntMap,	/* in */
    bfAccessT * cloneBfap,	/* in */
    bsInMemXtntMapT * src2XtntMap,	/* in */
    bsInMemXtntMapT ** newXtntMap	/* out */
);

statusT
overlay_xtnt_map(bfAccessT * bfap,	/* in */
    uint32_t stripeIndex,	/* in */
    bsInMemXtntMapT * baseXtntMap,	/* in */
    bsInMemXtntMapT * overXtntMap,	/* in */
    bsInMemXtntMapT ** retModXtntMap,	/* out */
    bsInMemXtntMapT ** retReplXtntMap,	/* out */
    ftxHT ftxH);

statusT
imm_overlay_xtnt_map(
    bsInMemXtntMapT * baseXtntMap,	/* in */
    bsInMemXtntMapT * overXtntMap,	/* in */
    bsXtntMapTypeT bsXtntMapType,	/* in */
    bsInMemXtntMapT ** retNewXtntMap,	/* out */
    bsInMemXtntMapT ** retReplXtntMap	/* out */
);

statusT
imm_copy_xtnt_map(
    bfAccessT * bfap,		/* in */
    bsInMemXtntMapT * srcXtntMap,	/* in */
    bsInMemXtntMapT ** dstXtntMap	/* out */
);

statusT
imm_remove_page_map(
    uint32_t bfPageOffset,	/* in */
    uint32_t bfPageCnt,		/* in */
    bsInMemXtntMapT * xtntMap,	/* in */
    bsXtntMapTypeT bsXtntMapType,	/* in */
    uint32_t * remPageOffset,	/* out */
    uint32_t * remPageCnt	/* out */
);

void
imm_compress_subxtnt_map(
    bsInMemXtntT * xtnts,	/* in */
    int bfPageSz		/* in */
);

#endif				/* _BS_INMEM_MAP_ */
