/****************************************************************************
 *                                                                          *
 *  (C) DIGITAL EQUIPMENT CORPORATION 1991, 1992, 1993                      *
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
* @(#)$RCSfile: bs_bitfile_sets.h,v $ $Revision: 1.1.75.7 $ (DEC) $Date: 2006/02/27 10:08:20 $
 */

#ifndef _BS_BITFILE_SETS_
#define _BS_BITFILE_SETS_

#include <sys/types.h>
#include <sys/stdint.h>

#include "dyn_hash.h"

#include <fs/msfs/bs_public.h>
#include <fs/msfs/bs_domain.h>
#include <fs/msfs/ms_generic_locks.h>


struct domain;
struct bfAccess;
struct fileSetNode;

/****************************************************************************
 * frag mgt
 ****************************************************************************/

/* number of pages per group */
#define BF_FRAG_GRP_PGS 16

/* bytes per frag slot */
#define BF_FRAG_SLOT_BYTES 1024

/* bytes per pg */
#define BF_FRAG_PG_BYTES 8192

/* slots per pg */
#define BF_FRAG_PG_SLOTS (BF_FRAG_PG_BYTES / BF_FRAG_SLOT_BYTES)

/* slots per group */
#define BF_FRAG_GRP_SLOTS \
    (BF_FRAG_GRP_PGS * BF_FRAG_PG_BYTES / BF_FRAG_SLOT_BYTES)

/* max slots (2^31-1 to allow 0xffffffff as the end of slot number) */
#define BF_FRAG_MAX_SLOTS 0xfffffffe

/* max groups */
#define BF_FRAG_MAX_GRPS (BF_FRAG_MAX_SLOTS / BF_FRAG_GRP_SLOTS)

/* max pages per frag bf */
#define BF_FRAG_MAX_PGS (BF_FRAG_MAX_GRPS * BF_FRAG_GRP_PGS)

#define FRAG2PG( frag ) ((frag) / BF_FRAG_PG_SLOTS)
#define FRAG2SLOT( frag ) ((frag) % BF_FRAG_PG_SLOTS)
#define FRAG2GRP( frag ) (( frag ) / BF_FRAG_GRP_SLOTS * BF_FRAG_GRP_PGS)

#define FRAGS_PER_GRP( fragType ) ((fragType == BF_FRAG_ANY) ? \
            (BF_FRAG_GRP_SLOTS - 1) : (BF_FRAG_GRP_SLOTS - 1) / (fragType))

/*
 * end-of-group-list.
 */
#define BF_FRAG_EOG 0xffffffff

/* end-of-frag-list */
#define BF_FRAG_EOF 0xffffffff

/* Group has internal free list corruption */
#define BF_FRAG_BAD_GROUP 0x00000001

#define BF_FRAG_NULL 0xffff
#define BF_FRAG_NEXT_FREE 0

#define BF_FRAG_FREE_GRPS 0
#define BF_FRAG_VERSION 1

#define BF_FRAG_FREE_GRPS_MIN 16/* 2MB */
#define BF_FRAG_FREE_GRPS_MAX 48/* 6MB */

/* per group header */

typedef struct grpHdr {
	uint32_t nextFreeFrag;	/* frag index (valid only when "version == 0") */
	uint32_t lastFreeFrag;	/* frag index (valid only when "version == 0") */
	/* If version is 1, value will be either */
	/* BF_FRAG_EOF (normally) or else */
	/* BF_FRAG_BAD_GROUP if internal free list */
	/* corruption has been detected */
	uint32_t nextFreeGrp;	/* page number */
	uint32_t self;		/* this group's starting page number */
	bfFragT fragType;	/* type of frags in this group */
	int freeFrags;		/* number of free frags in the group */
	bfSetIdT setId;		/* bitfile-set's ID */

	/* the following fields were added in ADVFS v3.0 they were all zeros
	 * in pre-ADVFS v3.0 */

	unsigned int version;	/* metadata version pre-ADVFS v1.0 == 0, ADVFS
				 * v3.0 == 1 */
	uint32_t firstFrag;	/* frag index */

	/*
         * the following is used a map of the free frags in the group.
         * it a linked list where element zero (0) is used as the head
         * of the list (since frag 0 is always the group header it can
         * never be allocated so element zero would otherwise be unused)
         */
	unsigned short freeList[BF_FRAG_GRP_SLOTS];
}      grpHdrT;

typedef struct fragHdr {
	uint32_t nextFreeFrag;
	uint32_t self;		/* this frag's starting slot number */
	bfFragT fragType;	/* type of frags in this group */
}       fragHdrT;


typedef struct fragSlotT {
	char bytes[BF_FRAG_SLOT_BYTES];
}         fragSlotT;

typedef struct slotPg {
	fragSlotT slots[BF_FRAG_PG_SLOTS];
}      slotsPgT;
/****************************************************************************
 * bf set mgt
 ****************************************************************************/

typedef enum {
	/* bitfile set states in bfSetT */
	BFS_INVALID,
	BFS_READY,		/* not special happening with the set */
	BFS_CLONING,		/* set is being cloned / set is chaning to a
				 * clone */
	BFS_BUSY,		/* prevent ss_open_file during mount or
				 * unmount */
	BFS_DELETING,		/* set is being deleted */
	BFS_DELETING_CLONE,	/* set's clone is being delete */
	BFS_UNMOUNTED		/* set is unmounted */
}    bfsStateT;
/*
 * bfSetT - Bitfile-set descriptor (in-memory)
 */

#ifdef _KERNEL
typedef struct bfSet {
	dyn_hashlinks_w_keyT hashlinks;	/* dyn_hashtable links */
	char bfSetName[BS_SET_NAME_SZ];	/* bitfile-set's name */
	bfSetIdT bfSetId;	/* bitfile-set's ID */
	u_int bfSetMagic;	/* magic number: structure validation */
	int fsRefCnt;		/* current number of bfs_access() accessors */
	struct domain *dmnP;		/* pointer to BF-set's domain structure */
	bfsQueueT bfSetList;	/* list of bfSetT's associated with this
				 * domain */
	mutexT accessChainLock;	/* protects the next two fields */
	struct bfAccess *accessFwd;	/* list of access structures for this fileset */
	struct bfAccess *accessBwd;
	dev_t dev;		/* set's dev_t; used for statfs() and stat() */

	bfTagT dirTag;		/* tag of bitfile-set's tag directory */
	struct bfAccess *dirBfAp;

	bfSetT *cloneSetp;	/* pointer to clone set */
	bfSetT *origSetp;	/* for clones, this is parent set desc ptr */
	uint32_t cloneId;	/* 0 ==> orig; "> 0" ==> clone */
	uint32_t cloneCnt;	/* times orig has been cloned */
	uint32_t numClones;	/* current number of clones */

	/*
         * The following state lock is used to coordinate the deletion of
         * a clone fileset and the transfer of extents from files in the original
         * fileset to files in the clone fileset.
         */
	mutexT cloneDelStateMutex;	/* Protects cloneDelState and
					 * xferThreads */
	stateLkT cloneDelState;	/* State of clone fileset deletion        */
	int xferThreads;	/* Number of threads doing transfer of    */
	/* storage from an original file to a     */
	/* clone file in this fileset.            */

	uint32_t infoLoaded;	/* true if correct tagdir info has been loaded */

	mutexT setMutex;	/* protects dirLock & fragLock lock header
				 * fields */

	ftxLkT dirLock;		/* tag dir lock */

	bfsStateT state;	/* state */

	int bfsDelPend;		/* true when a delete is started */
	int bfsOpPend;		/* true when set is about to be cloned or
				 * mounted */
	int bfsOpWait;		/* operation is waiting for holders to get
				 * done */
	int bfsHoldCnt;		/* count of threads holding the clone fileset */
	int bfsHoldWait;	/* count of hold_cloneset waiters */

	/* tagdir info - valid iff infoLoaded == TRUE */
	int bfCnt;		/* number of bitfiles in the bitfile set */
	unsigned long tagFrLst;	/* page no of head of free list + 1 */
	unsigned long tagUnInPg;/* first uninitialized page in tag dir */
	unsigned long tagUnMpPg;/* first unmapped page in tag dir */

	/* frag stuff */

	ftxLkT fragLock;	/* frag bf lock */

	bfTagT fragBfTag;
	struct bfAccess *fragBfAp;
	uint32_t freeFragGrps;

	uint32_t truncating;	/* true if truncating fragbf */

	fragGrpT fragGrps[BF_FRAG_MAX];	/* array of frag group list heads */
	struct fileSetNode *fsnp;	/* file set node pointer */

	mutexT bfSetMutex;	/* protect bfSetFlags */
	uint32_t bfSetFlags;	/* The high-order 16-bits of this field holds */
	/* in-memory attributes and the low-order */
	/* 16-bits holds on-disk flags */
}     bfSetT;
#endif				/* _KERNEL */

extern bfSetT nilBfSet;

/* Macro to set the filesystem id into an fsid_t */

#define BS_GET_FSID(_setp,_fsid) \
{ \
    _fsid.__fsid_val[0] = _setp->dev; \
    _fsid.__fsid_val[1] = MOUNT_MSFS; \
}


/*
 * bfSetFlags definitions
 *   The low 16 bits of bfSetFlags are defined in bs_ods.h (BFS_OD_...)
 *   The upper 16 bits of bfSetFlags are defined here (BFS_IM_...)
 */
#define BFS_IM_ON_DISK_MASK    0x0000FFFF	/* Used to select the on-disk
						 * flags */
#define BFS_IM_DIRECTIO        0x00010000	/* Default direct I/O */

#ifdef _KERNEL
/*
 * MACROs for working with the BfSetHashTbl
 */
#define BFSET_HASH_INITIAL_SIZE 32
#define BFSET_HASH_CHAIN_LENGTH 16
#define BFSET_HASH_ELEMENTS_TO_BUCKETS 4
#define BFSET_HASH_USECS_BETWEEN_SPLITS 1000000	/* 1 second */

#define BFSET_GET_HASH_KEY( _bfSetId ) \
    ( (_bfSetId).domainId.tv_sec + (_bfSetId).dirTag.num )

#define BFSET_HASH_LOCK( _key, _cnt ) \
    ( (bfSetT *)dyn_hash_obtain_chain( BfSetHashTbl, _key, _cnt ) )

#define BFSET_HASH_UNLOCK( _key ) \
    (void)dyn_hash_release_chain( BfSetHashTbl, _key )

#define BFSET_HASH_REMOVE( _bfSetp, _lock_action ) \
    (void)dyn_hash_remove( BfSetHashTbl, _bfSetp, _lock_action )

#define BFSET_HASH_INSERT( _bfSetp, _lock_action ) \
    (void)dyn_hash_insert( BfSetHashTbl, _bfSetp, _lock_action )


#define BS_MAX_CLONES 1

/*
 ** Clone id of zero represents original bitfile or bitfile set.
 */

#define BS_BFSET_ORIG 0

#define MSFS_INITIAL_FRAG_MSGQ_ENTRIES 16

/*
 * ADD_ACC_SETLIST places an access structure onto the head
 * of a fileset structure's list of access structures.  The
 * fileset's accessChainLock is held while that chain is manipulated.
 * This macros returns with the BFALOCK HELD.
 */
static inline void ADD_ACC_SETLIST(struct bfAccess *bfap)
{
    bfSetT *bfSetp = bfap->bfSetp;
    KASSERT(BFSET_VALID(bfSetp));
    mutex_enter(&bfSetp->accessChainLock.mutex);
    KASSERT(bfap->setFwd == NULL);
    KASSERT(bfap->setBwd == NULL);
    mutex_enter(&bfap->bfaLock.mutex);
    bfap->setFwd = bfSetp->accessFwd;
    bfap->setBwd = (struct bfAccess *)(&bfSetp->accessFwd);
    if (bfSetp->accessBwd == (struct bfAccess *)(&bfSetp->accessFwd))
        bfSetp->accessBwd = bfap;
    else
        bfSetp->accessFwd->setBwd = bfap;
    bfSetp->accessFwd = bfap;
    mutex_exit(&bfSetp->accessChainLock.mutex);
}

/*
 * RM_ACC_SETLIST removes an access structure from a fileset
 * structure's list of access structures.  The fileset's
 * accessChainLock is held while that chain is manipulated.
 * It will be locked and unlocked here if lock_list is TRUE.
 * Otherwise, it is assumed that the caller has locked the
 * chain and will unlock it.
 */
static inline void RM_ACC_SETLIST(struct bfAccess *bfap, int lock_list)
{
    bfSetT *bfSetp = bfap->bfSetp;
    KASSERT(BFSET_VALID(bfSetp));
    if (lock_list)
        mutex_enter(&bfSetp->accessChainLock.mutex);
    else
        KASSERT(mutex_owned(&bfSetp->accessChainLock.mutex));
    KASSERT(bfap->setFwd != NULL);
    KASSERT(bfap->setBwd != NULL);
    if (bfSetp->accessFwd == bfap)
        bfSetp->accessFwd = bfap->setFwd;
    else
        bfap->setBwd->setFwd = bfap->setFwd;
    if (bfSetp->accessBwd == bfap)
        bfSetp->accessBwd = bfap->setBwd;
    else
        bfap->setFwd->setBwd = bfap->setBwd;
    bfap->setFwd = bfap->setBwd = NULL;
    if (lock_list)
        mutex_exit(&bfSetp->accessChainLock.mutex);
}

int
rbf_bfs_create(
    struct domain * dmnP,	/* in - domain handle */
    serviceClassT reqServ,	/* in - required service class */
    serviceClassT optServ,	/* in - optional service class */
    char *setName,		/* in - set's name */
    uint32_t fsetOptions,	/* in - fileset option flags */
    ftxHT parentFtxH,		/* in - parent transaction handle */
    bfSetIdT * bfSetId		/* out - bitfile set id */
);

int
rbf_bfs_access(
    bfSetT ** retBfSetp,	/* out - pointer to open bitfile-set */
    bfSetIdT bfSetId,		/* in - bitfile-set id */
    ftxHT ftxH			/* in - transaction handle */
);

/*
 * Options for rbf_bfs_open() and bs_bfs_close().
 */

#define BFS_OP_DEF        0
#define BFS_OP_IGNORE_DEL 1
#define BFS_OP_XFER_XTNTS_TO_CLONE 2

int
rbf_bfs_open(
    bfSetT ** retBfSetp,	/* out - pointer to open bitfile-set */
    bfSetIdT bfSetId,		/* in - bitfile-set id */
    uint32_t options,		/* in - options flags */
    ftxHT ftxH			/* in - transaction handle */
);

void
bs_bfs_close(
    bfSetT * bfSetp,		/* in - pointer to open bitfile-set */
    ftxHT ftxH,			/* in - transaction handle */
    uint32_t options		/* in - options flags */
);

int
bs_bfs_delete(
    bfSetIdT bfSetId,		/* in - bitfile set id */
    struct domain * dmnP,	/* in - set's domain pointer */
    long xid,			/* in - CFS transaction id */
    u_long flag			/* in - to indicate cluster_root failover */
);

int
bs_bfs_clone(
    bfSetIdT origSetId,		/* in - activated orig set's id */
    char *setName,		/* in - set's name */
    bfSetIdT * cloneSetId,	/* out - new clone's id */
    struct domain * dmnP,	/* in - set's domain pointer */
    long xid			/* in - CFS transaction id */
);

void
bs_bfs_init(
    void
);

int
bs_bfs_add_root(
    bfSetIdT rootBfSetId,	/* in - bitfile-set id */
    struct domain * dmnP,	/* in - BF-set's domain's pointer */
    bfSetT ** retRootBfSetp	/* out - pointer to BF-set's descriptor */
);

void
bs_bfs_switch_root_tagdir(
    struct domain * domain,		/* in */
    bfTagT newTag		/* in */
);

void
bfs_delete_pending_list_finish_all(
    struct domain * dmnp,
    u_long flag
);

bfSetT *
bs_bfs_lookup_desc(
    bfSetIdT bfSetId		/* in - bitfile set's ID */
);


/*
 * also see bs_public.h for more routines (public bitfile set routines).
 */

int
bs_fragbf_open(
    bfSetT * bfSetp
);

int
bs_fragbf_close(
    bfSetT * bfSetp
);

int
bs_frag_alloc(
    bfSetT * setp,		/* in */
    bfFragT fragType,		/* in */
    ftxHT ftxH,			/* in */
    bfFragIdT * fragId		/* out */
);

void
bs_frag_dealloc(
    bfSetT * setp,		/* in */
    ftxHT ftxH,			/* in */
    bfFragIdT fragId		/* in */
);

/* bs_cow options */
#define COW_NONE  0
#define COW_PINPG 1

void
bs_cow(
    struct bfAccess * bfap,		/* in - bitfile access struct */
    unsigned cowOpt,		/* in - COW options */
    unsigned long pg,		/* in - page being modified (if applicable) */
    unsigned long pgCnt,	/* in - # pages being modified */
    ftxHT parentFtxH		/* in - parent transaction handle */
);

void bs_bfs_out_of_sync(bfSetT *, ftxHT);

bfSetT *hold_cloneset(struct bfAccess *, int);

void release_cloneset(bfSetT *);

int get_clu_clone_locks(struct bfAccess *, struct fsContext *, bfSetT **, struct bfAccess **);

void release_clu_clone_locks(struct bfAccess *, bfSetT *, struct bfAccess *, int);

#endif				/* _KERNEL */
#endif				/* _BS_BITFILE_SETS_ */
