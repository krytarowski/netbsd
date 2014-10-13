/****************************************************************************
 *                                                                          *
 *  (C) DIGITAL EQUIPMENT CORPORATION 1989, 1990, 1991                      *
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
 * @(#)$RCSfile: ms_privates.h,v $ $Revision: 1.1.81.2 $ (DEC) $Date: 2003/01/03 18:29:30 $
 */

#ifndef MS_PRIVATES
#define MS_PRIVATES

#include <sys/param.h>
#include <sys/types.h>
#include <sys/rwlock.h>

struct ftxLk;

void FRAG_LOCK_WRITE( struct ftxLk *sLk );
void FRAG_LOCK_READ( struct ftxLk *sLk );
void FRAG_UNLOCK( struct ftxLk *sLk );
void MCELL_LOCK_WRITE( struct ftxLk *sLk );
void MCELL_UNLOCK( struct ftxLk *sLk );
void STGMAP_LOCK_WRITE( struct ftxLk *sLk );
void STGMAP_UNLOCK( struct ftxLk *sLk );
void MCELLIST_LOCK_WRITE( struct ftxLk *sLk );
void MCELLIST_LOCK_READ( struct ftxLk *sLk );
void MCELLIST_UNLOCK( struct ftxLk *sLk );
void XTNMAP_LOCK_WRITE( struct ftxLk *sLk );
void XTNMAP_LOCK_READ( struct ftxLk *sLk );
void XTNMAP_UNLOCK( struct ftxLk *sLk );
void XTNMAP_LOCK_DOWNGRADE( struct ftxLk *sLk );
void COW_LOCK_WRITE( krwlock_t *sLk );
void COW_LOCK_READ( krwlock_t *sLk );
void COW_UNLOCK( krwlock_t *sLk );
void CLU_CLXTNT_READ_LOCK_RECURSIVE( krwlock_t *sLk );
void CLU_CLXTNT_UNLOCK_RECURSIVE( krwlock_t *sLk );
void CLU_CLXTNT_WRITE( krwlock_t *sLk );
void CLU_CLXTNT_READ( krwlock_t *sLk );
void CLU_CLXTNT_TRY_READ( krwlock_t *sLk );
void CLU_CLXTNT_UNLOCK( krwlock_t *sLk );
void RAWBUFREE_LOCK_WRITE( struct ftxLk *sLk );
void RAWBUFREE_UNLOCK( struct ftxLk *sLk );
void MIGTRUNC_LOCK_READ( krwlock_t *sLk );
void MIGTRUNC_LOCK_WRITE( krwlock_t *sLk );
void MIGTRUNC_UNLOCK( krwlock_t *sLk );
void DDLACTIVE_LOCK_READ( krwlock_t *sLk );
void DDLACTIVE_LOCK_WRITE( krwlock_t *sLk );
void DDLACTIVE_UNLOCK( krwlock_t *sLk );
void TRUNC_XFER_READ_LOCK_RECURSIVE( krwlock_t *sLk );
void TRUNC_XFER_UNLOCK_RECURSIVE( krwlock_t *sLk );
void TRUNC_XFER_LOCK_READ( krwlock_t *sLk );
void TRUNC_XFER_LOCK_WRITE( krwlock_t *sLk );
void TRUNC_XFER_UNLOCK( krwlock_t *sLk );

/*
 * Some private definitions
 */

#define MSFS_VN_PRIVATE \
    roundup( sizeof( struct bfNode ) + sizeof( struct fsContext ),  \
             sizeof( void * ) )

#define MAX_VIRT_VM_PAGE_RNG 256

#ifdef _KERNEL

#include "../msfs/ms_assert.h"
#define ASSERT_NO_LOCKS()     MS_SMP_ASSERT(current_thread()->lock_count == 0)

#endif

/*
 * smoothsync support
 */
#ifdef  _KERNEL
#ifdef ADVFS_SMP_ASSERT
/* only enable smoothsync_debug support if the rest of the AdvFS assert
 * package has been enabled.
 */
#define SMOOTHSYNC_DEBUG 1
#endif /* ADVFS_SMP_ASSERT */

#if SMOOTHSYNC_DEBUG
extern u_int smsync_debug;
#define SMSYNC_DBG_Q    0x1
#define SMSYNC_DBG_OP   0x2
#define SMSYNC_DBG_BUF  0x4
#define SMSYNC_DBG(flag,action) \
{ \
    if (smsync_debug & (flag)) (action); \
}
#else
#define SMSYNC_DBG(flag,action)
#endif /* SMOOTHSYNC_DEBUG */
#endif /* _KERNEL */

#include "../msfs/bs_ods.h"
#include "../msfs/bs_ims.h"
enum msfs_setproplist_enum { NO_SET_CTIME=0, SET_CTIME = 1 };

#endif /* MS_PRIVATES */
