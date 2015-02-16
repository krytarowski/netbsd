/**************************************************************************
 * 
 * Copyright 1998-1999 Precision Insight, Inc., Cedar Park, Texas.
 * All Rights Reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 * **************************************************************************/
/* $XFree86: xc/extras/Mesa/src/mesa/drivers/dri/i830/i830_screen.c,v 1.2 2004/12/13 22:40:51 tsi Exp $ */

/**
 * \file i830_screen.c
 * 
 * Adapted for use on the I830M by Jeff Hartmann.
 *
 * \author Keith Whitwell <keith@tungstengraphics.com>
 * \author Jeff Hartmann <jhartmann@2d3d.com>
 */


#include "glheader.h"
#include "context.h"
#include "matrix.h"
#include "simple_list.h"

#include "i830_screen.h"
#include "i830_dri.h"

#include "i830_state.h"
#include "i830_tex.h"
#include "i830_span.h"
#include "i830_tris.h"
#include "i830_ioctl.h"

#include "i830_dri.h"

#include "utils.h"
#include "xmlpool.h"

const char __driConfigOptions[] =
DRI_CONF_BEGIN
    DRI_CONF_SECTION_PERFORMANCE
       DRI_CONF_MAX_TEXTURE_UNITS(4,2,4)
    DRI_CONF_SECTION_END
DRI_CONF_END;
const GLuint __driNConfigOptions = 1;

#ifdef USE_NEW_INTERFACE
static PFNGLXCREATECONTEXTMODES create_context_modes = NULL;
#endif /*USE_NEW_INTERFACE*/

static int i830_malloc_proxy_buf(drmBufMapPtr buffers)
{
   char *buffer;
   drmBufPtr buf;
   int i;

   buffer = ALIGN_MALLOC(I830_DMA_BUF_SZ, 32);
   if(buffer == NULL) return -1;
   for(i = 0; i < I830_DMA_BUF_NR; i++) {
      buf = &(buffers->list[i]);
      buf->address = (drmAddress)buffer;
   }

   return 0;
}

static drmBufMapPtr i830_create_empty_buffers(void)
{
   drmBufMapPtr retval;

   retval = (drmBufMapPtr)ALIGN_MALLOC(sizeof(drmBufMap), 32);
   if(retval == NULL) return NULL;
   memset(retval, 0, sizeof(drmBufMap));
   retval->list = (drmBufPtr)ALIGN_MALLOC(sizeof(drmBuf) * I830_DMA_BUF_NR, 32);
   if(retval->list == NULL) {
      FREE(retval);
      return NULL;
   }

   memset(retval->list, 0, sizeof(drmBuf) * I830_DMA_BUF_NR);
   return retval;
}

static void i830PrintDRIInfo(i830ScreenPrivate *i830Screen,
			     __DRIscreenPrivate *sPriv,
			     I830DRIPtr gDRIPriv)
{
   GLuint size = (gDRIPriv->ringSize +
		  i830Screen->textureSize +
		  i830Screen->depth.size +
		  i830Screen->back.size +
		  sPriv->fbSize +
		  I830_DMA_BUF_NR * I830_DMA_BUF_SZ +
		  32768 /* Context Memory */ +
		  16*4096 /* Ring buffer */ +
		  64*1024 /* Scratch buffer */ +
		  4096 /* Cursor */);
   GLuint size_low = (gDRIPriv->ringSize +
		      i830Screen->textureSize +
		      sPriv->fbSize +
		      I830_DMA_BUF_NR * I830_DMA_BUF_SZ +
		      32768 /* Context Memory */ +
		      16*4096 /* Ring buffer */ +
		      64*1024 /* Scratch buffer */);

   fprintf(stderr, "\nFront size : 0x%x\n", sPriv->fbSize);
   fprintf(stderr, "Front offset : 0x%x\n", i830Screen->fbOffset);
   fprintf(stderr, "Back size : 0x%x\n", i830Screen->back.size);
   fprintf(stderr, "Back offset : 0x%x\n", i830Screen->backOffset);
   fprintf(stderr, "Depth size : 0x%x\n", i830Screen->depth.size);
   fprintf(stderr, "Depth offset : 0x%x\n", i830Screen->depthOffset);
   fprintf(stderr, "Texture size : 0x%x\n", i830Screen->textureSize);
   fprintf(stderr, "Texture offset : 0x%x\n", i830Screen->textureOffset);
   fprintf(stderr, "Ring offset : 0x%x\n", gDRIPriv->ringOffset);
   fprintf(stderr, "Ring size : 0x%x\n", gDRIPriv->ringSize);
   fprintf(stderr, "Memory : 0x%x\n", gDRIPriv->mem);
   fprintf(stderr, "Used Memory : low(0x%x) high(0x%x)\n", size_low, size);
}

static GLboolean i830InitDriver(__DRIscreenPrivate *sPriv)
{
   i830ScreenPrivate *i830Screen;
   I830DRIPtr         gDRIPriv = (I830DRIPtr)sPriv->pDevPriv;


   /* Allocate the private area */
   i830Screen = (i830ScreenPrivate *)CALLOC(sizeof(i830ScreenPrivate));
   if (!i830Screen) {
      fprintf(stderr,"\nERROR!  Allocating private area failed\n");
      return GL_FALSE;
   }

   /* parse information in __driConfigOptions */
   driParseOptionInfo (&i830Screen->optionCache,
		       __driConfigOptions, __driNConfigOptions);


   i830Screen->driScrnPriv = sPriv;
   sPriv->private = (void *)i830Screen;

   i830Screen->deviceID = gDRIPriv->deviceID;
   i830Screen->width = gDRIPriv->width;
   i830Screen->height = gDRIPriv->height;
   i830Screen->mem = gDRIPriv->mem;
   i830Screen->cpp = gDRIPriv->cpp;
   i830Screen->fbStride = gDRIPriv->fbStride;
   i830Screen->fbOffset = gDRIPriv->fbOffset;
			 
   switch (gDRIPriv->bitsPerPixel) {
   case 15: i830Screen->fbFormat = DV_PF_555; break;
   case 16: i830Screen->fbFormat = DV_PF_565; break;
   case 32: i830Screen->fbFormat = DV_PF_8888; break;
   }
			 
   i830Screen->backOffset = gDRIPriv->backOffset;
   i830Screen->depthOffset = gDRIPriv->depthOffset;
   i830Screen->backPitch = gDRIPriv->auxPitch;
   i830Screen->backPitchBits = gDRIPriv->auxPitchBits;
   i830Screen->textureOffset = gDRIPriv->textureOffset;
   i830Screen->textureSize = gDRIPriv->textureSize;
   i830Screen->logTextureGranularity = gDRIPriv->logTextureGranularity;
			 			    			 

   i830Screen->bufs = i830_create_empty_buffers();
   if(i830Screen->bufs == NULL) {
      fprintf(stderr,"\nERROR: Failed to create empty buffers in %s \n",
	      __FUNCTION__);
      FREE(i830Screen);
      return GL_FALSE;
   }

   /* Check if you need to create a fake buffer */
   if(i830_check_copy(sPriv->fd) == 1) {
      i830_malloc_proxy_buf(i830Screen->bufs);
      i830Screen->use_copy_buf = 1;
   } else {
      i830Screen->use_copy_buf = 0;
   }

   i830Screen->back.handle = gDRIPriv->backbuffer;
   i830Screen->back.size = gDRIPriv->backbufferSize;
			 
   if (drmMap(sPriv->fd,
	      i830Screen->back.handle,
	      i830Screen->back.size,
	      (drmAddress *)&i830Screen->back.map) != 0) {
      fprintf(stderr, "\nERROR: line %d, Function %s, File %s\n",
	      __LINE__, __FUNCTION__, __FILE__);
      FREE(i830Screen);
      sPriv->private = NULL;
      return GL_FALSE;
   }

   i830Screen->depth.handle = gDRIPriv->depthbuffer;
   i830Screen->depth.size = gDRIPriv->depthbufferSize;

   if (drmMap(sPriv->fd, 
	      i830Screen->depth.handle,
	      i830Screen->depth.size,
	      (drmAddress *)&i830Screen->depth.map) != 0) {
      fprintf(stderr, "\nERROR: line %d, Function %s, File %s\n", 
	      __LINE__, __FUNCTION__, __FILE__);
      FREE(i830Screen);
      drmUnmap(i830Screen->back.map, i830Screen->back.size);
      sPriv->private = NULL;
      return GL_FALSE;
   }

   i830Screen->tex.handle = gDRIPriv->textures;
   i830Screen->tex.size = gDRIPriv->textureSize;

   if (drmMap(sPriv->fd,
	      i830Screen->tex.handle,
	      i830Screen->tex.size,
	      (drmAddress *)&i830Screen->tex.map) != 0) {
      fprintf(stderr, "\nERROR: line %d, Function %s, File %s\n",
	      __LINE__, __FUNCTION__, __FILE__);
      FREE(i830Screen);
      drmUnmap(i830Screen->back.map, i830Screen->back.size);
      drmUnmap(i830Screen->depth.map, i830Screen->depth.size);
      sPriv->private = NULL;
      return GL_FALSE;
   }
			 
   i830Screen->sarea_priv_offset = gDRIPriv->sarea_priv_offset;
   
   if (0) i830PrintDRIInfo(i830Screen, sPriv, gDRIPriv);

   i830Screen->drmMinor = sPriv->drmMinor;

   if (sPriv->drmMinor >= 3) {
      int ret;
      drmI830GetParam gp;

      gp.param = I830_PARAM_IRQ_ACTIVE;
      gp.value = &i830Screen->irq_active;

      ret = drmCommandWriteRead( sPriv->fd, DRM_I830_GETPARAM,
				 &gp, sizeof(gp));
      if (ret) {
	 fprintf(stderr, "drmI830GetParam: %d\n", ret);
	 return GL_FALSE;
      }
   }

#if 0
   if (sPriv->drmMinor >= 3) {
      int ret;
      drmI830SetParam sp;

      sp.param = I830_SETPARAM_PERF_BOXES;
      sp.value = (getenv("I830_DO_BOXES") != 0);

      ret = drmCommandWrite( sPriv->fd, DRM_I830_SETPARAM,
			     &sp, sizeof(sp));
      if (ret) 
	 fprintf(stderr, "Couldn't set perfboxes: %d\n", ret);
   }
#endif

   if ( driCompareGLXAPIVersion( 20030813 ) >= 0 ) {
      PFNGLXSCRENABLEEXTENSIONPROC glx_enable_extension =
          (PFNGLXSCRENABLEEXTENSIONPROC) glXGetProcAddress( (const GLubyte *) "__glXScrEnableExtension" );
      void * const psc = sPriv->psc->screenConfigs;

      if ( glx_enable_extension != NULL ) {
	 (*glx_enable_extension)( psc, "GLX_SGI_make_current_read" );

	 if ( driCompareGLXAPIVersion( 20030915 ) >= 0 ) {
	    (*glx_enable_extension)( psc, "GLX_SGIX_fbconfig" );
	    (*glx_enable_extension)( psc, "GLX_OML_swap_method" );
	 }
      }
   }

   return GL_TRUE;
}
		
		
static void i830DestroyScreen(__DRIscreenPrivate *sPriv)
{
   i830ScreenPrivate *i830Screen = (i830ScreenPrivate *)sPriv->private;

   /* Need to unmap all the bufs and maps here:
    */
   drmUnmap(i830Screen->back.map, i830Screen->back.size);
   drmUnmap(i830Screen->depth.map, i830Screen->depth.size);
   drmUnmap(i830Screen->tex.map, i830Screen->tex.size);
   FREE(i830Screen);
   sPriv->private = NULL;
}

static GLboolean i830CreateBuffer(__DRIscreenPrivate *driScrnPriv,
				  __DRIdrawablePrivate *driDrawPriv,
				  const __GLcontextModes *mesaVis,
				  GLboolean isPixmap )
{
   if (isPixmap) {
      return GL_FALSE; /* not implemented */
   } else {
#if 0
      GLboolean swStencil = (mesaVis->stencilBits > 0 && 
			     mesaVis->depthBits != 24);
#else
      GLboolean swStencil = mesaVis->stencilBits > 0;
#endif
      driDrawPriv->driverPrivate = (void *) 
	 _mesa_create_framebuffer(mesaVis,
				  GL_FALSE,  /* software depth buffer? */
				  swStencil,
				  mesaVis->accumRedBits > 0,
				  GL_FALSE /* s/w alpha planes */);
      
      return (driDrawPriv->driverPrivate != NULL);
   }
}

static void i830DestroyBuffer(__DRIdrawablePrivate *driDrawPriv)
{
   _mesa_destroy_framebuffer((GLframebuffer *) (driDrawPriv->driverPrivate));
}

static const struct __DriverAPIRec i830API = {
   .InitDriver      = i830InitDriver,
   .DestroyScreen   = i830DestroyScreen,
   .CreateContext   = i830CreateContext,
   .DestroyContext  = i830DestroyContext,
   .CreateBuffer    = i830CreateBuffer,
   .DestroyBuffer   = i830DestroyBuffer,
   .SwapBuffers     = i830SwapBuffers,
   .MakeCurrent     = i830MakeCurrent,
   .UnbindContext   = i830UnbindContext,
   .GetSwapInfo     = NULL,
   .GetMSC          = NULL,
   .WaitForMSC      = NULL,
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
   psp = __driUtilCreateScreen(dpy, scrn, psc, numConfigs, config, &i830API);
   return (void *) psp;
}
#endif /* !defined(DRI_NEW_INTERFACE_ONLY) */


#ifdef USE_NEW_INTERFACE
static __GLcontextModes *
i830FillInModes( unsigned pixel_bits, unsigned depth_bits,
		 unsigned stencil_bits, GLboolean have_back_buffer )
{
   __GLcontextModes * modes;
   __GLcontextModes * m;
   unsigned num_modes;
   unsigned depth_buffer_factor;
   unsigned back_buffer_factor;
   GLenum fb_format;
   GLenum fb_type;

   /* GLX_SWAP_COPY_OML is only supported because the MGA driver doesn't
    * support pageflipping at all.
    */
   static const GLenum back_buffer_modes[] = {
      GLX_NONE, GLX_SWAP_UNDEFINED_OML, GLX_SWAP_COPY_OML
   };

   u_int8_t depth_bits_array[2];
   u_int8_t stencil_bits_array[2];


   depth_bits_array[0] = 0;
   depth_bits_array[1] = depth_bits;

   /* Just like with the accumulation buffer, always provide some modes
    * with a stencil buffer.  It will be a sw fallback, but some apps won't
    * care about that.
    */
   stencil_bits_array[0] = 0;
   stencil_bits_array[1] = (stencil_bits == 0) ? 8 : stencil_bits;

   depth_buffer_factor = ((depth_bits != 0) || (stencil_bits != 0)) ? 2 : 1;
   back_buffer_factor  = (have_back_buffer) ? 3 : 1;

   num_modes = depth_buffer_factor * back_buffer_factor * 4;

    if ( pixel_bits == 16 ) {
        fb_format = GL_RGB;
        fb_type = GL_UNSIGNED_SHORT_5_6_5;
    }
    else {
        fb_format = GL_BGRA;
        fb_type = GL_UNSIGNED_INT_8_8_8_8_REV;
    }

   modes = (*create_context_modes)( num_modes, sizeof( __GLcontextModes ) );
   m = modes;
   if ( ! driFillInModes( & m, fb_format, fb_type,
			  depth_bits_array, stencil_bits_array, depth_buffer_factor,
			  back_buffer_modes, back_buffer_factor,
			  GLX_TRUE_COLOR ) ) {
	fprintf( stderr, "[%s:%u] Error creating FBConfig!\n",
		 __func__, __LINE__ );
	return NULL;
    }

   /* There's no direct color modes on i830? */

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
   static const __DRIversion drm_expected = { 1, 3, 0 };

   if ( ! driCheckDriDdxDrmVersions2( "i830",
				      dri_version, & dri_expected,
				      ddx_version, & ddx_expected,
				      drm_version, & drm_expected ) ) {
      return NULL;
   }

   psp = __driUtilCreateNewScreen(dpy, scrn, psc, NULL,
				  ddx_version, dri_version, drm_version,
				  frame_buffer, pSAREA, fd,
				  internal_api_version, &i830API);
   if ( psp != NULL ) {
      create_context_modes = (PFNGLXCREATECONTEXTMODES)
	  glXGetProcAddress( (const GLubyte *) "__glXCreateContextModes" );
      if ( create_context_modes != NULL ) {
	 I830DRIPtr dri_priv = (I830DRIPtr) psp->pDevPriv;
	 *driver_modes = i830FillInModes( dri_priv->cpp * 8,
					  (dri_priv->cpp == 2) ? 16 : 24,
					  (dri_priv->cpp == 2) ? 0  : 8,
					  (dri_priv->backOffset != dri_priv->depthOffset) );
      }
   }

   return (void *) psp;
}
#endif /* USE_NEW_INTERFACE */
