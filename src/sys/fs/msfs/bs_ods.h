/****************************************************************************
 *                                                                          *
 *  (C) DIGITAL EQUIPMENT CORPORATION 1990, 1991, 1992, 1993                *
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
 * @(#)$RCSfile: bs_ods.h,v $ $Revision: 1.1.69.2 $ (DEC) $Date: 2006/01/03 18:55:01 $
*/

#ifndef BS_ODS
#define BS_ODS

/*
 * "Include" section
 */
#include <sys/param.h>
#include <sys/types.h>

/* Defines */

/****************************************************************/
/**** MSFS_RESERVED_BLKS defines the first block on       *******/
/**** a disk that MegaSafe can use.  Given that the       *******/
/**** first 16 blocks of a disk contain the disk label    *******/
/**** and the UFS boot blocks, this should be set at      *******/
/**** least to 16.                                        *******/
/****                                                     *******/
/**** Blks              Usage                             *******/
/**** ---------------   -------------------------------   *******/
/****  0 to 15          Disk label, UFS boot blocks       *******/
/**** 16 to 19          Fake super block                  *******/
/**** 20 to 31          Unused                            *******/
/**** 64 to 95          Real boot blocks (MISC rsvd mcell *******/
/****************************************************************/

#define MSFS_RESERVED_BLKS 32

#define MSFS_FAKE_SB_BLK   16	/* fake super block starts at blk 16 */
#define MSFS_MAGIC_OFFSET  1372	/* byte offset into super block */
#define MSFS_MAGIC      0x11081953	/* must find this number at above
					 * offset */

/****************************************************************/
/****  Structure version constant - change this whenever  *******/
/****  an on-disk structure is modified *************************/
/****                                                     *******/
/****  version 1 was used for field test only *******************/
/****  version 2 was used for advfs V1.0 & V2.0 *****************/
/****  version 3 was introduced in advfs V3.0 due to a **********/
/****            change in the frag bitfile format **************/
/****  version 4 was introduced in advfs V5.0 to support ********/
/****            larger quota fields ****************************/
/****************************************************************/

/****************************************************************/
/**** These constants define the allowable on-disk versions  ****/
/**** for newly-created domains.                             ****/
/****************************************************************/
#define FIRST_VALID_FS_VERSION 3
#define LAST_VALID_FS_VERSION  4

/****************************************************************/
/****                Version-dependent macros                ****/
/****                                                        ****/
/****  These macros are used to determine whether certain    ****/
/****  features are in use.  Since these featured are tied   ****/
/****  to certain on-disk changes, the on-disk version is    ****/
/****  used in the determination of whether the feature is   ****/
/****  in use or not.                                        ****/
/****************************************************************/

/*
 * The first on-disk version that supports large (8 byte) quotas
 */
#define FIRST_LARGE_QUOTAS_VERSION 4

/*
 * The first on-disk version that uses the RBMT metadata file.
 */
#define FIRST_RBMT_VERSION 4

/*
 * The first on-disk version that uses a notation in clone
 * extent maps to denote a hole in the original file rather
 * than copying zero-filled pages to the clone when holes
 * in the original file are written to.
 */
#define FIRST_PERM_HOLE_XTNTS_VERSION 4

/*
 * The first on-disk version that supports extent information in
 * the primary mcell of non-reserved files.
 */
#define FIRST_XTNT_IN_PRIM_MCELL_VERSION 4

/*
 * The first on-disk version that supports directory indexing.
 */
#define FIRST_INDEXED_DIRECTORIES_VERSION 4

/*
 * The first on-disk version that supports numbered property lists,
 * as well as numbered segments within the property lists
 */
#define FIRST_NUMBERED_PROPLIST_VERSION 4

/*
 * The first on-disk version that supports the modification
 * of the xor fields in the SBM pages under transaction control.
 */
#define FIRST_VALID_SBM_XOR_VERSION 4

/*
 * The first on-disk version that supports the new starting
 * frag slots in order to minimize page crossings.
 *
 */
#define FIRST_MIN_FRAG_CROSSINGS_VERSION 4

/*
 * V4 domains now support 65533 subdirectories.  Older domains only
 * support 32767 subdirectories.
 */
#define FIRST_64K_LINK_MAX_VERSION 4

/*
 * Crash recovery is always run on V4 domains.  Crash recovery is run
 * on older domains when the domain state is not DISMOUNTED or VIRGIN.
 */
#define  FIRST_ALWAYS_RUN_RECOVERY_VERSION 4

/*
 * A tag map is in use if the high bit of the sequence number is set.
 */
#define BS_TD_IN_USE  0x8000
/*
 * When the sequence number reaches this value, the slot
 * is permanently unavailable.
 */
#define BS_TD_DEAD_SLOT    0x07FFF

/*
 * number of pages to expand the tag dir by when a new extent is needed
 */
#define BS_TD_XPND_PGS 8

/*
 * Bitfile Set Flags
 *  The ondisk flags are defined here (BFS_OD_...)
 *  The inmemory flags are defined in bs_bitfile_sets.h (BFS_IM_...)
 */

#define BFS_OD_OUT_OF_SYNC 0x0001	/* Clone could not allocate storage
					 * for a copy-on-write, so it is now
					 * out-of-sync with the original
					 * fileset. */

#define BFS_OD_NOFRAG      0x0002	/* Disable the frag file */


#define BFS_OD_OBJ_SAFETY  0x0010	/* enables/disables forcing zeroed
					 * newly allocated storage in a file
					 * to disk before allowing the file to
					 * have the storage. */

/*
 * The size of pl_num, pl_seg within bsPropListPageT
 */

#define NUM_SEG_SIZE (2*sizeof(uint32_t))

/*
 * PROPERTY LIST DATA RECORD
 */
#define BSR_PROPLIST_DATA 20

#define BSR_PROPLIST_DATA_SIZE BS_USABLE_MCELL_SPACE

#define BSR_PL_MAX_SMALL (BSR_PROPLIST_DATA_SIZE/2)


/*
 * PROPERTY LIST HEADER RECORD
 */
#define BSR_PROPLIST_HEAD 19
#define BSR_PROPLIST_HEAD_SIZE (sizeof(uint64_t)+4*sizeof(uint32_t))
#define BSR_PROPLIST_HEAD_SIZE_V3 (sizeof(uint64_t)+2*sizeof(uint32_t))

/* flags */
/* XXX: Verify sanity of int size 32-64 bit */
#define BSR_PL_LARGE    0x0100000000000000
#define BSR_PL_DELETED  0x0200000000000000
#define BSR_PL_PAGE     0x0400000000000000
#define BSR_PL_RESERVED 0xFF00000000000000

/* Enums */

typedef enum bfdVersion {
	BFD_ODS_LAST_VERSION = 4/* NOTE **** only 5 bits (< 32) */
} bfdVersionT;


#define RBMT_THERE(dp)  (((dp)->dmnVersion) >= FIRST_RBMT_VERSION)

/******************************************************************************
 * Basic Bitfile M(ap/eta) Table (BMT) structures.
 * The BMT bitfile has a 3 level hierarchical structure:
 *  - BMT pages
 *  - BMT cells
 *  - BMT records
 *
 * Pages are an integral number of blocks with a fixed header
 * containing a log sequence number (LSN) which coordinates updates
 * with log records that describe the update.  Pages are the unit
 * of buffer access/update (read/write).  Pages contain a fixed
 * number of fixed length cells.
 *
 * Cells are the unit of storage assignment within the BMT.  A cell
 * is assigned to serve a specific bitfile.  A cell may be linked
 * to additional cells in a simple singly linked list.  A cell has a
 * fixed header with its assigned tag and a link to additional cells.
 * After the fixed header is a sequence of variable length recorbs.
 *
 * Records are typed, variable length units that contain various
 * structs such as mapping extents and bitfile attributes.
 *
 * A NIL record terminates the record list within a cell.
 *
 * Pages are 8192 = 8 + (31 * 264 byte cells).
 *
 ******************************************************************************/

/*
 * bsMRT - mcell record header
 *
 * Each record in an mcell has preceeded by this record header.
 *
 * Note that records must start on an integer boundary.  The bCnt
 * in each record header is not rounded up but whenever it is used
 * to calculate the location of the next record header then it is
 * rounded up to the next integer boundary.
 */

typedef struct bsMR {
	uint32_t bCnt:16;	/* Count of bytes in record */
	uint32_t type:8;		/* Type of structure contained by record */
	uint32_t version:8;	/* Version of the record's type */
} bsMRT;


extern uint16_t bsNilVdIndex;
extern bfMCIdT bsNilMCId;

#define BSC_R_SZ \
    (292 - sizeof(bfMCIdT) - (2*sizeof(bfTagT)) - (2*sizeof(uint16_t)))

#define BS_USABLE_MCELL_SPACE   (BSC_R_SZ - 2*sizeof( bsMRT ))

/*
 * bsMCT
 *
 * Bitfile metadata cell (mcell).  Used to store bitfile metadata
 * like attributes, extent maps, etc.  The metadata is stored in
 * typed records which are kept in the 'bsMR0' byte array.
 */

typedef struct bsMC {
	bfMCIdT nextMCId;	/* Link to next mcell */
	uint16_t nextVdIndex;	/* vd index of next mcell */
	uint16_t linkSegment;	/* segment in link, starts at zero */
	bfTagT tag;		/* Tag this mcell is assigned to */
	bfTagT bfSetTag;	/* tag of this bitfile's bf set dir */
	char bsMR0[BSC_R_SZ];	/* Records */
}    bsMCT;
/* on 12-Apr-1993 BSPG_CELLS equals 28 */

#define BSPG_CELLS \
  ((ADVFS_PGSZ - (3 * sizeof(uint32_t) + sizeof(bfMCIdT))) / sizeof(struct bsMC))

/* Test the validity of a mcell number. */
/* "Return" E_RANGE if the mcell number is invalid. Else "return" EOK. */
/* Proper range test depends on cn being an unsigned number. */
#define TEST_CELL(cn) ( (cn) >= BSPG_CELLS ? E_RANGE : EOK )

/* Last mcell on RBMT page reserved for RBMT itself */
#define RBMT_RSVD_CELL (BSPG_CELLS - 1)

/* on 6-Nov-1991 BSPG_FILLER equals 16 */

#define BSPG_FILLER \
  (ADVFS_PGSZ - (3 * sizeof(uint32_t) + sizeof(bfMCIdT) + \
   BSPG_CELLS * sizeof(struct bsMC)))

typedef struct bsMPg {
	bfMCIdT nextfreeMCId;	/* Next free MCId on the page */
	uint32_t nextFreePg;	/* Next page in the mcell free list */
	uint32_t freeMcellCnt;	/* Number of free mcells on this pg */
	uint32_t pageId:27;	/* Page number */
	uint32_t megaVersion:5;	/* Overall structure version */
	struct bsMC bsMCA[BSPG_CELLS];	/* Array of Bs Cells */
} bsMPgT;
/******************************************************************************
 * The next section contains structure definitions for                        *
 * the typed records in the BMT.                                              *
 *****************************************************************************/

#define BSR_XTNTS 1		/* BsXtntR structure type */

/*
 * The attributes record and the extent record share an mcell.
 * Be careful when adjusting the extent record's size.  It only
 * contains extents for the reserved metadata files that are hand
 * crafted at domain init time.
 */

/* An extent with vdBlk set to XTNT_TERM terminates the previous descriptor. */
#define XTNT_TERM ((uint32_t)-1)
/* An extent with vdBlk set to PERM_HOLE_START starts a permanent hole */
/* desriptor in a clone file. */
#define PERM_HOLE_START ((uint32_t)-2)

#define CREATE_PERM_HOLE(dmnp) \
    ((dmnp)->dmnVersion >= FIRST_PERM_HOLE_XTNTS_VERSION)

/*
 * The basic disk extent mapping structure is simply
 * an array of bitfile/logical block number pairs.
 * It is designed for efficient binary searching.
 */

typedef struct bsXtnt {
	uint32_t bsPage;		/* Bitfile page number */
	uint32_t vdBlk;		/* Logical (disk) block number */
} bsXtntT;

/*
 * Definition of delete link for global deferred deleted list.
 */
typedef struct delLink {
	bfMCIdT nextMCId;	/* next mcell in the list */
	bfMCIdT prevMCId;	/* preceding mcell in list */
} delLinkT;

/*
 * Structure which records the progress of a delete.
 */
typedef struct delRst {
	bfMCIdT mcid;		/* first unfinished mcell in del pend chain */
	vdIndexT vdIndex;	/* vd index of first unfinished mcell */
	uint32_t xtntIndex;	/* index of the first unfinished extent */
	uint32_t offset;		/* first unprocessed page in extent */
	uint32_t blocks;		/* quota blocks already freed */
} delRstT;

#define BMT_XTNTS 2		/* num. xtnt descriptors per bsXtntR */

typedef struct bfPrimXT {
	uint16_t mcellCnt;	/* total number of mcells with descriptors */
	uint16_t xCnt;		/* Count of elements used in bsXA */
	bsXtntT bsXA[BMT_XTNTS];/* Array of disk extent descriptors */
} bfPrimXT;

/*
 * The extent header, which mostly has stuff other than extents
 * (except for reserved files).
 */

typedef struct bsXtntR {
	bsXtntMapTypeT type;	/* The type of extent map */
	uint32_t chainVdIndex;	/* link to next extent record in the chain */
	bfMCIdT chainMCId;	/* link to next extent record in the chain */
	uint32_t rsvd1;		/* pass stripe info to xfer_xtnts_to_clone */
	bfMCIdT rsvd2;
	uint32_t blksPerPage;	/* Number of blocks per page */
	uint32_t segmentSize;	/* minimum number of pages that are allocated */
	/* to a stripe segment. */
	delLinkT delLink;	/* doubly linked global deferred delete list */
	delRstT delRst;		/* large bitfile deletion management */
	bfPrimXT firstXtnt;	/* first extent for non-striped files */
}       bsXtntRT;
#define BSR_ATTR 2		/* bitfile attributes record */

typedef enum {
	BSRA_INVALID, BSRA_CREATING, BSRA_DELETING, BSRA_VALID
}    bfStatesT;
/*
 * Bitfile client attributes.  These attributes are freely changeable
 * by clients (users and file system layer).  This struct is also used
 * for the inheritable attributes which the file system layer associates
 * with directories.
 */

typedef struct bsBfClAttr {
	bfDataSafetyT dataSafety;	/* Bitfile data safety requirement */
	serviceClassT reqServices;	/* required service class */
	serviceClassT optServices;	/* optional service class */
	int32_t extendSize;	/* add'l extend size in blocks */
	int32_t clientArea[BS_CLIENT_AREA_SZ];	/* client-specific area */
	int32_t rsvd1;
	int32_t rsvd2;
	bfTagT acl;		/* rsvd for ACL */
	int32_t rsvd_sec1;	/* rsvd for security stuff */
	int32_t rsvd_sec2;	/* rsvd for security stuff */
	int32_t rsvd_sec3;	/* rsvd for security stuff */
}          bsBfClAttrT;
/*
 * Bitfile attributes
 */

typedef struct bsBfAttr {
	bfStatesT state;	/* bitfile state of existence */
	uint32_t bfPgSz;		/* Bitfile area page size */
	ftxIdT transitionId;	/* ftxId when ds state is ambiguous */
	uint32_t cloneId;	/* 0 ==> orig; "> 0" ==> clone */
	uint32_t cloneCnt;	/* set's clone cnt last time bf changed */
	uint32_t maxClonePgs;	/* max pages in clone */
	int16_t deleteWithClone;	/* delete bitfile when its clone is deleted */
	int16_t outOfSyncClone;	/* clone may not be accessed */
	bsBfClAttrT cl;		/* client attributes */
}        bsBfAttrT;
#define BSR_VD_ATTR 3		/* Vd attributes */

typedef struct bsVdAttr {
	bsIdT vdMntId;		/* vd mount id */
	bsVdStatesT state;	/* vd state */
	vdIndexT vdIndex;	/* this vd's index in the domain */
	uint16_t jays_new_field;
	uint32_t vdBlkCnt;	/* block count of this vd */
	uint32_t stgCluster;	/* num blks each stg bitmap bit */
	uint32_t maxPgSz;	/* largest possible page size on vd */
	uint32_t bmtXtntPgs;	/* number of pages per BMT extent */
	serviceClassT serviceClass;	/* service class provided by vd */
}        bsVdAttrT;
#define BSR_DMN_ATTR 4		/* permanent (not mutable) domain attr */

typedef struct bsDmnAttr {
	bsIdT bfDomainId;	/* id of this bitfile domain */
	uint32_t maxVds;		/* maximum vd index */
	bfTagT bfSetDirTag;	/* tag of root bitfile-set tag directory */
}         bsDmnAttrT;

extern bsDmnAttrT bsNilDmnAttr;

#define BSR_XTRA_XTNTS 5	/* BsXtraXtntR structure type */

/* num. xtnt descriptors per bsXtraXtntR */
/* As of version 4.0B, BMT_XTRA_XTNTS equals 32. */

#define BMT_XTRA_XTNTS \
    ((BSC_R_SZ - 2*sizeof( bsMRT ) - (2 * sizeof( uint16_t ))) / sizeof( bsXtntT ))

typedef struct bsXtraXtntR {
	uint16_t blksPerPage;	/* Number of blocks per page */
	uint16_t xCnt;		/* Count of elements used in bsXA */
	bsXtntT bsXA[BMT_XTRA_XTNTS];	/* Array of disk extent descriptors */
}           bsXtraXtntRT;
#define BSR_SHADOW_XTNTS 6	/* bsShadowXtntT structure type */

/* num. xtnt descriptors per bsShadowXtnt */
/* As of version 4.0B, BMT_SHADOW_XTNTS equals 31. */

#define BMT_SHADOW_XTNTS \
    ((BSC_R_SZ - 2*sizeof( bsMRT ) - sizeof (vdIndexT) - (3 * sizeof( uint16_t ))) \
     / sizeof( bsXtntT ))

typedef struct bsShadowXtnt {
	vdIndexT allocVdIndex;	/* Disk on which to allocate storage */
	uint16_t mcellCnt;	/* total number of mcells with descriptors */
	uint16_t blksPerPage;	/* Number of blocks per page */
	uint16_t xCnt;		/* Count of elements used in bsXA */
	bsXtntT bsXA[BMT_SHADOW_XTNTS];	/* Array of disk extent descriptors */
}            bsShadowXtntT;
#define BSR_MCELL_FREE_LIST 7

typedef struct {
	uint32_t headPg;		/* first page of mcell free list */
}      bsMcellFreeListT;
#define BSR_BFS_ATTR 8
#define BFS_FRAG_MAX 8


typedef struct {
	bfSetIdT bfSetId;	/* bitfile-set's ID */
	bfTagT fragBfTag;	/* tag of frag bitfile */
	bfTagT nextCloneSetTag;	/* tag of next set in clone list */
	bfTagT origSetTag;	/* for clones, this is parent set */
	bfTagT nxtDelPendingBfSet;	/* next delete pending bf set */
	uint16_t state;		/* state of bitfile set */
	uint16_t flags;
	uint32_t cloneId;	/* 0 ==> orig; "> 0" ==> clone */
	uint32_t cloneCnt;	/* times orig has been cloned */
	uint32_t numClones;	/* current number of clones */
	uint32_t fsDev;		/* Unique ID */
	uint32_t freeFragGrps;
	uint32_t oldQuotaStatus;	/* not properly initialized */
	uid_t uid;		/* set's owner */
	gid_t gid;		/* set's group */
	mode_t mode;		/* set's permissions mode */
	char setName[BS_SET_NAME_SZ];	/* bitfile set's name */
	uint32_t fsContext[BS_FS_CONTEXT_SZ];	/* client fs's data */
	fragGrpT fragGrps[BFS_FRAG_MAX];	/* array of frag group list
						 * heads */
}      bsBfSetAttrT;

extern bsBfSetAttrT NilBfSetAttr;

/*
 * The migrate mcell list contains a list of mcells used during
 * bitfile migration.  Each mcell describes transition storage
 * used by migrate.
 *
 * A group of related mcells are linked together via the "nextMCId"
 * field in each mcell's header (bsMCT).  The groups are linked together
 * via the "nextMCId" field (bsMigStgT) in the first mcell of each group.
 * The groups are linked to bsMigMcellListT via the "firstMCId" field.
 */

#define BSR_VD_IO_PARAMS 9

typedef struct vdIoParams {
	int rdMaxIo;		/* max blocks that can be read/written  */
	int wrMaxIo;		/* in a consolidated I/O */
	int qtoDev;		/* max number of I/O's to be queued to dev */
	int consolidate;	/* whether should consolidate for disk */
	int blockingFact;	/* rm from blockingQ before rm from lazyQ */
	int lazyThresh;		/* min bufs on lazy q before driver called */
}          vdIoParamsT;
#define BSR_FREE_2 10

#define BSR_RSVD11 11		/* Used by HSM in DU versions prior to 4.0 */
#define BSR_RSVD12 12		/* Used by HSM in DU versions prior to 4.0 */
#define BSR_RSVD13 13		/* Used by HSM in DU versions prior to 4.0 */

typedef struct {
	int32_t r1;
	int32_t r2;
	int32_t r3;
	int32_t r4;
	int32_t r5;
	int32_t r6;
	uint32_t r7;
}      bsrRsvd11T;
#define BSR_DEF_DEL_MCELL_LIST 14
typedef delLinkT delLinkRT;

#define BSR_DMN_MATTR 15

/*
 * Only one disk contains the most current domain mutable attributes
 * record.  The record with the highest sequence number is
 * the current record.  This record is "moved" only in lgr_switch_vol().
 */

typedef struct bsDmnMAttr {
	uint32_t seqNum;		/* sequence number of this record */
	bfTagT delPendingBfSet;	/* head of 'bf set delete pending' list */
	uid_t uid;		/* domain's owner */
	gid_t gid;		/* domain's group */
	mode_t mode;		/* domain's access permissions */
	uint16_t vdCnt;		/* number of disks in the domain */
	uint16_t recoveryFailed;	/* Recovery failed, don't activate */
	bfTagT bfSetDirTag;	/* tag of root bitfile-set tag directory */
	bfTagT ftxLogTag;	/* tag of domain log */
	uint32_t ftxLogPgs;	/* number of pages in log */
}          bsDmnMAttrT;
/*
 * Bitfile inheritable attributes.  see bs_inherit().
 */

#define BSR_BF_INHERIT_ATTR 16

typedef bsBfClAttrT bsBfInheritAttrT;

#define BSR_RSVD17 17
typedef struct {
	int32_t r1;
	int32_t r2;
	int32_t r3;
	int32_t r4;
	int32_t r5;
	int32_t r6;
	uint32_t r7;
	uint32_t r8;
}      bsrRsvd17T;
extern bsrRsvd17T DefbsrRsvd17;

#define BSR_BFS_QUOTA_ATTR 18

/*
 * bsQuotaAttrT - Per-fileset quota attributes
 */

typedef struct bsQuotaAttr {
	/* the following are really 64-bit fields (hence the 'hi', 'lo' to
	 * designate the high 32 bits and the low 32 bits */
	uint32_t blkHLimitLo;	/* hard block limit */
	uint32_t blkHLimitHi;	/* hard block limit */
	uint32_t blkSLimitLo;	/* soft block limit */
	uint32_t blkSLimitHi;	/* soft block limit */
	uint32_t fileHLimitLo;	/* hard file limit */
	uint32_t fileHLimitHi;	/* zeros - for future use */
	uint32_t fileSLimitLo;	/* soft file limit */
	uint32_t fileSLimitHi;	/* zeros - for future use */
	/* end of 64-bit fields */

	time_t blkTLimit;	/* time limit for excessive disk blk use */
	time_t fileTLimit;	/* time limit for excessive file use */
	uint32_t quotaStatus;	/* quota system status */

	uint32_t unused1;	/* zeros */
	uint32_t unused2;	/* zeros */
	uint32_t unused3;	/* zeros */
	uint32_t unused4;	/* zeros */
}           bsQuotaAttrT;

typedef struct bsPropListHead {
	uint64_t flags;
	uint32_t pl_num;		/* Which pl in this tag */
	uint32_t spare;		/* currently unused field */
	uint32_t namelen;
	uint32_t valuelen;
	char buffer[1];		/* var len field */
}              bsPropListHeadT;

typedef struct bsPropListHead_v3 {
	uint64_t flags;
	uint32_t namelen;
	uint32_t valuelen;
	char buffer[1];		/* var len field */
}                 bsPropListHeadT_v3;

/*
 * PROPERTY LIST PAGE RECORD
 */

/*
 * Page size = total mcell space on page minus the size of one
 *             mcell header and two record headers, one for the
 *             record and one for a trailing nil record
 */
#define BSR_PROPLIST_PAGE_SIZE ((BSPG_CELLS*sizeof(bsMCT)) - \
                                sizeof(bfMCIdT) - \
                                (2*sizeof(bfTagT)) - \
                                (2*sizeof(uint16_t)) - \
                                (2*sizeof(bsMRT)))

/*
 * Max "large" data size found by experementation.  It is the maximum
 *             size of name and value stored in regular mcell chains
 */

/*
   BSR_PL_MAX_LARGE is the maximum size of a property list
   entry (aligned name + aligned value).  This is reinforced
   by MsfsPlMaxLen.  These fit into regular mcell chains.
   They use up to FTX_MX_PINR = 7 records per page.

   Page functionality has not been used since 4.0.

   If considering increasing BSR_PL_MAX_LARGE in the future,
   it is currently documented in 4.0 Release Notes.  Also,
   keep in mind FTX_MX_PINR.
*/
#define BSR_PL_MAX_LARGE (256+1292)

typedef struct bsPropListPage {
	uint32_t pl_num;		/* Which pl in this tag */
	uint32_t pl_seg;		/* Which segment for this pl_num */
	char buffer[BSR_PROPLIST_PAGE_SIZE];
}              bsPropListPageT;

typedef struct bsPropListPage_v3 {
	char buffer[BSR_PROPLIST_PAGE_SIZE];
}                 bsPropListPageT_v3;
#define BSR_DMN_TRANS_ATTR 21

typedef enum {
	BSR_VD_NO_OP,
	BSR_VD_ADDVOL,
	BSR_VD_RMVOL
}    bsVdOpT;

typedef struct bsDmnTAttr {
	uint32_t chainVdIndex;
	bfMCIdT chainMCId;
	bsVdOpT op;
	dev_t dev;
}          bsDmnTAttrT;
/*
 * vfast on-disk record
 *
 * WARNING: If changes are made to the following vfast
 *          enum or structure, make the same changes to the
 *          sbin file src/sbin/advfs/vods/print_mcells.c
 */

#define BSR_DMN_SS_ATTR 22

typedef enum {
	SS_DEACTIVATED,		/* vfast disabled on domain */
	SS_ACTIVATED,		/* vfast enabled on domain */
	SS_SUSPEND,		/* vfast has work suspended, lists still
				 * active */
	SS_ERROR,		/* vfast is disabled on domain due to unknown
				 * error */
	SS_CFS_RELOC,		/* CFS is in process of relocating domain to a
				 * new server node. vfast will have this
				 * state, if current state was SS_ACTIVATED,
				 * until last umount, when domain deactivation
				 * will reset state to SS_ACTIVATED. Special
				 * cluster reloc state that is not to be
				 * written to disk. */
	SS_CFS_MOUNT,
	SS_CFS_UMOUNT
	/* CFS is in process of (u)mounting. vfast will be temporarily set to
	 * these states, IF current state is SS_ACTIVATED. Otherwise, vfast
	 * state is untouched during the cluster (u)mount. */
}    ssDmnOpT;

typedef struct bsSSDmnAttr {
	ssDmnOpT ssDmnState;	/* processing state that domain is in */
	/* UI visable configurable options */
	uint16_t ssDmnDefragment;
	uint16_t ssDmnSmartPlace;
	uint16_t ssDmnBalance;
	uint16_t ssDmnDirectIo;
	uint16_t ssMaxPercentOfIoWhenBusy;
	/* output only variables */
	uint32_t ssFilesDefraged;
	uint32_t ssPagesDefraged;
	uint32_t ssPagesBalanced;
	uint32_t ssFilesIOBal;
	uint32_t ssExtentsConsol;
	uint32_t ssPagesConsol;
	/* UI unvisable configurable options */
	uint16_t ssAccessThreshHits;
	uint16_t ssSteadyState;
	uint16_t ssMinutesInHotList;
	uint16_t ssReserved0;
	/* UI unvisable configurable options */
	uint32_t ssReserved1;
	uint32_t ssReserved2;
}           bsSSDmnAttrT;
#define BSR_DMN_FREEZE_ATTR 23

typedef struct bsDmnFreezeAttr {
	time_t freezeBegin;	/* Time of last dmn freeze */
	time_t freezeEnd;	/* Time of last dmn thaw   */
	uint32_t freezeCount;	/* Number of times domain has been frozen */

	uint32_t unused1;	/* zeros */
	uint32_t unused2;	/* zeros */
	uint32_t unused3;	/* zeros */
	uint32_t unused4;	/* zeros */

}               bsDmnFreezeAttrT;
/* #define BSR_MAX 23           DEFINED IN bs_public.h */
#define BSR_NIL 0		/* a nil or unused record */

/******************************************************************************
 * Storage Bitmap structures.                                                 *
 ******************************************************************************/
/* The storage bitmap is an array of bits describing storage available
 * on a given disk.  The bits are organized into pages.  Each page
 * has an LSN and a checksum (XOR?) field.
 *
 * Bit clear means that block is unassigned.
 *
 * Bitmap pages are contained in the BsBm bitfile.
 *
 * Pages are 8192 bytes yielding 65536 bits (minus overhead) on each page.
 */

#define SBM_LONGS_PG     ((ADVFS_PGSZ - 2 * sizeof(uint32_t)) / sizeof(uint32_t))
#define SBM_BITS_LONG    (sizeof( uint32_t ) * 8 /* bits per byte */)
#define SBM_BITS_PG      (SBM_LONGS_PG * SBM_BITS_LONG)

/*
* In order to extend the BMT the new BMT pages, SBM pages and
* transaction objects coresponding to that operation will be logged.
* If the bmtXtntPgs value is so large that logging this operation will
* overflow the log we have a problem ("Log Half Full Panic").
* We need to monitor this parameter and make the appropriate run time
* decision reagarding its value.  A reasonable relationship between
* the value of bmtXtntPgs and the size of the log has been determined
* as follows:
*
* The number of log pages required to log the SBM pages for a given
* bmtXtntPgs value is approximatley:
*
*                 bmtXtntPgs / SBM_ADVFS_PGS
*
* This means that when (bmtXtntPgs / SBM_ADVFS_PGS) is greater then 1/2 the
* number of log pages, we are going to panic with "Log Half Full".
* Given this relationship and the additional overhead of transaction objects
* and other events being logged,  we will take a conservative approach and limit
* this relationship to 25%.
* The following macros serve to implement this strategy:
*/
/* # of AdvFs pages represented by one SBM Page (minus overhead) */
/* clsz = vd->stgCluster */
#define SBM_ADVFS_PGS(clsz) (SBM_BITS_PG / (ADVFS_PGSZ_IN_BLKS / (clsz) ))

/* Calculate the default logPgs value from the target bmtXtntPgs and stgCluster*/
#define BMT_EXT_LOGSZ_DEFAULT(xnt,clsz) \
  ((((xnt+SBM_ADVFS_PGS(clsz)) / SBM_ADVFS_PGS(clsz) \
      * SBM_ADVFS_PGS(clsz)) * 4)/SBM_ADVFS_PGS(clsz))

/* Calculate the default bmtXtntPgs value from the target log size  (25% of total) */
#define BMT_EXT_REQUEST_DEFAULT(lpgs, clsz) (((lpgs) * SBM_ADVFS_PGS(clsz)) / 4)

/* TRUE if requested bmtXtntPgs will NOT violate the above relationship */
#define BMT_EXT_REQUEST_OK(xnt, lpgs, clsz) (xnt <= BMT_EXT_REQUEST_DEFAULT(lpgs, clsz))
/* END OF DUPLICATE CODE */


typedef struct bsStgBm {
	uint32_t lgSqNm;		/* log sequence number */
	uint32_t xor;		/* bit xor */
	uint32_t mapInt[SBM_LONGS_PG];	/* uint32_t array (can't do bit arrays) */
}       bsStgBmT;
/******************************************************************************
 * Tag Directory structures.                                                  *
 ******************************************************************************/

/*
 * Make sure your compiler packs this structure into 8 bytes.
 * sequence number is only twelve bits.  The 4-tuple
 *
 *     <domain id, bitfile set tag, bitfile tag, sequence number>
 *
 * must be unique for all time.  When the sequence number wraps the
 * slot containing the tagmap struct becomes permanently unavailable.
 */
typedef struct bsTMap {
	union {
		/*
	         * First tagmap struct on page zero only.
	         */
		struct {
			uint32_t freeList;	/* head of free list */
			uint32_t unInitPg;	/* first uninitialized page */
		}      tm_s1;

		/*
	         * Tagmap struct on free list.
	         */
		struct {
			uint16_t seqNo;	/* must overlay seqNo in tm_s3 */
			uint16_t unused;	/* padding to 4 byte boundary */
			uint32_t nextMap;	/* next free tagmap struct
						 * within page */
		}      tm_s2;

		/*
	         * In use tagmap struct.
	         */
		struct {
			uint16_t seqNo;	/* must overlay seqNo in tm_s2 */
			uint16_t vdIndex;	/* virtual disk index */
			bfMCIdT bfMCId;	/* bitfile mcell id */
		}      tm_s3;
	}     tm_u;
}      bsTMapT;
/* Defintions for convenient access to fields of the bsTMapT type. */
#define tmFreeListHead tm_u.tm_s1.freeList
#define tmUninitPg tm_u.tm_s1.unInitPg
#define tmNextMap tm_u.tm_s2.nextMap
#define tmBfMCId tm_u.tm_s3.bfMCId
#define tmSeqNo tm_u.tm_s3.seqNo
#define tmVdIndex tm_u.tm_s3.vdIndex

/*
 * Structure defined for the convenience of pin_record.
 */
typedef struct bsTDirPgHdr {
	uint32_t currPage;	/* page number of this page */
	uint32_t nextFreePage;	/* next page having free TMaps */
	uint16_t nextFreeMap;	/* index of next free TMap, 1 based */
	uint16_t numAllocTMaps;	/* count of allocated tmaps */
	uint16_t numDeadTMaps;	/* count of dead tmaps */
	uint16_t padding;
}           bsTDirPgHdrT;
#define BS_TD_TAGS_PG ((ADVFS_PGSZ - (sizeof(bsTDirPgHdrT))) / sizeof(bsTMapT))

typedef struct bsTDirPg {
	struct bsTDirPgHdr tpPgHdr;
	struct bsTMap tMapA[BS_TD_TAGS_PG];	/* array of TMap elements */
}        bsTDirPgT;
#define tpCurrPage tpPgHdr.currPage
#define tpNextFreePage tpPgHdr.nextFreePage
#define tpNextFreeMap tpPgHdr.nextFreeMap
#define tpNumDeadTMaps tpPgHdr.numDeadTMaps
#define tpNumAllocTMaps tpPgHdr.numAllocTMaps

/*****************************************************************************
 * Logger On-Disk Structures.
 *****************************************************************************/

#define MAX_LOGS    BS_MAX_DOMAINS	/* Number of logs supported */

/*
 * logPgHdrT -
 *
 * Header found the beginning of each log page.
 */
typedef struct logPgHdr {
	lsnT thisPageLSN;	/* LSN of first record in the page */
	bfdBfMetaT pgType;	/* used as a page identifier */
	bfDomainIdT dmnId;	/* used as a page identifier */
	uint16_t pgSafe;		/* TRUE if page is in safe format */
	uint16_t chkBit;
	int16_t curLastRec;	/* word offset into pg.data */
	int16_t prevLastRec;	/* word offset into pg.data */
	logRecAddrT firstLogRec;/* first log rec addr */
}        logPgHdrT;
/*
 * logPgTrlrT -
 *
 * Trailer found at the end of each log page.
 */
typedef struct logPgTrlr {
	lsnT lsnOverwriteVal[ADVFS_PGSZ_IN_BLKS - 1];	/* The values in each
							 * block */
	/* (except the first block) that were */
	/* overwritten by the page's LSN in the routine */
	/* lgr_make_pg_safe().  lgr_restore_pg() uses   */
	/* this array to put back the overwritten */
	/* values.  */
	bfdBfMetaT pgType;	/* used as a page identifier */
	bfDomainIdT dmnId;	/* used as a page identifier */
}         logPgTrlrT;
/*
 * Number of data words per log page
 */
#define DATA_WORDS_PG \
    howmany( (ADVFS_PGSZ - sizeof(logPgHdrT) - sizeof(logPgTrlrT)), \
             sizeof(uint32_t) )

/*
 * logPgT -
 *
 * A log page consists of a header followed by log records.  The
 * records are accessed via the 'data' array of longwords.  The last
 * field is a page trailer.
 */
typedef struct logPg {
	logPgHdrT hdr;		/* Log page header */
	uint32_t data[DATA_WORDS_PG];	/* Log records (must type cast) */
	logPgTrlrT trlr;	/* Log page trailer */
}     logPgT;


/*****************************************************************************
 * Function prototypes for on-disk structure utility routines.               *
 *****************************************************************************/

#ifdef _KERNEL
int
read_raw_page(
    struct vnode * vp,		/* in - device's vnode */
    lbnT vdblk,			/* in - starting disk block number */
    uint blksPerPg,		/* in - number of blocks per page */
    void *pg);			/* in - bitfile metadata table page ptr */

int
read_raw_bmt_page(
    struct vnode * vp,		/* device's vnode */
    lbnT vdblk,			/* virtual disk block number */
    struct bsMPg * dpg);	/* master bitfile page ptr */

int
write_raw_page(
    struct vnode * vp,		/* in - device's vnode */
    lbnT vdblk,			/* in - starting disk block number */
    uint blksPerPg,		/* in - number of blocks per page */
    void *pg);			/* in - bitfile metadata table page ptr */

int
write_raw_bmt_page(
    struct vnode * vp,		/* device's vnode */
    lbnT vdblk,			/* virtual disk block number */
    struct bsMPg * dpg);	/* master bitfile page ptr */
int
write_raw_sbm_page(
    struct vnode * vp,		/* device's vnode */
    lbnT vdblk,			/* vd block number to write */
    struct bsStgBm * sbmPg	/* ptr to sbm buffer */
);

#endif				/* _KERNEL */
#endif				/* BS_ODS */
