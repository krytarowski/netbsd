/*
 * Copyright 1998-2003 VIA Technologies, Inc. All Rights Reserved.
 * Copyright 2001-2003 S3 Graphics, Inc. All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * VIA, S3 GRAPHICS, AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */


#ifndef __SAVAGE_DRI_H__
#define __SAVAGE_DRI_H__

#include "xf86drm.h"
#include "drm.h"

#define SAVAGE_DEFAULT_AGP_MODE     1
#define SAVAGE_MAX_AGP_MODE         4

/* Buffer are aligned on 4096 byte boundaries.
 */
/*  this is used for backbuffer, depthbuffer, etc..*/
/*          alignment                                      */

#define SAVAGE_BUFFER_ALIGN	0x00000fff

typedef struct{
    drm_context_t ctxOwner;
    unsigned long agp_offset;
    unsigned long agp_handle;
    unsigned long map_handle;
    int flags;
} savageAgpBuffer , *savageAgpBufferPtr;

typedef struct _server{
   int reserved_map_agpstart;
   int reserved_map_idx;
   
#if 0
   int buffer_map_idx;
#endif 

   int sarea_priv_offset;

#if 0
   int primary_size;
   int warp_ucode_size;
#endif 

   int chipset;
   int sgram;     /* seems no use */

   unsigned int frontOffset;
   unsigned int frontPitch;
   unsigned int frontbufferSize;
   
   unsigned int backOffset;
   unsigned int backPitch;
   unsigned int backbufferSize;

   unsigned int depthOffset;
   unsigned int depthPitch;
   unsigned int depthbufferSize;

   unsigned int textureOffset;
   int textureSize;
   int logTextureGranularity;

   drmRegion agp;

   /* PCI mappings */
   drmRegion aperture;
   drmRegion registers;
   drmRegion status;

   /* AGP mappings */
#if 0
   drmRegion warp;
   drmRegion primary;
   drmRegion buffers;
#endif

   drmRegion agpTextures;
   int logAgpTextureGranularity;

#if 0
   drmBufMapPtr drmBuffers;
#endif
    /*for agp*/
    int numBuffer;
    savageAgpBufferPtr agpBuffer;
} SAVAGEDRIServerPrivateRec, *SAVAGEDRIServerPrivatePtr;

typedef struct {
   int chipset;
   int width;
   int height;
   int mem;
   int cpp;
   int zpp;

   int agpMode;
   
   drm_handle_t frontbuffer;
   unsigned int frontbufferSize;
   unsigned int frontOffset;
   unsigned int frontPitch;
   unsigned int frontBitmapDesc;   /*Bitmap Descriptior*/ 
   unsigned int IsfrontTiled;

   drm_handle_t backbuffer;
   unsigned int backbufferSize;
   unsigned int backOffset;
   unsigned int backPitch;
   unsigned int backBitmapDesc;   /*Bitmap Descriptior*/

   drm_handle_t depthbuffer;
   unsigned int depthbufferSize;
   unsigned int depthOffset;
   unsigned int depthPitch;
   unsigned int depthBitmapDesc;   /*Bitmap Descriptior*/



   drm_handle_t textures;
   drm_handle_t xvmcSurfHandle;
   unsigned int textureOffset;
   unsigned int textureSize;
   int logTextureGranularity;

   /* Allow calculation of setup dma addresses.
    */
   unsigned int agpBufferOffset;

   unsigned int agpTextureOffset;
   unsigned int agpTextureSize;
   drmRegion agpTextures;
   int logAgpTextureGranularity;

/*   unsigned int mAccess;*/

   drmRegion aperture;
   unsigned int aperturePitch;    /* in byte */


   drmRegion registers;
   drmRegion BCIcmdBuf;
   drmRegion status;

#if 0
   drmRegion primary;
   drmRegion buffers;
#endif
  /*For shadow status*/
  unsigned long sareaPhysAddr;

   unsigned int sarea_priv_offset;
  int shadowStatus;
} SAVAGEDRIRec, *SAVAGEDRIPtr;

#endif








