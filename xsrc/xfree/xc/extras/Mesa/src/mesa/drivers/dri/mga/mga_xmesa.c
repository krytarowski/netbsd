/* $XFree86: xc/extras/Mesa/src/mesa/drivers/dri/mga/mga_xmesa.c,v 1.2 2004/12/13 22:40:52 tsi Exp $ */
/*
 * Copyright 2000-2001 VA Linux Systems, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * VA LINUX SYSTEMS AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Keith Whitwell <keith@tungstengraphics.com>
 */

#include <stdlib.h>
#include "drm.h"
#include "mga_drm.h"
#include "mga_xmesa.h"
#include "context.h"
#include "matrix.h"
#include "simple_list.h"
#include "imports.h"

#include "swrast/swrast.h"
#include "swrast_setup/swrast_setup.h"
#include "tnl/tnl.h"
#include "array_cache/acache.h"

#include "tnl/t_pipeline.h"

#include "drivers/common/driverfuncs.h"

#include "mgadd.h"
#include "mgastate.h"
#include "mgatex.h"
#include "mgaspan.h"
#include "mgaioctl.h"
#include "mgatris.h"
#include "mgavb.h"
#include "mgapixel.h"
#include "mga_xmesa.h"
#include "mga_dri.h"


#include "utils.h"
#include "vblank.h"

#include "extensions.h"

#include "GL/internal/dri_interface.h"

/* MGA configuration
 */
#include "xmlpool.h"

const char __driConfigOptions[] =
DRI_CONF_BEGIN
    DRI_CONF_SECTION_PERFORMANCE
        DRI_CONF_VBLANK_MODE(DRI_CONF_VBLANK_DEF_INTERVAL_0)
    DRI_CONF_SECTION_END
    DRI_CONF_SECTION_QUALITY
        DRI_CONF_TEXTURE_DEPTH(DRI_CONF_TEXTURE_DEPTH_FB)
        DRI_CONF_COLOR_REDUCTION(DRI_CONF_COLOR_REDUCTION_DITHER)
    DRI_CONF_SECTION_END
    DRI_CONF_SECTION_SOFTWARE
        DRI_CONF_ARB_VERTEX_PROGRAM(true)
        DRI_CONF_NV_VERTEX_PROGRAM(true)
    DRI_CONF_SECTION_END
DRI_CONF_END;
static const GLuint __driNConfigOptions = 5;

#ifdef USE_NEW_INTERFACE
static PFNGLXCREATECONTEXTMODES create_context_modes = NULL;
#endif /* USE_NEW_INTERFACE */

#ifndef MGA_DEBUG
int MGA_DEBUG = 0;
#endif

static int getSwapInfo( __DRIdrawablePrivate *dPriv, __DRIswapInfo * sInfo );

#ifdef USE_NEW_INTERFACE
static __GLcontextModes * fill_in_modes( __GLcontextModes * modes,
					 unsigned pixel_bits, 
					 unsigned depth_bits,
					 unsigned stencil_bits,
					 const GLenum * db_modes,
					 unsigned num_db_modes,
					 int visType )
{
    static const u_int8_t bits[2][4] = {
	{          5,          6,          5,          0 },
	{          8,          8,          8,          0 }
    };

    static const u_int32_t masks[2][4] = {
	{ 0x0000F800, 0x000007E0, 0x0000001F, 0x00000000 },
	{ 0x00FF0000, 0x0000FF00, 0x000000FF, 0x00000000 }
    };

    unsigned   i;
    unsigned   j;
    const unsigned index = ((pixel_bits + 15) / 16) - 1;

    for ( i = 0 ; i < num_db_modes ; i++ ) {
	for ( j = 0 ; j < 2 ; j++ ) {

	    modes->redBits   = bits[index][0];
	    modes->greenBits = bits[index][1];
	    modes->blueBits  = bits[index][2];
	    modes->alphaBits = bits[index][3];
	    modes->redMask   = masks[index][0];
	    modes->greenMask = masks[index][1];
	    modes->blueMask  = masks[index][2];
	    modes->alphaMask = masks[index][3];
	    modes->rgbBits   = modes->redBits + modes->greenBits
		+ modes->blueBits;

	    modes->accumRedBits   = 16 * j;
	    modes->accumGreenBits = 16 * j;
	    modes->accumBlueBits  = 16 * j;
	    modes->accumAlphaBits = (masks[index][3] != 0) ? 16 * j : 0;
	    modes->visualRating = (j == 0) ? GLX_NONE : GLX_SLOW_CONFIG;

	    modes->stencilBits = stencil_bits;
	    modes->depthBits = depth_bits;

	    modes->visualType = visType;
	    modes->renderType = GLX_RGBA_BIT;
	    modes->drawableType = GLX_WINDOW_BIT;
	    modes->rgbMode = GL_TRUE;

	    if ( db_modes[i] == GLX_NONE ) {
		modes->doubleBufferMode = GL_FALSE;
	    }
	    else {
		modes->doubleBufferMode = GL_TRUE;
		modes->swapMethod = db_modes[i];
	    }

	    modes = modes->next;
	}
    }
    
    return modes;
}
#endif /* USE_NEW_INTERFACE */


#ifdef USE_NEW_INTERFACE
static __GLcontextModes *
mgaFillInModes( unsigned pixel_bits, unsigned depth_bits,
		unsigned stencil_bits, GLboolean have_back_buffer )
{
    __GLcontextModes * modes;
    __GLcontextModes * m;
    unsigned num_modes;
    unsigned depth_buffer_factor;
    unsigned back_buffer_factor;
    unsigned i;

    /* GLX_SWAP_COPY_OML is only supported because the MGA driver doesn't
     * support pageflipping at all.
     */
    static const GLenum back_buffer_modes[] = {
	GLX_NONE, GLX_SWAP_UNDEFINED_OML, GLX_SWAP_COPY_OML
    };

    int depth_buffer_modes[2][2];


    depth_buffer_modes[0][0] = depth_bits;
    depth_buffer_modes[1][0] = depth_bits;

    /* Just like with the accumulation buffer, always provide some modes
     * with a stencil buffer.  It will be a sw fallback, but some apps won't
     * care about that.
     */
    depth_buffer_modes[0][1] = 0;
    depth_buffer_modes[1][1] = (stencil_bits == 0) ? 8 : stencil_bits;

    depth_buffer_factor = ((depth_bits != 0) || (stencil_bits != 0)) ? 2 : 1;
    back_buffer_factor  = (have_back_buffer) ? 2 : 1;

    num_modes = depth_buffer_factor * back_buffer_factor * 4;

    modes = (*create_context_modes)( num_modes, sizeof( __GLcontextModes ) );
    m = modes;
    for ( i = 0 ; i < depth_buffer_factor ; i++ ) {
	m = fill_in_modes( m, pixel_bits, 
			   depth_buffer_modes[i][0], depth_buffer_modes[i][1],
			   back_buffer_modes, back_buffer_factor,
			   GLX_TRUE_COLOR );
    }

    for ( i = 0 ; i < depth_buffer_factor ; i++ ) {
	m = fill_in_modes( m, pixel_bits, 
			   depth_buffer_modes[i][0], depth_buffer_modes[i][1],
			   back_buffer_modes, back_buffer_factor,
			   GLX_DIRECT_COLOR );
    }

    /* Mark the visual as slow if there are "fake" stencil bits.
     */
    for ( m = modes ; m != NULL ; m = m->next ) {
	if ( (m->stencilBits != 0) && (m->stencilBits != stencil_bits) ) {
	    m->visualRating = GLX_SLOW_CONFIG;
	}
    }

    return modes;
}
#endif /* USE_NEW_INTERFACE */


static GLboolean
mgaInitDriver(__DRIscreenPrivate *sPriv)
{
   mgaScreenPrivate *mgaScreen;
   MGADRIPtr         serverInfo = (MGADRIPtr)sPriv->pDevPriv;


   /* Allocate the private area */
   mgaScreen = (mgaScreenPrivate *)MALLOC(sizeof(mgaScreenPrivate));
   if (!mgaScreen) {
      __driUtilMessage("Couldn't malloc screen struct");
      return GL_FALSE;
   }

   mgaScreen->sPriv = sPriv;
   sPriv->private = (void *)mgaScreen;

   if (sPriv->drmMinor >= 1) {
      int ret;
      drm_mga_getparam_t gp;

      gp.param = MGA_PARAM_IRQ_NR;
      gp.value = &mgaScreen->irq;
      mgaScreen->irq = 0;

      ret = drmCommandWriteRead( sPriv->fd, DRM_MGA_GETPARAM,
				    &gp, sizeof(gp));
      if (ret) {
	    fprintf(stderr, "drmMgaGetParam (MGA_PARAM_IRQ_NR): %d\n", ret);
	    FREE(mgaScreen);
	    sPriv->private = NULL;
	    return GL_FALSE;
      }
   }
   
   mgaScreen->linecomp_sane = (sPriv->ddxMajor > 1) || (sPriv->ddxMinor > 1)
       || ((sPriv->ddxMinor == 1) && (sPriv->ddxPatch > 0));

   if ( driCompareGLXAPIVersion( 20030813 ) >= 0 ) {
      PFNGLXSCRENABLEEXTENSIONPROC glx_enable_extension =
          (PFNGLXSCRENABLEEXTENSIONPROC) glXGetProcAddress( (const GLubyte *) "__glXScrEnableExtension" );
      void * const psc = sPriv->psc->screenConfigs;

      if ( glx_enable_extension != NULL ) {
	 if ( mgaScreen->linecomp_sane ) {
	    (*glx_enable_extension)( psc, "GLX_SGI_swap_control" );
	    (*glx_enable_extension)( psc, "GLX_SGI_video_sync" );
	    (*glx_enable_extension)( psc, "GLX_MESA_swap_control" );
	 }

	 (*glx_enable_extension)( psc, "GLX_SGI_make_current_read" );
	 (*glx_enable_extension)( psc, "GLX_MESA_swap_frame_usage" );

	 if ( driCompareGLXAPIVersion( 20030915 ) >= 0 ) {
	    (*glx_enable_extension)( psc, "GLX_SGIX_fbconfig" );
	    (*glx_enable_extension)( psc, "GLX_OML_swap_method" );
	 }
      }
   }

   if (serverInfo->chipset != MGA_CARD_TYPE_G200 &&
       serverInfo->chipset != MGA_CARD_TYPE_G400) {
      FREE(mgaScreen);
      sPriv->private = NULL;
      __driUtilMessage("Unrecognized chipset");
      return GL_FALSE;
   }


   mgaScreen->chipset = serverInfo->chipset;
   mgaScreen->width = serverInfo->width;
   mgaScreen->height = serverInfo->height;
   mgaScreen->mem = serverInfo->mem;
   mgaScreen->cpp = serverInfo->cpp;

   mgaScreen->agpMode = serverInfo->agpMode;

   mgaScreen->frontPitch = serverInfo->frontPitch;
   mgaScreen->frontOffset = serverInfo->frontOffset;
   mgaScreen->backOffset = serverInfo->backOffset;
   mgaScreen->backPitch  =  serverInfo->backPitch;
   mgaScreen->depthOffset = serverInfo->depthOffset;
   mgaScreen->depthPitch  =  serverInfo->depthPitch;

   mgaScreen->mmio.handle = serverInfo->registers.handle;
   mgaScreen->mmio.size = serverInfo->registers.size;
   if ( drmMap( sPriv->fd,
		mgaScreen->mmio.handle, mgaScreen->mmio.size,
		&mgaScreen->mmio.map ) < 0 ) {
      FREE( mgaScreen );
      sPriv->private = NULL;
      __driUtilMessage( "Couldn't map MMIO registers" );
      return GL_FALSE;
   }

   mgaScreen->primary.handle = serverInfo->primary.handle;
   mgaScreen->primary.size = serverInfo->primary.size;
   mgaScreen->buffers.handle = serverInfo->buffers.handle;
   mgaScreen->buffers.size = serverInfo->buffers.size;

#if 0
   mgaScreen->agp.handle = serverInfo->agp;
   mgaScreen->agp.size = serverInfo->agpSize;

   if (drmMap(sPriv->fd,
	      mgaScreen->agp.handle,
	      mgaScreen->agp.size,
	      (drmAddress *)&mgaScreen->agp.map) != 0)
   {
      Xfree(mgaScreen);
      sPriv->private = NULL;
      __driUtilMessage("Couldn't map agp region");
      return GL_FALSE;
   }
#endif

   mgaScreen->textureOffset[MGA_CARD_HEAP] = serverInfo->textureOffset;
   mgaScreen->textureOffset[MGA_AGP_HEAP] = (serverInfo->agpTextureOffset |
					     PDEA_pagpxfer_enable | 1);

   mgaScreen->textureSize[MGA_CARD_HEAP] = serverInfo->textureSize;
   mgaScreen->textureSize[MGA_AGP_HEAP] = serverInfo->agpTextureSize;

   mgaScreen->logTextureGranularity[MGA_CARD_HEAP] =
      serverInfo->logTextureGranularity;
   mgaScreen->logTextureGranularity[MGA_AGP_HEAP] =
      serverInfo->logAgpTextureGranularity;

   mgaScreen->texVirtual[MGA_CARD_HEAP] = (char *)(mgaScreen->sPriv->pFB +
					   serverInfo->textureOffset);
   if (drmMap(sPriv->fd,
              serverInfo->agpTextureOffset,
              serverInfo->agpTextureSize,
              (drmAddress *)&mgaScreen->texVirtual[MGA_AGP_HEAP]) != 0)
   {
      FREE(mgaScreen);
      sPriv->private = NULL;
      __driUtilMessage("Couldn't map agptexture region");
      return GL_FALSE;
   }

#if 0
   mgaScreen->texVirtual[MGA_AGP_HEAP] = (mgaScreen->agp.map +
					  serverInfo->agpTextureOffset);
#endif

   mgaScreen->mAccess = serverInfo->mAccess;

   /* For calculating setupdma addresses.
    */
   mgaScreen->dmaOffset = serverInfo->buffers.handle;

   mgaScreen->bufs = drmMapBufs(sPriv->fd);
   if (!mgaScreen->bufs) {
      /*drmUnmap(mgaScreen->agp_tex.map, mgaScreen->agp_tex.size);*/
      FREE(mgaScreen);
      sPriv->private = NULL;
      __driUtilMessage("Couldn't map dma buffers");
      return GL_FALSE;
   }
   mgaScreen->sarea_priv_offset = serverInfo->sarea_priv_offset;

   /* parse information in __driConfigOptions */
   driParseOptionInfo (&mgaScreen->optionCache,
		       __driConfigOptions, __driNConfigOptions);

   return GL_TRUE;
}


static void
mgaDestroyScreen(__DRIscreenPrivate *sPriv)
{
   mgaScreenPrivate *mgaScreen = (mgaScreenPrivate *) sPriv->private;

   if (MGA_DEBUG&DEBUG_VERBOSE_DRI)
      fprintf(stderr, "mgaDestroyScreen\n");

   drmUnmapBufs(mgaScreen->bufs);

   /*drmUnmap(mgaScreen->agp_tex.map, mgaScreen->agp_tex.size);*/

   /* free all option information */
   driDestroyOptionInfo (&mgaScreen->optionCache);

   FREE(mgaScreen);
   sPriv->private = NULL;
}


extern const struct tnl_pipeline_stage _mga_render_stage;

static const struct tnl_pipeline_stage *mga_pipeline[] = {
   &_tnl_vertex_transform_stage, 
   &_tnl_normal_transform_stage, 
   &_tnl_lighting_stage,	
   &_tnl_fog_coordinate_stage,
   &_tnl_texgen_stage, 
   &_tnl_texture_transform_stage, 
   &_tnl_vertex_program_stage,

				/* REMOVE: point attenuation stage */
#if 0
   &_mga_render_stage,		/* ADD: unclipped rastersetup-to-dma */
                                /* Need new ioctl for wacceptseq */
#endif
   &_tnl_render_stage,		
   0,
};


static const char * const g400_extensions[] =
{
   "GL_ARB_multitexture",
   "GL_ARB_texture_env_add",
   "GL_ARB_texture_env_combine",
   "GL_ARB_texture_env_crossbar",
   "GL_EXT_texture_env_combine",
   "GL_EXT_texture_edge_clamp",
   "GL_ATI_texture_env_combine3",
#if defined (MESA_packed_depth_stencil)
   "GL_MESA_packed_depth_stencil",
#endif
   NULL
};

static const char * const card_extensions[] =
{
   "GL_ARB_multisample",
   "GL_ARB_texture_compression",
   "GL_ARB_texture_rectangle",
   "GL_EXT_blend_logic_op",
   "GL_EXT_fog_coord",
   "GL_EXT_multi_draw_arrays",
   /* paletted_textures currently doesn't work, but we could fix them later */
#if 0
   "GL_EXT_shared_texture_palette",
   "GL_EXT_paletted_texture",
#endif
   "GL_EXT_secondary_color",
   "GL_EXT_stencil_wrap",
   "GL_MESA_ycbcr_texture",
   "GL_SGIS_generate_mipmap",
   NULL
};

static const struct dri_debug_control debug_control[] =
{
    { "fall",  DEBUG_VERBOSE_FALLBACK },
    { "tex",   DEBUG_VERBOSE_TEXTURE },
    { "ioctl", DEBUG_VERBOSE_IOCTL },
    { "verb",  DEBUG_VERBOSE_MSG },
    { "dri",   DEBUG_VERBOSE_DRI },
    { NULL,    0 }
};


static int
get_ust_nop( int64_t * ust )
{
   *ust = 1;
   return 0;
}


static GLboolean
mgaCreateContext( const __GLcontextModes *mesaVis,
                  __DRIcontextPrivate *driContextPriv,
                  void *sharedContextPrivate )
{
   int i;
   unsigned   maxlevels;
   GLcontext *ctx, *shareCtx;
   mgaContextPtr mmesa;
   __DRIscreenPrivate *sPriv = driContextPriv->driScreenPriv;
   mgaScreenPrivate *mgaScreen = (mgaScreenPrivate *)sPriv->private;
   drm_mga_sarea_t *saPriv = (drm_mga_sarea_t *)(((char*)sPriv->pSAREA)+
					      mgaScreen->sarea_priv_offset);
   struct dd_function_table functions;

   if (MGA_DEBUG&DEBUG_VERBOSE_DRI)
      fprintf(stderr, "mgaCreateContext\n");

   /* allocate mga context */
   mmesa = (mgaContextPtr) CALLOC(sizeof(mgaContext));
   if (!mmesa) {
      return GL_FALSE;
   }

   /* Init default driver functions then plug in our Radeon-specific functions
    * (the texture functions are especially important)
    */
   _mesa_init_driver_functions( &functions );
   mgaInitDriverFuncs( &functions );
   mgaInitTextureFuncs( &functions );
   mgaInitIoctlFuncs( &functions );

   /* Allocate the Mesa context */
   if (sharedContextPrivate)
      shareCtx = ((mgaContextPtr) sharedContextPrivate)->glCtx;
   else 
      shareCtx = NULL;
   mmesa->glCtx = _mesa_create_context(mesaVis, shareCtx,
                                       &functions, (void *) mmesa);
   if (!mmesa->glCtx) {
      FREE(mmesa);
      return GL_FALSE;
   }
   driContextPriv->driverPrivate = mmesa;

   /* Init mga state */
   mmesa->hHWContext = driContextPriv->hHWContext;
   mmesa->driFd = sPriv->fd;
   mmesa->driHwLock = &sPriv->pSAREA->lock;

   mmesa->mgaScreen = mgaScreen;
   mmesa->driScreen = sPriv;
   mmesa->sarea = (void *)saPriv;

   /* Parse configuration files */
   driParseConfigFiles (&mmesa->optionCache, &mgaScreen->optionCache,
                        sPriv->myNum, "mga");

   (void) memset( mmesa->texture_heaps, 0, sizeof( mmesa->texture_heaps ) );
   make_empty_list( & mmesa->swapped );

   mmesa->nr_heaps = mgaScreen->texVirtual[MGA_AGP_HEAP] ? 2 : 1;
   for ( i = 0 ; i < mmesa->nr_heaps ; i++ ) {
      mmesa->texture_heaps[i] = driCreateTextureHeap( i, mmesa,
	    mgaScreen->textureSize[i],
	    6,
	    MGA_NR_TEX_REGIONS,
	    (drmTextureRegionPtr)mmesa->sarea->texList[i],
	    &mmesa->sarea->texAge[i],
	    &mmesa->swapped,
	    sizeof( mgaTextureObject_t ),
	    (destroy_texture_object_t *) mgaDestroyTexObj );
   }

   /* Set the maximum texture size small enough that we can guarentee
    * that both texture units can bind a maximal texture and have them
    * on the card at once.
    */
   ctx = mmesa->glCtx;
   if ( mgaScreen->chipset == MGA_CARD_TYPE_G200 ) {
      ctx->Const.MaxTextureUnits = 1;
      ctx->Const.MaxTextureImageUnits = 1;
      ctx->Const.MaxTextureCoordUnits = 1;
      maxlevels = G200_TEX_MAXLEVELS;

   }
   else {
      ctx->Const.MaxTextureUnits = 2;
      ctx->Const.MaxTextureImageUnits = 2;
      ctx->Const.MaxTextureCoordUnits = 2;
      maxlevels = G400_TEX_MAXLEVELS;
   }

   driCalculateMaxTextureLevels( mmesa->texture_heaps,
				 mmesa->nr_heaps,
				 & ctx->Const,
				 4,
				 11, /* max 2D texture size is 2048x2048 */
				 0,  /* 3D textures unsupported. */
				 0,  /* cube textures unsupported. */
				 11, /* max texture rect size is 2048x2048 */
				 maxlevels,
				 GL_FALSE );

   ctx->Const.MinLineWidth = 1.0;
   ctx->Const.MinLineWidthAA = 1.0;
   ctx->Const.MaxLineWidth = 10.0;
   ctx->Const.MaxLineWidthAA = 10.0;
   ctx->Const.LineWidthGranularity = 1.0;

   mmesa->texture_depth = driQueryOptioni (&mmesa->optionCache,
					   "texture_depth");
   if (mmesa->texture_depth == DRI_CONF_TEXTURE_DEPTH_FB)
      mmesa->texture_depth = ( mesaVis->rgbBits >= 24 ) ?
	 DRI_CONF_TEXTURE_DEPTH_32 : DRI_CONF_TEXTURE_DEPTH_16;
   mmesa->hw_stencil = mesaVis->stencilBits && mesaVis->depthBits == 24;

   switch (mesaVis->depthBits) {
   case 16: 
      mmesa->depth_scale = 1.0/(GLdouble)0xffff; 
      mmesa->depth_clear_mask = ~0;
      mmesa->ClearDepth = 0xffff;
      break;
   case 24:
      mmesa->depth_scale = 1.0/(GLdouble)0xffffff;
      if (mmesa->hw_stencil) {
	 mmesa->depth_clear_mask = 0xffffff00;
	 mmesa->stencil_clear_mask = 0x000000ff;
      } else
	 mmesa->depth_clear_mask = ~0;
      mmesa->ClearDepth = 0xffffff00;
      break;
   case 32:
      mmesa->depth_scale = 1.0/(GLdouble)0xffffffff;
      mmesa->depth_clear_mask = ~0;
      mmesa->ClearDepth = 0xffffffff;
      break;
   };

   mmesa->haveHwStipple = GL_FALSE;
   mmesa->RenderIndex = -1;		/* impossible value */
   mmesa->dirty = ~0;
   mmesa->vertex_format = 0;   
   mmesa->CurrentTexObj[0] = 0;
   mmesa->CurrentTexObj[1] = 0;
   mmesa->tmu_source[0] = 0;
   mmesa->tmu_source[1] = 1;

   mmesa->texAge[0] = 0;
   mmesa->texAge[1] = 0;
   
   /* Initialize the software rasterizer and helper modules.
    */
   _swrast_CreateContext( ctx );
   _ac_CreateContext( ctx );
   _tnl_CreateContext( ctx );
   
   _swsetup_CreateContext( ctx );

   /* Install the customized pipeline:
    */
   _tnl_destroy_pipeline( ctx );
   _tnl_install_pipeline( ctx, mga_pipeline );

   /* Configure swrast and T&L to match hardware characteristics:
    */
   _swrast_allow_pixel_fog( ctx, GL_FALSE );
   _swrast_allow_vertex_fog( ctx, GL_TRUE );
   _tnl_allow_pixel_fog( ctx, GL_FALSE );
   _tnl_allow_vertex_fog( ctx, GL_TRUE );

   mmesa->primary_offset = mmesa->mgaScreen->primary.handle;

   ctx->DriverCtx = (void *) mmesa;
   mmesa->glCtx = ctx;

   driInitExtensions( ctx, card_extensions, GL_FALSE );

   if (MGA_IS_G400(MGA_CONTEXT(ctx))) {
      driInitExtensions( ctx, g400_extensions, GL_FALSE );
   }

   if ( driQueryOptionb( &mmesa->optionCache, "arb_vertex_program" ) ) {
      _mesa_enable_extension( ctx, "GL_ARB_vertex_program" );
   }
   
   if ( driQueryOptionb( &mmesa->optionCache, "nv_vertex_program" ) ) {
      _mesa_enable_extension( ctx, "GL_NV_vertex_program" );
      _mesa_enable_extension( ctx, "GL_NV_vertex_program1_1" );
   }

	
   /* XXX these should really go right after _mesa_init_driver_functions() */
   mgaDDInitStateFuncs( ctx );
   mgaDDInitSpanFuncs( ctx );
   mgaDDInitPixelFuncs( ctx );
   mgaDDInitTriFuncs( ctx );

   mgaInitVB( ctx );
   mgaInitState( mmesa );

   driContextPriv->driverPrivate = (void *) mmesa;

#if DO_DEBUG
   MGA_DEBUG = driParseDebugString( getenv( "MGA_DEBUG" ),
				    debug_control );
#endif

   mmesa->vblank_flags = ((mmesa->mgaScreen->irq == 0) 
			  || !mmesa->mgaScreen->linecomp_sane)
       ? VBLANK_FLAG_NO_IRQ : driGetDefaultVBlankFlags(&mmesa->optionCache);

   mmesa->get_ust = (PFNGLXGETUSTPROC) glXGetProcAddress( (const GLubyte *) "__glXGetUST" );
   if ( mmesa->get_ust == NULL ) {
      mmesa->get_ust = get_ust_nop;
   }

   (*mmesa->get_ust)( & mmesa->swap_ust );

   return GL_TRUE;
}

static void
mgaDestroyContext(__DRIcontextPrivate *driContextPriv)
{
   mgaContextPtr mmesa = (mgaContextPtr) driContextPriv->driverPrivate;

   if (MGA_DEBUG&DEBUG_VERBOSE_DRI)
      fprintf( stderr, "[%s:%d] mgaDestroyContext start\n",
	       __FILE__, __LINE__ );

   assert(mmesa); /* should never be null */
   if (mmesa) {
      GLboolean   release_texture_heaps;


      release_texture_heaps = (mmesa->glCtx->Shared->RefCount == 1);
      _swsetup_DestroyContext( mmesa->glCtx );
      _tnl_DestroyContext( mmesa->glCtx );
      _ac_DestroyContext( mmesa->glCtx );
      _swrast_DestroyContext( mmesa->glCtx );

      mgaFreeVB( mmesa->glCtx );

      /* free the Mesa context */
      mmesa->glCtx->DriverCtx = NULL;
      _mesa_destroy_context(mmesa->glCtx);
       
      if ( release_texture_heaps ) {
         /* This share group is about to go away, free our private
          * texture object data.
          */
         int i;

         for ( i = 0 ; i < mmesa->nr_heaps ; i++ ) {
	    driDestroyTextureHeap( mmesa->texture_heaps[ i ] );
	    mmesa->texture_heaps[ i ] = NULL;
         }

	 assert( is_empty_list( & mmesa->swapped ) );
      }

      /* free the option cache */
      driDestroyOptionCache (&mmesa->optionCache);

      FREE(mmesa);
   }

   if (MGA_DEBUG&DEBUG_VERBOSE_DRI)
      fprintf( stderr, "[%s:%d] mgaDestroyContext done\n",
	       __FILE__, __LINE__ );
}


static GLboolean
mgaCreateBuffer( __DRIscreenPrivate *driScrnPriv,
                 __DRIdrawablePrivate *driDrawPriv,
                 const __GLcontextModes *mesaVis,
                 GLboolean isPixmap )
{
   if (isPixmap) {
      return GL_FALSE; /* not implemented */
   }
   else {
      GLboolean swStencil = (mesaVis->stencilBits > 0 && 
			     mesaVis->depthBits != 24);

      driDrawPriv->driverPrivate = (void *) 
         _mesa_create_framebuffer(mesaVis,
                                  GL_FALSE,  /* software depth buffer? */
                                  swStencil,
                                  mesaVis->accumRedBits > 0,
                                  mesaVis->alphaBits > 0 );

      return (driDrawPriv->driverPrivate != NULL);
   }
}


static void
mgaDestroyBuffer(__DRIdrawablePrivate *driDrawPriv)
{
   _mesa_destroy_framebuffer((GLframebuffer *) (driDrawPriv->driverPrivate));
}

static void
mgaSwapBuffers(__DRIdrawablePrivate *dPriv)
{
   if (dPriv->driContextPriv && dPriv->driContextPriv->driverPrivate) {
      mgaContextPtr mmesa;
      GLcontext *ctx;
      mmesa = (mgaContextPtr) dPriv->driContextPriv->driverPrivate;
      ctx = mmesa->glCtx;

      if (ctx->Visual.doubleBufferMode) {
         _mesa_notifySwapBuffers( ctx );
         mgaCopyBuffer( dPriv );
      }
   } else {
      /* XXX this shouldn't be an error but we can't handle it for now */
      _mesa_problem(NULL, "%s: drawable has no context!\n", __FUNCTION__);
   }
}

static GLboolean
mgaUnbindContext(__DRIcontextPrivate *driContextPriv)
{
   mgaContextPtr mmesa = (mgaContextPtr) driContextPriv->driverPrivate;
   if (mmesa)
      mmesa->dirty = ~0;

   return GL_TRUE;
}

/* This looks buggy to me - the 'b' variable isn't used anywhere...
 * Hmm - It seems that the drawable is already hooked in to
 * driDrawablePriv.
 *
 * But why are we doing context initialization here???
 */
static GLboolean
mgaMakeCurrent(__DRIcontextPrivate *driContextPriv,
               __DRIdrawablePrivate *driDrawPriv,
               __DRIdrawablePrivate *driReadPriv)
{
   if (driContextPriv) {
      mgaContextPtr mmesa = (mgaContextPtr) driContextPriv->driverPrivate;

      if (mmesa->driDrawable != driDrawPriv) {
	 driDrawableInitVBlank( driDrawPriv, mmesa->vblank_flags );
	 mmesa->driDrawable = driDrawPriv;
	 mmesa->dirty = ~0; 
	 mmesa->dirty_cliprects = (MGA_FRONT|MGA_BACK); 
	 mmesa->mesa_drawable = driDrawPriv;
      }

      mmesa->driReadable = driReadPriv;

      _mesa_make_current2(mmesa->glCtx,
                          (GLframebuffer *) driDrawPriv->driverPrivate,
                          (GLframebuffer *) driReadPriv->driverPrivate);

      if (!mmesa->glCtx->Viewport.Width)
	 _mesa_set_viewport(mmesa->glCtx, 0, 0,
                            driDrawPriv->w, driDrawPriv->h);

   }
   else {
      _mesa_make_current(NULL, NULL);
   }

   return GL_TRUE;
}


void mgaGetLock( mgaContextPtr mmesa, GLuint flags )
{
   __DRIdrawablePrivate *dPriv = mmesa->driDrawable;
   drm_mga_sarea_t *sarea = mmesa->sarea;
   int me = mmesa->hHWContext;
   int i;

   drmGetLock(mmesa->driFd, mmesa->hHWContext, flags);

   if (*(dPriv->pStamp) != mmesa->lastStamp) {
      mmesa->lastStamp = *(dPriv->pStamp);
      mmesa->SetupNewInputs |= VERT_BIT_POS;
      mmesa->dirty_cliprects = (MGA_FRONT|MGA_BACK);
      mgaUpdateRects( mmesa, (MGA_FRONT|MGA_BACK) );
   }

   mmesa->dirty |= MGA_UPLOAD_CONTEXT | MGA_UPLOAD_CLIPRECTS;

    mmesa->sarea->dirty |= MGA_UPLOAD_CONTEXT;

   if (sarea->ctxOwner != me) {
      mmesa->dirty |= (MGA_UPLOAD_CONTEXT | MGA_UPLOAD_TEX0 |
		       MGA_UPLOAD_TEX1 | MGA_UPLOAD_PIPE);
      sarea->ctxOwner=me;
   }

   for ( i = 0 ; i < mmesa->nr_heaps ; i++ ) {
      DRI_AGE_TEXTURES( mmesa->texture_heaps[ i ] );
   }

   sarea->last_quiescent = -1;	/* just kill it for now */
}


static const struct __DriverAPIRec mgaAPI = {
   .InitDriver      = mgaInitDriver,
   .DestroyScreen   = mgaDestroyScreen,
   .CreateContext   = mgaCreateContext,
   .DestroyContext  = mgaDestroyContext,
   .CreateBuffer    = mgaCreateBuffer,
   .DestroyBuffer   = mgaDestroyBuffer,
   .SwapBuffers     = mgaSwapBuffers,
   .MakeCurrent     = mgaMakeCurrent,
   .UnbindContext   = mgaUnbindContext,
   .GetSwapInfo     = getSwapInfo,
   .GetMSC          = driGetMSC32,
   .WaitForMSC      = driWaitForMSC32,
   .WaitForSBC      = NULL,
   .SwapBuffersMSC  = NULL
};


/*
 * This is the bootstrap function for the driver.
 * The __driCreateScreen name is the symbol that libGL.so fetches.
 * Return:  pointer to a __DRIscreenPrivate.
 */
#if !defined(DRI_NEW_INTERFACE_ONLY)
void *__driCreateScreen(Display *dpy, int scrn, __DRIscreen *psc,
                        int numConfigs, __GLXvisualConfig *config)
{
   __DRIscreenPrivate *psp;
   psp = __driUtilCreateScreen(dpy, scrn, psc, numConfigs, config, &mgaAPI);
   return (void *) psp;
}
#endif /* !defined(DRI_NEW_INTERFACE_ONLY) */


/**
 * This is the bootstrap function for the driver.  libGL supplies all of the
 * requisite information about the system, and the driver initializes itself.
 * This routine also fills in the linked list pointed to by \c driver_modes
 * with the \c __GLcontextModes that the driver can support for windows or
 * pbuffers.
 * 
 * \return A pointer to a \c __DRIscreenPrivate on success, or \c NULL on 
 *         failure.
 */
#ifdef USE_NEW_INTERFACE
void * __driCreateNewScreen( __DRInativeDisplay *dpy, int scrn, __DRIscreen *psc,
			     const __GLcontextModes * modes,
			     const __DRIversion * ddx_version,
			     const __DRIversion * dri_version,
			     const __DRIversion * drm_version,
			     const __DRIframebuffer * frame_buffer,
			     drmAddress pSAREA, int fd, 
			     int internal_api_version,
			     __GLcontextModes ** driver_modes )
			     
{
   __DRIscreenPrivate *psp;
   static const __DRIversion ddx_expected = { 1, 0, 0 };
   static const __DRIversion dri_expected = { 4, 0, 0 };
   static const __DRIversion drm_expected = { 3, 0, 0 };

   if ( ! driCheckDriDdxDrmVersions2( "MGA",
				      dri_version, & dri_expected,
				      ddx_version, & ddx_expected,
				      drm_version, & drm_expected ) ) {
      return NULL;
   }

   psp = __driUtilCreateNewScreen(dpy, scrn, psc, NULL,
				  ddx_version, dri_version, drm_version,
				  frame_buffer, pSAREA, fd,
				  internal_api_version, &mgaAPI);
   if ( psp != NULL ) {
      create_context_modes = (PFNGLXCREATECONTEXTMODES)
	  glXGetProcAddress( (const GLubyte *) "__glXCreateContextModes" );
      if ( create_context_modes != NULL ) {
	 MGADRIPtr dri_priv = (MGADRIPtr) psp->pDevPriv;
	 *driver_modes = mgaFillInModes( dri_priv->cpp * 8,
					 (dri_priv->cpp == 2) ? 16 : 24,
					 (dri_priv->cpp == 2) ? 0  : 8,
					 (dri_priv->backOffset != dri_priv->depthOffset) );
      }
   }

   return (void *) psp;
}
#endif /* USE_NEW_INTERFACE */


/**
 * Get information about previous buffer swaps.
 */
static int
getSwapInfo( __DRIdrawablePrivate *dPriv, __DRIswapInfo * sInfo )
{
   mgaContextPtr  mmesa;

   if ( (dPriv == NULL) || (dPriv->driContextPriv == NULL)
	|| (dPriv->driContextPriv->driverPrivate == NULL)
	|| (sInfo == NULL) ) {
      return -1;
   }

   mmesa = (mgaContextPtr) dPriv->driContextPriv->driverPrivate;
   sInfo->swap_count = mmesa->swap_count;
   sInfo->swap_ust = mmesa->swap_ust;
   sInfo->swap_missed_count = mmesa->swap_missed_count;

   sInfo->swap_missed_usage = (sInfo->swap_missed_count != 0)
       ? driCalculateSwapUsage( dPriv, 0, mmesa->swap_missed_ust )
       : 0.0;

   return 0;
}
