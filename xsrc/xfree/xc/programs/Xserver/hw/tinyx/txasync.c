/*
 * Copyright � 1999 Keith Packard
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Keith Packard not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Keith Packard makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * KEITH PACKARD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL KEITH PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
/* $XFree86: xc/programs/Xserver/hw/tinyx/txasync.c,v 1.1 2004/06/02 22:43:00 dawes Exp $ */
/*
 * Copyright (c) 2004 by The XFree86 Project, Inc.
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 *
 *   1.  Redistributions of source code must retain the above copyright
 *       notice, this list of conditions, and the following disclaimer.
 *
 *   2.  Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer
 *       in the documentation and/or other materials provided with the
 *       distribution, and in the same place and form as other copyright,
 *       license and disclaimer information.
 *
 *   3.  The end-user documentation included with the redistribution,
 *       if any, must include the following acknowledgment: "This product
 *       includes software developed by The XFree86 Project, Inc
 *       (http://www.xfree86.org/) and its contributors", in the same
 *       place and form as other third-party acknowledgments.  Alternately,
 *       this acknowledgment may appear in the software itself, in the
 *       same form and location as other such third-party acknowledgments.
 *
 *   4.  Except as contained in this notice, the name of The XFree86
 *       Project, Inc shall not be used in advertising or otherwise to
 *       promote the sale, use or other dealings in this Software without
 *       prior written authorization from The XFree86 Project, Inc.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE XFREE86 PROJECT, INC OR ITS CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "tinyx.h"

/*
 * These functions wrap the low-level fb rendering functions and
 * synchronize framebuffer/accelerated drawing by stalling until
 * the accelerator is idle
 */

void
KdCheckFillSpans  (DrawablePtr pDrawable, GCPtr pGC, int nspans,
		   DDXPointPtr ppt, int *pwidth, int fSorted)
{
    KdCheckSync (pDrawable->pScreen);
    fbFillSpans (pDrawable, pGC, nspans, ppt, pwidth, fSorted);
}

void
KdCheckSetSpans (DrawablePtr pDrawable, GCPtr pGC, char *psrc,
		 DDXPointPtr ppt, int *pwidth, int nspans, int fSorted)
{
    KdCheckSync (pDrawable->pScreen);
    fbSetSpans (pDrawable, pGC, psrc, ppt, pwidth, nspans, fSorted);
}

void
KdCheckPutImage (DrawablePtr pDrawable, GCPtr pGC, int depth,
		 int x, int y, int w, int h, int leftPad, int format,
		 char *bits)
{
    KdCheckSync (pDrawable->pScreen);
    fbPutImage (pDrawable, pGC, depth, x, y, w, h, leftPad, format, bits);
}

RegionPtr
KdCheckCopyArea (DrawablePtr pSrc, DrawablePtr pDst, GCPtr pGC,
		 int srcx, int srcy, int w, int h, int dstx, int dsty)
{
    if (pSrc->type == DRAWABLE_WINDOW || pDst->type == DRAWABLE_WINDOW)
	KdCheckSync (pSrc->pScreen);
    return fbCopyArea (pSrc, pDst, pGC, srcx, srcy, w, h, dstx, dsty);
}

RegionPtr
KdCheckCopyPlane (DrawablePtr pSrc, DrawablePtr pDst, GCPtr pGC,
		  int srcx, int srcy, int w, int h, int dstx, int dsty,
		  unsigned long bitPlane)
{
    if (pSrc->type == DRAWABLE_WINDOW || pDst->type == DRAWABLE_WINDOW)
	KdCheckSync (pSrc->pScreen);
    return fbCopyPlane (pSrc, pDst, pGC, srcx, srcy, w, h, dstx, dsty,
			bitPlane);
}

void
KdCheckPolyPoint (DrawablePtr pDrawable, GCPtr pGC, int mode, int npt,
		  DDXPointPtr pptInit)
{
    KdCheckSync (pDrawable->pScreen);
    fbPolyPoint (pDrawable, pGC, mode, npt, pptInit);
}

void
KdCheckPolylines (DrawablePtr pDrawable, GCPtr pGC,
		  int mode, int npt, DDXPointPtr ppt)
{

    if (pGC->lineWidth == 0)
	KdCheckSync (pDrawable->pScreen);
    fbPolyLine (pDrawable, pGC, mode, npt, ppt);
}

void
KdCheckPolySegment (DrawablePtr pDrawable, GCPtr pGC, 
		    int nsegInit, xSegment *pSegInit)
{
    if (pGC->lineWidth == 0)
	KdCheckSync(pDrawable->pScreen);
    fbPolySegment (pDrawable, pGC, nsegInit, pSegInit);
}

void
KdCheckPolyRectangle (DrawablePtr pDrawable, GCPtr pGC, 
		      int nrects, xRectangle *prect)
{
    if (pGC->lineWidth == 0)
	KdCheckSync(pDrawable->pScreen);
    fbPolyRectangle (pDrawable, pGC, nrects, prect);
}

void
KdCheckPolyArc (DrawablePtr pDrawable, GCPtr pGC, 
		int narcs, xArc *pArcs)
{
    if (pGC->lineWidth == 0)
    {
	KdCheckSync(pDrawable->pScreen);
	fbPolyArc (pDrawable, pGC, narcs, pArcs);
    }
    else
	miPolyArc (pDrawable, pGC, narcs, pArcs);
}

#if 0
void
KdCheckFillPolygon (DrawablePtr pDrawable, GCPtr pGC, 
		    int shape, int mode, int count, DDXPointPtr pPts)
{
    KdCheckSync(pDrawable->pScreen);
    fbFillPolygon (pDrawable, pGC, mode, count, pPts);
}
#endif

void
KdCheckPolyFillRect (DrawablePtr pDrawable, GCPtr pGC,
		     int nrect, xRectangle *prect)
{
    KdCheckSync(pDrawable->pScreen);
    fbPolyFillRect (pDrawable, pGC, nrect, prect);
}

void
KdCheckPolyFillArc (DrawablePtr pDrawable, GCPtr pGC, 
		    int narcs, xArc *pArcs)
{
    KdCheckSync(pDrawable->pScreen);
    fbPolyFillArc (pDrawable, pGC, narcs, pArcs);
}

void
KdCheckImageGlyphBlt (DrawablePtr pDrawable, GCPtr pGC,
		      int x, int y, unsigned int nglyph,
		      CharInfoPtr *ppci, pointer pglyphBase)
{
    KdCheckSync(pDrawable->pScreen);
    fbImageGlyphBlt (pDrawable, pGC, x, y, nglyph, ppci, pglyphBase);
}

void
KdCheckPolyGlyphBlt (DrawablePtr pDrawable, GCPtr pGC,
		     int x, int y, unsigned int nglyph,
		     CharInfoPtr *ppci, pointer pglyphBase)
{
    KdCheckSync(pDrawable->pScreen);
    fbPolyGlyphBlt (pDrawable, pGC, x, y, nglyph, ppci, pglyphBase);
}

void
KdCheckPushPixels (GCPtr pGC, PixmapPtr pBitmap,
		   DrawablePtr pDrawable,
		   int w, int h, int x, int y)
{
    KdCheckSync(pDrawable->pScreen);
    fbPushPixels (pGC, pBitmap, pDrawable, w, h, x, y);
}

void
KdCheckGetImage (DrawablePtr pDrawable,
		 int x, int y, int w, int h,
		 unsigned int format, unsigned long planeMask,
		 char *d)
{
    KdCheckSync(pDrawable->pScreen);
    fbGetImage (pDrawable, x, y, w, h, format, planeMask, d);
}

void
KdCheckGetSpans (DrawablePtr pDrawable,
		 int wMax,
		 DDXPointPtr ppt,
		 int *pwidth,
		 int nspans,
		 char *pdstStart)
{
    if (pDrawable->type != DRAWABLE_PIXMAP)
	KdCheckSync(pDrawable->pScreen);
    fbGetSpans (pDrawable, wMax, ppt, pwidth, nspans, pdstStart);
}

void
KdCheckSaveAreas (PixmapPtr	pPixmap,
		  RegionPtr	prgnSave,
		  int		xorg,
		  int		yorg,
		  WindowPtr	pWin)
{
    KdCheckSync(pWin->drawable.pScreen);
    fbSaveAreas (pPixmap, prgnSave, xorg, yorg, pWin);
}

void
KdCheckRestoreAreas (PixmapPtr	pPixmap,
		     RegionPtr	prgnSave,
		     int	xorg,
		     int    	yorg,
		     WindowPtr	pWin)
{
    KdCheckSync(pWin->drawable.pScreen);
    fbRestoreAreas (pPixmap, prgnSave, xorg, yorg, pWin);
}

void
KdCheckPaintWindow (WindowPtr pWin, RegionPtr pRegion, int what)
{
    KdCheckSync (pWin->drawable.pScreen);
    fbPaintWindow (pWin, pRegion, what);
}

void
KdCheckCopyWindow (WindowPtr pWin, DDXPointRec ptOldOrg, RegionPtr prgnSrc)
{
    KdCheckSync (pWin->drawable.pScreen);
    fbCopyWindow (pWin, ptOldOrg, prgnSrc);
}

#if KD_MAX_FB > 1
void
KdCheckPaintKey(DrawablePtr  pDrawable,
		RegionPtr    pRegion,
		CARD32       pixel,
		int          layer)
{
    KdCheckSync (pDrawable->pScreen);
    fbOverlayPaintKey (pDrawable,  pRegion, pixel, layer);
}

void
KdCheckOverlayCopyWindow  (WindowPtr pWin, DDXPointRec ptOldOrg, RegionPtr prgnSrc)
{
    KdCheckSync (pWin->drawable.pScreen);
    fbOverlayCopyWindow (pWin, ptOldOrg, prgnSrc);
}
#endif

void
KdScreenInitAsync (ScreenPtr pScreen)
{
    pScreen->GetImage = KdCheckGetImage;
    pScreen->GetSpans = KdCheckGetSpans;
    pScreen->PaintWindowBackground = KdCheckPaintWindow;
    pScreen->PaintWindowBorder = KdCheckPaintWindow;
    pScreen->CopyWindow = KdCheckCopyWindow;
    
#ifndef FB_OLD_SCREEN
    pScreen->BackingStoreFuncs.SaveAreas = KdCheckSaveAreas;
    pScreen->BackingStoreFuncs.RestoreAreas = KdCheckRestoreAreas;
#else
    pScreenPriv->BackingStoreFuncs.SaveAreas = KdCheckSaveAreas;
    pScreenPriv->BackingStoreFuncs.RestoreAreas = KdCheckRestoreAreas;
#endif
#ifdef RENDER
    KdPictureInitAsync (pScreen);
#endif
}

/*
 * Only need to stall for copyarea/copyplane
 */
const GCOps kdAsyncPixmapGCOps = {
    fbFillSpans,
    fbSetSpans,
    fbPutImage,
    KdCheckCopyArea,
    KdCheckCopyPlane,
    fbPolyPoint,
    fbPolyLine,
    fbPolySegment,
    fbPolyRectangle,
    fbPolyArc,
    fbFillPolygon,
    fbPolyFillRect,
    fbPolyFillArc,
    miPolyText8,
    miPolyText16,
    miImageText8,
    miImageText16,
    fbImageGlyphBlt,
    fbPolyGlyphBlt,
    fbPushPixels
#ifdef NEED_LINEHELPER
    ,NULL
#endif
};
