/****************************************************************************
 *                                                                          *
 *  (C) DIGITAL EQUIPMENT CORPORATION 1990                                  *
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
 * @(#)$RCSfile: ms_generic_locks.h,v $ $Revision: 1.1.37.2 $ (DEC) $Date: 2001/01/24 21:06:57 $
 */

#ifndef _GENERIC_LOCKS_
#define _GENERIC_LOCKS_

#ifdef _KERNEL
#include <sys/lock.h>
#include <sys/kernel.h>
#else				/* _KERNEL */
#include <stdio.h>
#include <strings.h>
#include <sys/types.h>
#endif				/* _KERNEL */

#include <sys/time.h>

/*
 * cvT
 *
 * Defines a condition variable.
 */

typedef short cvT;

/*
 * mutexT - This structure defines a file system mutex.  It contains an
 * actual (primitive/CMA/...).
 *
 * INITIALIZATION:
 *
 *    XXX: Fix it:
 *    You must call mutex_init2() before using a mutexT variable.
 */

typedef struct mutex {
#ifdef _KERNEL
	kmutex_t mutex;
#else				/* _KERNEL */
	pthread_mutex_t mutex;
#endif				/* _KERNEL */
}     mutexT;

extern mutexT *MutexList;

/*
 * lkTypeT
 *
 * Lock type.  Enumerates the lock types supported by the lock manager.
 *
 * NOTE: If you change this enum then also update lkTypeNames below.
 */

typedef enum {
	LKT_INVALID,
	LKT_STATE,		/* state lock */
	LKT_BUF,		/* buffer lock */
	LKT_FTX			/* ftx lock */
}    lkTypeT;

/*
 * lkUsageT
 *
 * These are used to associate a notion of the lock's usage with the
 * lock so that when the locks are dumped it is easy to identify the
 * locks.
 *
 * For each lock type there is a comment that indicates the locking order
 * with respect to root transaction starts.  In other words, the comment
 * tells you if the lock can be locked before or after a call to ftx_start()
 * to start a root transaction.

 * NOTE: If you change this enum then also update lkUsageNames below.
 */

typedef enum {			/* ##, ftx_start() locking order    */
	LKU_UNKNOWN,		/* 0 */
	LKU_LOG_DESC,		/* 1, N/A                          */
	LKU_LOG_READ_STREAM,	/* 2, N/A                          */
	LKU_BF_STATE,		/* 3, lock after root ftx_start()  */
	LKU_BF_XTNT_MAP,	/* 4 */
	LKU_BF_COW,		/* 5, lock after root ftx_start()  */
	/* lock before MCELL_LIST       */
	LKU_BF_MCELL_LIST,	/* 6, lock after root ftx_start()  */
	/* lock after BF_COW            */
	LKU_BF_FLUSH,		/* 7 */
	LKU_BUFFER,		/* 8 */
	LKU_BF_SET_TBL,		/* 9, lock after root ftx_start()  */
	LKU_BF_SET_TAG_DIR,	/* 10, lock after root ftx_start()  */
	LKU_VD_STG_MAP,		/* 11 */
	LKU_VD_MCELLS,		/* 12, lock after root ftx_start()  */
	LKU_VD_PAGE0_MCELLS,	/* 13 */
	LKU_VD_MIG_MCELLS,	/* 14 */
	LKU_VD_DEV_BUSY,	/* 15 */
	LKU_VD_ACTIVE,		/* 16 */
	LKU_VD_LAZY_BLOCK,	/* 17 */
	LKU_WIRED_FREE,		/* 18 */
	LKU_RAW_BUF_FREE,	/* 19 */
	LKU_INIT,		/* 20 */
	LKU_FS_BF_GET,		/* 21 */
	LKU_CLONE_DEL,		/* 22 */
	LKU_obsolete_23,	/* 23 */
	LKU_obsolete_24,	/* 24 */
	LKU_FS_CONTEXT_SEM,	/* 25 */
	LKU_DQ_LOCK,		/* 26 */
	LKU_FILE_SET_LK,	/* 27 */
	LKU_DOMAIN_TBL,		/* 28 */
	LKU_BF_SET_STATE,	/* 29, to be obsoleted */
	LKU_SERVICE_CLASS_TBL,	/* 30, lock after root ftx_start()  */
	LKU_BF_SHLV,		/* 31 */
	LKU_FS_FILE,		/* 32, lock before root ftx_start() */
	LKU_ZAP_MCELLS,		/* 33 */
	LKU_BF_MOVE_METADATA,	/* 34, lock after root ftx_start()  */
	LKU_BF_MIG_TRUNC,	/* 35, N/A */
	LKU_VD_DDL_ACTIVE,	/* 36, lock before root ftx_start() */
	LKU_VD_DDL_ACTIVE_WAIT,	/* 37, N/A */
	LKU_QUOTA_FILE_LK,	/* 38 */
	LKU_FRAG_BF,		/* 39, lock after root ftx_start()  */
	LKU_BF_SET_SHLV,	/* 40 */
	LKU_MSS_PQD,		/* 41 */
	LKU_MSS_PQD_LOWPRI,	/* 42 */
	LKU_num_usage_types
}    lkUsageT;

/*
 * lkHdrT
 *
 * Common struct header for all locks.
 */

typedef struct lkHdr {
	lkTypeT lkType;

	/*
         * The following are used only for/by ftx locks.
         */
	void *nxtFtxLk;
	mutexT *mutex;
	lkUsageT lkUsage;
}     lkHdrT;

typedef struct ftxLk {
	lkHdrT hdr;		/* header used by ftx lock routines only */
	krwlock_t lock;		/* shared access, exclusive writer.  */
	cvT cv;			/* condition variable */
} ftxLkT;
/*
 * unLkActionT
 *
 * Used to determine whether or not a thread that just unlocked a lock
 * should signal or broadcast to other threads that are waiting on
 * the lock which the thread just unlocked.
 */

typedef enum {
	UNLK_NEITHER,
	UNLK_SIGNAL,
	UNLK_BROADCAST
}    unLkActionT;

typedef enum {
	LKS_UNLOCKED,
	LKS_SHARE,
	LKS_EXCL
}    shareExclLkStateT;
/****************************************************************************
 * STATE LOCK SUPPORT
 *
 * State locks provide a general mechanism for changing states and
 * waiting on state changes.
 */

/*
 * lkStatesT
 *
 * Client-defined states for state locks.  We use an enumerated type
 * to make it easier to debug.
 */

typedef enum {

	LKW_NONE,

	/* bfAccessT client states */
	ACC_VALID,		/* represents an accessible bitfile */
	ACC_INVALID,		/* does not represent an accessible bitfile */
	ACC_INIT_TRANS,		/* access struct is being initialized */
	ACC_FTX_TRANS,		/* bf's state is subject to completion of cur
				 * ftx */
	ACC_RECYCLE,		/* access struct is being recycled */
	ACC_DEALLOC,		/* access struct is being deallocated */

	/* struct vd */
	ACTIVE_DISK,		/* indicates whether disk active */
	INACTIVE_DISK,
	BLOCKED_Q,		/* whether thread must block on queue insert */
	UNBLOCKED_Q,

	/* struct bsBuf */
	BUF_DIRTY,
	BUF_BUSY,
	BUF_UNPIN_BLOCK,
	BUF_PIN_BLOCK,

	/* block for a free buffer header */
	BUF_AVAIL,
	NO_BUF_AVAIL,

	/* bfSetT cloneDelState states */
	CLONE_DEL_NORMAL,	/* No delete or xfer of storage from orig file
				 * now */
	CLONE_DEL_XFER_STG,	/* Xfer of storage from file in orig fileset
				 * now */
	CLONE_DEL_PENDING,	/* Deletion of clone fileset is pending */
	CLONE_DEL_DELETING	/* Deletion of clone fileset is occurring */

}    lkStatesT;
/*
 * stateLkT
 */

typedef struct stateLk {
	lkHdrT hdr;		/* header used by ftx lock routines only */
	lkStatesT state;	/* current state */
	lkStatesT pendingState;	/* pending state - ftx lock routines only */
	u_short waiters;	/* num threads waiting on the cvp */
	cvT cv;			/* condition variable */
}       stateLkT;
/*
 * buffer lock
 *
 * This lock is not currently mananged by the lock manager.  lk_init(),
 * lk_destroy() and bs_dump_locks() are the only routines that support
 * buffer locks.  The buffer cache code currently does the locking/unlocking
 * directly.  bufLkT exists mainly so that buffer cache locks can be
 * viewed via bs_dump_locks().
 */

typedef struct bufLk {
	lkHdrT hdr;		/* header used by ftx lock routines only */
	unsigned state;		/* define'd flags in bs_buf.h */
	cvT bufCond;		/* condition variable */
	u_short waiting;	/* number of threads waiting on the lock */
}     bufLkT;


typedef struct advfsLockUsageStats {
	unsigned long lock;
	unsigned long wait;
	unsigned long reWait;
	unsigned long signal;
	unsigned long broadcast;
}                   advfsLockUsageStatsT;

typedef struct advfsLockStats {
	unsigned long mutexLock;
	unsigned long mutexUnlock;
	unsigned long wait;
	unsigned long signal;
	unsigned long broadcast;
	unsigned long genLock;
	unsigned long genWait;
	unsigned long genReWait;
	unsigned long genSignal;
	unsigned long genBroadcast;
	unsigned long stateLock;
	unsigned long stateWait;
	unsigned long stateReWait;
	unsigned long stateSignal;
	unsigned long stateBroadcast;
	unsigned long shrLock;
	unsigned long shrWait;
	unsigned long shrReWait;
	unsigned long shrSignal;
	unsigned long excLock;
	unsigned long excWaitExc;
	unsigned long excWaitShr;
	unsigned long excReWaitExc;
	unsigned long excReWaitShr;
	unsigned long excSignal;
	unsigned long excBroadcast;
	unsigned long bufWait;
	unsigned long bufReWait;
	unsigned long bufSignal;
	unsigned long bufBroadcast;
	unsigned long pinBlockWait;
	unsigned long pinBlockReWait;
	unsigned long pinBlockSignal;
	unsigned long pinBlockBroadcast;
	unsigned long bfFlushWait;
	unsigned long bfFlushReWait;
	unsigned long bfFlushSignal;
	unsigned long bfFlushBroadcast;
	unsigned long ftxFreeWait;
	unsigned long ftxFreeReWait;
	unsigned long ftxFreeSignal;
	unsigned long ftxFreeBroadCast;
	unsigned long msgQWait;
	unsigned long msgQReWait;
	unsigned long msgQSignal;
	unsigned long msgQBroadcast;
	advfsLockUsageStatsT usageStats[LKU_num_usage_types];

	/* These are at the end to avoid breaking msfs_syscalls.h in user
	 * space */
	unsigned long ftxSlotWait;
	unsigned long ftxSlotFairWait;
	unsigned long ftxSlotReWait;
	unsigned long ftxSlotSignalFree;
	unsigned long ftxSlotSignalNext;
	unsigned long ftxExcWait;
	unsigned long ftxExcReWait;
	unsigned long ftxExcSignal;
	unsigned long ftxTrimWait;
	unsigned long ftxTrimReWait;
	unsigned long ftxTrimSignal;
	unsigned long ftxTrimBroadcast;
}              advfsLockStatsT;

extern advfsLockStatsT *AdvfsLockStats;

static inline u_short lk_waiters(const stateLkT *lk)
{
	return lk->waiters;
}

static inline lkStatesT lk_get_state(const stateLkT *lk)
{
	return lk->state;
}

/*
 ** Prototypes.
 */

void cond_wait(cvT * cvp, mutexT * mp);
void cond_broadcast(cvT * cvp);
void cond_signal(cvT * cvp);
void lk_signal(unLkActionT action, void *lk);
int lk_is_locked(void *lock);
void trace_hdr(void);
void bs_dump_locks(int locked);
void lk_init(void *lkp, mutexT * mp, lkTypeT type, int res, lkUsageT usage);
void lk_re_init(void *lkp, mutexT * mp, lkTypeT lkType, int res, lkUsageT usage);
void lk_destroy(void *lk);
void advfs_cv_init(cvT * cvp);

void
lk_wait_while(
    stateLkT * lk,
    mutexT * lk_mutex,
    lkStatesT waitState
);

void
lk_wait_for(
    stateLkT * lk,
    mutexT * lk_mutex,
    lkStatesT waitState
);

void
lk_wait_for2(
    stateLkT * lk,
    mutexT * lk_mutex,
    lkStatesT waitState1,
    lkStatesT waitState2
);

unLkActionT
lk_set_state(
    stateLkT * lk,
    lkStatesT newState
);

#endif				/* _GENERIC_LOCKS_ */
