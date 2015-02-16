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
/* $XFree86: xc/programs/Xserver/hw/tinyx/mach64/mach64draw.c,v 1.1 2004/06/02 22:43:02 dawes Exp $ */
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

#include "mach64.h"
#include "mach64draw.h"

#include	"Xmd.h"
#include	"gcstruct.h"
#include	"scrnintstr.h"
#include	"pixmapstr.h"
#include	"regionstr.h"
#include	"mistruct.h"
#include	"fontstruct.h"
#include	"dixfontstr.h"
#include	"fb.h"
#include	"migc.h"
#include	"miline.h"
#include	"picturestr.h"

CARD8 mach64Rop[16] = {
    /* GXclear      */      0x01,         /* 0 */
    /* GXand        */      0x0c,         /* src AND dst */
    /* GXandReverse */      0x0d,         /* src AND NOT dst */
    /* GXcopy       */      0x07,         /* src */
    /* GXandInverted*/      0x0e,         /* NOT src AND dst */
    /* GXnoop       */      0x03,         /* dst */
    /* GXxor        */      0x05,         /* src XOR dst */
    /* GXor         */      0x0b,         /* src OR dst */
    /* GXnor        */      0x0f,         /* NOT src AND NOT dst */
    /* GXequiv      */      0x06,         /* NOT src XOR dst */
    /* GXinvert     */      0x00,         /* NOT dst */
    /* GXorReverse  */      0x0a,         /* src OR NOT dst */
    /* GXcopyInverted*/     0x04,         /* NOT src */
    /* GXorInverted */      0x09,         /* NOT src OR dst */
    /* GXnand       */      0x08,         /* NOT src OR NOT dst */
    /* GXset        */      0x02,         /* 1 */
};

#define MACH64_DRAW_COMBO_SOLID	0x1
#define MACH64_DRAW_COMBO_COPY	0x8

static Reg	*reg;
static CARD32	avail;
static CARD32	triple;

#define IDX(reg,n)  (&(reg)->n - &(reg)->CRTC_H_TOTAL_DISP)

void
mach64WaitAvail(Reg *reg, int n)
{
    if (avail < n)
    {
	while ((avail = ((reg->GUI_STAT) >> 16) & 0x3ff) < n)
	    ;
    }
    avail -= n;
}

void
mach64WaitIdle (Reg *reg)
{
    while (reg->GUI_STAT & 1)
	;
}

static Bool
mach64Setup (ScreenPtr	pScreen, CARD32 combo, int wait)
{
    KdScreenPriv(pScreen);
    mach64ScreenInfo(pScreenPriv);
    mach64CardInfo(pScreenPriv);

    avail = 0;
    reg = mach64c->reg;
    triple = mach64s->bpp24;
    if (!reg)
	return FALSE;
    
    mach64WaitAvail(reg, wait + 3);
    reg->DP_PIX_WIDTH = mach64s->DP_PIX_WIDTH;
    reg->USR1_DST_OFF_PITCH = mach64s->USR1_DST_OFF_PITCH;
    reg->DP_SET_GUI_ENGINE = mach64s->DP_SET_GUI_ENGINE | (combo << 20);
    return TRUE;
}

static Bool
mach64PrepareSolid (DrawablePtr    pDrawable,
		     int	    alu,
		     Pixel	    pm,
		     Pixel	    fg)
{
    if (!mach64Setup (pDrawable->pScreen, 1, 3))
	return FALSE;
    reg->DP_MIX = (mach64Rop[alu] << 16) | 0;
    reg->DP_WRITE_MSK = pm;
    reg->DP_FRGD_CLR = fg;
    return TRUE;
}

static void
mach64Solid (int x1, int y1, int x2, int y2)
{
    if (triple)
    {
	CARD32	traj;

	x1 *= 3;
	x2 *= 3;

	traj = (DST_X_DIR | 
		DST_Y_DIR |
		DST_24_ROT_EN | 
		DST_24_ROT((x1 / 4) % 6));
	mach64WaitAvail (reg, 1);
	reg->GUI_TRAJ_CNTL = traj;
    }
    mach64WaitAvail(reg,2);
    reg->DST_X_Y = MACH64_XY(x1,y1);
    reg->DST_WIDTH_HEIGHT = MACH64_XY(x2-x1,y2-y1);
}

static void
mach64DoneSolid (void)
{
}

static int copyDx;
static int copyDy;

static Bool
mach64PrepareCopy (DrawablePtr	pSrcDrawable,
		   DrawablePtr	pDstDrawable,
		   int		dx,
		   int		dy,
		   int		alu,
		   Pixel	pm)
{
    CARD32	combo = 8;

    if ((copyDx = dx) > 0)
	combo |= 1;
    if ((copyDy = dy) > 0)
	combo |= 2;
    if (!mach64Setup (pDstDrawable->pScreen, combo, 2))
	return FALSE;
    
    reg->DP_MIX = (mach64Rop[alu] << 16) | 0;
    reg->DP_WRITE_MSK = pm;
    return TRUE;
}

static void
mach64Copy (int srcX,
	    int srcY,
	    int dstX,
	    int dstY,
	    int w,
	    int h)
{
    if (triple)
    {
	CARD32	traj;

	srcX *= 3;
	dstX *= 3;
	w *= 3;

	traj = DST_24_ROT_EN | DST_24_ROT((dstX / 4) % 6);
	
	if (copyDx > 0)
	    traj |= 1;
	if (copyDy > 0)
	    traj |= 2;
	
	mach64WaitAvail (reg, 1);
	reg->GUI_TRAJ_CNTL = traj;
    }
    if (copyDx <= 0)
    {
	srcX += w - 1;
	dstX += w - 1;
    }
    if (copyDy <= 0)
    {
	srcY += h - 1;
	dstY += h - 1;
    }
    mach64WaitAvail (reg, 4);
    reg->SRC_Y_X = MACH64_YX(srcX, srcY);
    reg->SRC_WIDTH1 = w;
    reg->DST_Y_X = MACH64_YX(dstX, dstY);
    reg->DST_HEIGHT_WIDTH = MACH64_YX(w,h);
}

static void
mach64DoneCopy (void)
{
}

KaaScreenPrivRec    mach64Kaa = {
    mach64PrepareSolid,
    mach64Solid,
    mach64DoneSolid,

    mach64PrepareCopy,
    mach64Copy,
    mach64DoneCopy,
};

Bool
mach64DrawInit (ScreenPtr pScreen)
{
    KdScreenPriv(pScreen);
    
    if (pScreenPriv->screen->fb[0].depth == 4)
	return FALSE;
    
    if (!kaaDrawInit (pScreen, &mach64Kaa))
	return FALSE;

    return TRUE;
}

#define PIX_FORMAT_MONO	0
#define PIX_FORMAT_PSEUDO_8	2
#define PIX_FORMAT_TRUE_1555	3
#define PIX_FORMAT_TRUE_565	4
#define PIX_FORMAT_TRUE_8888    6
#define PIX_FORMAT_TRUE_332	7
#define PIX_FORMAT_GRAY_8	8
#define PIX_FORMAT_YUV_422	0xb
#define PIX_FORMAT_YUV_444	0xe
#define PIX_FORMAT_TRUE_4444	0xf

void
mach64DrawEnable (ScreenPtr pScreen)
{
    KdScreenPriv(pScreen);
    mach64ScreenInfo(pScreenPriv);
    CARD32  DP_PIX_WIDTH;
    CARD32  SET_DP_DST_PIX_WIDTH;
    CARD32  DST1_PITCH;
    
    mach64s->bpp24 = FALSE;
    switch (pScreenPriv->screen->fb[0].depth) {
    case 1:
	DP_PIX_WIDTH = ((PIX_FORMAT_MONO << 0) |	/* DP_DST_PIX_WIDTH */
			(PIX_FORMAT_TRUE_8888 << 4) |	/* COMPOSITE_PIX_WIDTH */
			(PIX_FORMAT_MONO << 8) |	/* DP_SRC_PIX_WIDTH */
			(0 << 13) |			/* DP_HOST_TRIPLE_EN */
			(0 << 14) |			/* DP_PALETTE_TYPE */
			(PIX_FORMAT_MONO << 16) |	/* DP_HOST_PIX_WIDTH */
			(0 << 20) |			/* DP_C14_RGB_INDEX */
			(0 << 24) |			/* DP_BYTE_PIX_ORDER */
			(0 << 25) |			/* DP_CONVERSION_TEMP */
			(0 << 26) |			/* DP_C14_RGB_LOW_NIBBLE */
			(0 << 27) |			/* DP_C14_RGB_HIGH_NIBBLE */
			(PIX_FORMAT_TRUE_8888 << 28) |	/* DP_SCALE_PIX_WIDTH */
			0);
	SET_DP_DST_PIX_WIDTH = PIX_FORMAT_MONO;
	break;
    case 4:
	FatalError ("mach64 can't accelerate 4bpp");
	break;
    case 8:
	DP_PIX_WIDTH = ((PIX_FORMAT_PSEUDO_8 << 0) |	/* DP_DST_PIX_WIDTH */
			(PIX_FORMAT_TRUE_8888 << 4) |	/* COMPOSITE_PIX_WIDTH */
			(PIX_FORMAT_PSEUDO_8 << 8) |	/* DP_SRC_PIX_WIDTH */
			(0 << 13) |			/* DP_HOST_TRIPLE_EN */
			(0 << 14) |			/* DP_PALETTE_TYPE */
			(PIX_FORMAT_PSEUDO_8 << 16) |	/* DP_HOST_PIX_WIDTH */
			(0 << 20) |			/* DP_C14_RGB_INDEX */
			(0 << 24) |			/* DP_BYTE_PIX_ORDER */
			(0 << 25) |			/* DP_CONVERSION_TEMP */
			(0 << 26) |			/* DP_C14_RGB_LOW_NIBBLE */
			(0 << 27) |			/* DP_C14_RGB_HIGH_NIBBLE */
			(PIX_FORMAT_TRUE_8888 << 28) |	/* DP_SCALE_PIX_WIDTH */
			0);
	SET_DP_DST_PIX_WIDTH = PIX_FORMAT_PSEUDO_8;
	break;
    case 15:
	DP_PIX_WIDTH = ((PIX_FORMAT_TRUE_1555 << 0) |	/* DP_DST_PIX_WIDTH */
			(PIX_FORMAT_TRUE_8888 << 4) |	/* COMPOSITE_PIX_WIDTH */
			(PIX_FORMAT_TRUE_1555 << 8) |	/* DP_SRC_PIX_WIDTH */
			(0 << 13) |			/* DP_HOST_TRIPLE_EN */
			(0 << 14) |			/* DP_PALETTE_TYPE */
			(PIX_FORMAT_TRUE_1555 << 16) |	/* DP_HOST_PIX_WIDTH */
			(0 << 20) |			/* DP_C14_RGB_INDEX */
			(0 << 24) |			/* DP_BYTE_PIX_ORDER */
			(0 << 25) |			/* DP_CONVERSION_TEMP */
			(0 << 26) |			/* DP_C14_RGB_LOW_NIBBLE */
			(0 << 27) |			/* DP_C14_RGB_HIGH_NIBBLE */
			(PIX_FORMAT_TRUE_8888 << 28) |	/* DP_SCALE_PIX_WIDTH */
			0);
	SET_DP_DST_PIX_WIDTH = PIX_FORMAT_TRUE_1555;
	break;
    case 16:
	DP_PIX_WIDTH = ((PIX_FORMAT_TRUE_565 << 0) |	/* DP_DST_PIX_WIDTH */
			(PIX_FORMAT_TRUE_8888 << 4) |	/* COMPOSITE_PIX_WIDTH */
			(PIX_FORMAT_TRUE_565 << 8) |	/* DP_SRC_PIX_WIDTH */
			(0 << 13) |			/* DP_HOST_TRIPLE_EN */
			(0 << 14) |			/* DP_PALETTE_TYPE */
			(PIX_FORMAT_TRUE_565 << 16) |	/* DP_HOST_PIX_WIDTH */
			(0 << 20) |			/* DP_C14_RGB_INDEX */
			(0 << 24) |			/* DP_BYTE_PIX_ORDER */
			(0 << 25) |			/* DP_CONVERSION_TEMP */
			(0 << 26) |			/* DP_C14_RGB_LOW_NIBBLE */
			(0 << 27) |			/* DP_C14_RGB_HIGH_NIBBLE */
			(PIX_FORMAT_TRUE_8888 << 28) |	/* DP_SCALE_PIX_WIDTH */
			0);
	SET_DP_DST_PIX_WIDTH = PIX_FORMAT_TRUE_565;
	break;
    case 24:
	if (pScreenPriv->screen->fb[0].bitsPerPixel == 24)
	{
	    mach64s->bpp24 = TRUE;
	    DP_PIX_WIDTH = ((PIX_FORMAT_PSEUDO_8 << 0) |    /* DP_DST_PIX_WIDTH */
			    (PIX_FORMAT_PSEUDO_8 << 4) |    /* COMPOSITE_PIX_WIDTH */
			    (PIX_FORMAT_PSEUDO_8 << 8) |    /* DP_SRC_PIX_WIDTH */
			    (0 << 13) |			    /* DP_HOST_TRIPLE_EN */
			    (0 << 14) |			    /* DP_PALETTE_TYPE */
			    (PIX_FORMAT_PSEUDO_8 << 16) |   /* DP_HOST_PIX_WIDTH */
			    (0 << 20) |			    /* DP_C14_RGB_INDEX */
			    (0 << 24) |			    /* DP_BYTE_PIX_ORDER */
			    (0 << 25) |			    /* DP_CONVERSION_TEMP */
			    (0 << 26) |			    /* DP_C14_RGB_LOW_NIBBLE */
			    (0 << 27) |			    /* DP_C14_RGB_HIGH_NIBBLE */
			    (PIX_FORMAT_TRUE_8888 << 28) |  /* DP_SCALE_PIX_WIDTH */
			    0);
	    SET_DP_DST_PIX_WIDTH = PIX_FORMAT_PSEUDO_8;
	}
	else
	{
	    DP_PIX_WIDTH = ((PIX_FORMAT_TRUE_8888 << 0) |   /* DP_DST_PIX_WIDTH */
			    (PIX_FORMAT_TRUE_8888 << 4) |   /* COMPOSITE_PIX_WIDTH */
			    (PIX_FORMAT_TRUE_8888 << 8) |   /* DP_SRC_PIX_WIDTH */
			    (0 << 13) |			    /* DP_HOST_TRIPLE_EN */
			    (0 << 14) |			    /* DP_PALETTE_TYPE */
			    (PIX_FORMAT_TRUE_8888 << 16) |  /* DP_HOST_PIX_WIDTH */
			    (0 << 20) |			    /* DP_C14_RGB_INDEX */
			    (0 << 24) |			    /* DP_BYTE_PIX_ORDER */
			    (0 << 25) |			    /* DP_CONVERSION_TEMP */
			    (0 << 26) |			    /* DP_C14_RGB_LOW_NIBBLE */
			    (0 << 27) |			    /* DP_C14_RGB_HIGH_NIBBLE */
			    (PIX_FORMAT_TRUE_8888 << 28) |  /* DP_SCALE_PIX_WIDTH */
			    0);
	    SET_DP_DST_PIX_WIDTH = PIX_FORMAT_TRUE_8888;
	}
	break;
    default:
	FatalError("mach64DrawEnable: unsupported bpp: %d\n",
		   pScreenPriv->screen->fb[0].depth);
    }
    
    mach64s->DP_PIX_WIDTH = DP_PIX_WIDTH;
    DST1_PITCH = (pScreenPriv->screen->fb[0].pixelStride) >> 3;
    if (mach64s->bpp24)
	DST1_PITCH *= 3;
    mach64s->USR1_DST_OFF_PITCH = ((0 << 0) |		/* USR1_DST_OFFSET */
				   (DST1_PITCH << 22) |	/* USR1_DST_PITCH */
				   0);
    mach64s->DP_SET_GUI_ENGINE = ((SET_DP_DST_PIX_WIDTH << 3) |
				  (1 << 6) |		/* SET_DP_SRC_PIX_WIDTH */
				  (0 << 7) |		/* SET_DST_OFFSET */
				  (0 << 10) |		/* SET_DST_PITCH */
				  (0 << 14) |		/* SET_DST_PITCH_BY_2 */
				  (1 << 15) |		/* SET_SRC_OFFPITCH_COPY */
				  (0 << 16) |		/* SET_SRC_HGTWID1_2 */
				  (0 << 20) |		/* SET_DRAWING_COMBO */
				  (1 << 24) |		/* SET_BUS_MASTER_OP */
				  (0 << 26) |		/* SET_BUS_MASTER_EN */
				  (0 << 27) |		/* SET_BUS_MASTER_SYNC */
				  (0 << 28) |		/* DP_HOST_TRIPLE_EN */
				  (0 << 29) |		/* FAST_FILL_EN */
				  (0 << 30) |		/* BLOCK_WRITE_EN */
				  0);
    KdMarkSync (pScreen);
}

void
mach64DrawDisable (ScreenPtr pScreen)
{
}

void
mach64DrawFini (ScreenPtr pScreen)
{
}

void
mach64DrawSync (ScreenPtr pScreen)
{
    KdScreenPriv(pScreen);
    mach64CardInfo(pScreenPriv);
    reg = mach64c->reg;
    
    mach64WaitIdle (reg);
}
