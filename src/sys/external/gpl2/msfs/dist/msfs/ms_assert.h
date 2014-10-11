/****************************************************************************
 *                                                                          *
 *  (C) DIGITAL EQUIPMENT CORPORATION 1992                                  *
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
 * @(#)$RCSfile: ms_assert.h,v $ $Revision: 1.1.44.9 $ (DEC) $Date: 2007/06/18 18:37:51 $
 */

#ifndef MS_ASSERT_H
#define MS_ASSERT_H

/*
 * Redesigned debug functionality from scratch and adapted for NetBSD.
 * There is no code-share with the original Tru64 work, but keep the original
 * copy-right notice for credit.
 *
 * To enable debugging define MSFS_DEBUG in compilation-time (eg -DMSFS_DEBUG).
 */
#ifdef MSFS_DEBUG

#include <sys/param.h>
#include <sys/types.h>
#include <sys/systm.h>
#include <sys/stdarg.h>

/*
 * Debug levels:
 *   0 - disabled
 *   1 - enable standard asserts (fatal)
 *   2 - enable debug asserts (non fatal)
 *  >2 - enable extensive tests
 *
 * Change debug levels in run-time with sysctl(8).
 *
 * The default debug level is possible to specify in compilation time,
 * e.g.: -DMSFSDEBUGLEVEL_DEFAULT=0
 *
 * If the default level is not explicitly specified then it defaults to 1.
 *
 * WARNING: Specifing MsfsDebugLevel to 2 in run-time enables additional
 *          code paths. It may be dangerous to hot-swap it.
 */
extern unsigned int MsfsDebugLevel;

/**
 * Assert an expression and print a debug message in case of a false statement.
 * This assert is fatal and will panic the running system.
 *
 * @param ex
 *   Evaluation of an expression
 * @param fmt
 *   Debug message to be displayed in case of triggering a false assert
 */
static inline void msfs_assert(int ex, char *fmt, ...)
{
    if (MsfsDebugLevel && !ex) {
        va_list ap;
        va_start(ap, fmt);
        vpanic(fmt, ap);
        va_end(ap);
    }
}

/**
 * Assert an expression and print a debug message in case of a false statement.
 * This assert is not fatal and will not panic the running system.
 *
 * @param ex
 *   Evaluation of an expression
 * @param fmt
 *   Debug message to be displayed in case of triggering a false assert
 */
static inline void msfs_debug_assert(int ex, char *fmt, ...)
{
    if (MsfsDebugLevel > 1 && !ex) {
        va_list ap;
        va_start(ap, fmt);
        vprintf(fmt, ap);
        va_end(ap);
    }
}

/**
 * Check the current debug level for demand of additional extensive tests.
 *
 * @return
 *   - 1 if the extensive tests are desired to be enabled
 *   - 0 if the extensive tests are not desired
 */
static inline int msfs_extensive_tests(void)
{
	return (MsfsDebugLevel > 2);
}

#else  /* MSFS_DEBUG */

#define msfs_assert()           do { } while(0)
#define msfs_debug_assert()     do { } while(0)
#define msfs_extensive_tests()  (0)

#endif /* MSFS_DEBUG */

#endif /* MS_ASSERT_H */
