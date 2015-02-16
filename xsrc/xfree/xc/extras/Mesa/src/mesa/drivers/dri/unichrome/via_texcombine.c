/*
 * (C) Copyright IBM Corporation 2004
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
 * IBM AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * \file via_texcombine.c
 * Calculate texture combine hardware state.
 *
 * \author Ian Romanick <idr@us.ibm.com>
 */

#include <stdio.h>

#include "glheader.h"
#include "context.h"
#include "macros.h"
#include "colormac.h"
#include "enums.h"
#include "dd.h"

#include "mm.h"
#include "via_context.h"
#include "via_state.h"
#include "via_tex.h"
#include "via_vb.h"
#include "via_tris.h"
#include "via_ioctl.h"

#include "swrast/swrast.h"
#include "array_cache/acache.h"
#include "tnl/tnl.h"
#include "swrast_setup/swrast_setup.h"

#include "tnl/t_pipeline.h"

#define VIA_USE_ALPHA (HC_XTC_Adif - HC_XTC_Dif)

#define INPUT_A_SHIFT     14
#define INPUT_B_SHIFT     7
#define INPUT_C_SHIFT     0
#define INPUT_BiasC_SHIFT 14
#define INPUT_BiasA_SHIFT 3

#define CONST_ONE         (HC_XTC_0 | HC_XTC_InvTOPC)

static const unsigned color_operand_modifier[4] = {
   0,
   HC_XTC_InvTOPC,
   VIA_USE_ALPHA,
   VIA_USE_ALPHA | HC_XTC_InvTOPC,
};

static const unsigned alpha_operand_modifier[2] = {
   0, HC_XTA_InvTOPA
};

static const unsigned c_shift_table[3] = {
   HC_HTXnTBLCshift_No, HC_HTXnTBLCshift_1, HC_HTXnTBLCshift_2
};

static const unsigned  a_shift_table[3] = {
   HC_HTXnTBLAshift_No, HC_HTXnTBLAshift_1, HC_HTXnTBLAshift_2
};


/**
 * Calculate the hardware state for the specified texture combine mode
 *
 * \bug
 * For the alpha combine, \c GL_CONSTANT is still probably wrong.
 *
 * \bug
 * All forms of DOT3 bumpmapping are completely untested, and are most
 * likely wrong.
 *
 * \bug
 * This code still fails progs/demos/texenv for all modes with \c GL_ALPHA
 * textures.  This was also the case with the code that Via supplied.  It
 * also fails for \c GL_REPLACE with \c GL_RGBA textures.  Everything else
 * that texenv tests looks good.
 */
void
viaTexCombineState( viaContextPtr vmesa,
		    const struct gl_tex_env_combine_state * combine,
		    unsigned unit )
{
   unsigned color_arg[3];
   unsigned alpha_arg[3];
   unsigned color = 0;
   unsigned alpha = 0;
   unsigned bias = 0;
   unsigned op = 0;
   unsigned a_shift = combine->ScaleShiftA;
   unsigned c_shift = combine->ScaleShiftRGB;
   unsigned i;
   unsigned constant_color[3];
   unsigned ordered_constant_color[4];
   unsigned constant_alpha = 0;
   unsigned bias_alpha = 0;
   const struct gl_texture_unit const * texUnit = & vmesa->glCtx->Texture.Unit[unit];
   unsigned env_color[4];


   CLAMPED_FLOAT_TO_UBYTE(env_color[0], texUnit->EnvColor[0]);
   CLAMPED_FLOAT_TO_UBYTE(env_color[1], texUnit->EnvColor[1]);
   CLAMPED_FLOAT_TO_UBYTE(env_color[2], texUnit->EnvColor[2]);
   CLAMPED_FLOAT_TO_UBYTE(env_color[3], texUnit->EnvColor[3]);

   (void) memset( constant_color, 0, sizeof( constant_color ) );
   (void) memset( ordered_constant_color, 0, sizeof( ordered_constant_color ) );

   for ( i = 0 ; i < combine->_NumArgsRGB ; i++ ) {
      const GLint op = combine->OperandRGB[i] - GL_SRC_COLOR;

      switch ( combine->SourceRGB[i] ) {
      case GL_TEXTURE:
	 color_arg[i] = HC_XTC_Tex;
	 color_arg[i] += color_operand_modifier[op];
	 break;
      case GL_CONSTANT:
	 color_arg[i] = HC_XTC_HTXnTBLRC;

	 switch( op ) {
	 case 0:
	    constant_color[i] = ((env_color[0] << 16) 
				 | (env_color[1] << 8) 
				 | env_color[2]);
	    break;
	 case 1:
	    constant_color[i] = ~((env_color[0] << 16) 
				  | (env_color[1] << 8) 
				  | env_color[2]) & 0x00ffffff;
	    break;
	 case 2:
	    constant_color[i] = ((env_color[3] << 16)
				 | (env_color[3] << 8)
				 | env_color[3]);
	    break;
	 case 3:
	    constant_color[i] = ~((env_color[3] << 16) 
				  | (env_color[3] << 8) 
				  | env_color[3]) & 0x00ffffff;
	    break;
	 }
	 break;
      case GL_PRIMARY_COLOR:
	 color_arg[i] = HC_XTC_Dif;
	 color_arg[i] += color_operand_modifier[op];
	 break;
      case GL_PREVIOUS:
	 color_arg[i] = (unit == 0) ? HC_XTC_Dif : HC_XTC_Cur;
	 color_arg[i] += color_operand_modifier[op];
	 break;
      }
   }
	
   for ( i = 0 ; i < combine->_NumArgsA ; i++ ) {
      const GLint op = combine->OperandA[i] - GL_SRC_ALPHA;

      switch ( combine->SourceA[i] ) {
      case GL_TEXTURE:
	 alpha_arg[i] = HC_XTA_Atex;
	 alpha_arg[i] += alpha_operand_modifier[op];
	 break;
      case GL_CONSTANT:
	 alpha_arg[i] = HC_XTA_HTXnTBLRA;
	 constant_alpha = (op == 0) 
	   ? env_color[3] : ~(env_color[3]) & 0x000000ff;
	 break;
      case GL_PRIMARY_COLOR:
	 alpha_arg[i] = HC_XTA_Adif;
	 alpha_arg[i] += alpha_operand_modifier[op];
	 break;
      case GL_PREVIOUS:
	 alpha_arg[i] = (unit == 0) ? HC_XTA_Adif : HC_XTA_Acur;
	 alpha_arg[i] += alpha_operand_modifier[op];
	 break;
      }
   }

   
   /* On the Unichrome, all combine operations take on some form of:
    *
    *     A * (B op Bias) + C
    * 
    * 'op' can be selected as add, subtract, min, max, or mask.  The min, max
    * and mask modes are currently unused.  With the exception of DOT3, all
    * standard GL_COMBINE modes can be implemented simply by selecting the
    * correct inputs for A, B, C, and Bias and the correct operation for op.
    */

   color = HC_HTXnTBLCsat_MASK;
   alpha = HC_HTXnTBLAsat_MASK;

   switch( combine->ModeRGB ) {
   /* A = 0, B = 0, C = arg0, Bias = 0
    */
   case GL_REPLACE:
      bias |= (color_arg[0] << INPUT_BiasC_SHIFT);
      ordered_constant_color[3] = constant_color[0];
      break;
      
   /* A = arg[0], B = arg[1], C = 0, Bias = 0
    */
   case GL_MODULATE:
      color |= (color_arg[0] << INPUT_A_SHIFT)
	| (color_arg[1] << INPUT_B_SHIFT);

      ordered_constant_color[0] = constant_color[0];
      ordered_constant_color[1] = constant_color[1];
      break;

   /* A = 1.0, B = arg[0], C = 0, Bias = arg[1]
    */
   case GL_ADD:
   case GL_SUBTRACT:
      if ( combine->ModeRGB == GL_SUBTRACT ) {
	 op |= HC_HTXnTBLCop_Sub;
      }

      color |= (color_arg[0] << INPUT_B_SHIFT)
	| (CONST_ONE << INPUT_A_SHIFT);
      
      bias |= (color_arg[1] << INPUT_BiasC_SHIFT);
      ordered_constant_color[1] = constant_color[0];
      ordered_constant_color[3] = constant_color[1];
      break;

   /* A = 0, B = arg[0], C = arg[1], Bias = 0.5
    */
   case GL_ADD_SIGNED:
      color |= (color_arg[0] << INPUT_B_SHIFT)
	| (color_arg[1] << INPUT_C_SHIFT);
      bias |= HC_HTXnTBLCbias_HTXnTBLRC;
      op |= HC_HTXnTBLCop_Sub;

      ordered_constant_color[1] = constant_color[0];
      ordered_constant_color[2] = constant_color[1];
      ordered_constant_color[3] = 0x00808080;
      break;

   /* A = arg[2], B = arg[0], C = arg[1], Bias = arg[1]
    */
   case GL_INTERPOLATE:
      op |= HC_HTXnTBLCop_Sub;

      color |= (color_arg[2] << INPUT_A_SHIFT) |
	(color_arg[0] << INPUT_B_SHIFT) |
	(color_arg[1] << INPUT_C_SHIFT);
      bias |= (color_arg[1] << INPUT_BiasC_SHIFT);

      ordered_constant_color[0] = constant_color[2];
      ordered_constant_color[1] = constant_color[0];
      ordered_constant_color[2] = constant_color[1];
      ordered_constant_color[3] = constant_color[1];
      break;

   /* At this point this code is completely untested.  It appears that the
    * Unichrome has the same limitation as the Radeon R100.  The only
    * supported post-scale when doing DOT3 bumpmapping is 1x.
    */
   case GL_DOT3_RGB_EXT:
   case GL_DOT3_RGBA_EXT:
   case GL_DOT3_RGB:
   case GL_DOT3_RGBA:
      c_shift = 2;
      a_shift = 2;
      color |= (color_arg[0] << INPUT_A_SHIFT) |
	(color_arg[1] << INPUT_B_SHIFT);
      op |= HC_HTXnTBLDOT4;
      break;
   }


   /* The alpha blend stage has the annoying quirk of not having a
    * hard-wired 0 input, like the color stage.  As a result, we have
    * to program the constant register with 0 and use that as our
    * 0 input.
    */

   switch( combine->ModeA ) {
   /* A = 0, B = 0, C = 0, Bias = arg0
    */
   case GL_REPLACE:
      bias |= (alpha_arg[0] << INPUT_BiasA_SHIFT);

      alpha |= (HC_XTA_HTXnTBLRA << INPUT_A_SHIFT) |
	(HC_XTA_HTXnTBLRA << INPUT_B_SHIFT) |
	(HC_XTA_HTXnTBLRA << INPUT_C_SHIFT);
      break;
      
   /* A = arg[0], B = arg[1], C = 0, Bias = 0
    */
   case GL_MODULATE:
      alpha |= (alpha_arg[1] << INPUT_A_SHIFT)
	| (alpha_arg[0] << INPUT_B_SHIFT)
        | (HC_XTA_HTXnTBLRA << INPUT_C_SHIFT);

      bias |= (HC_XTA_HTXnTBLRA << INPUT_BiasA_SHIFT);
      break;

   /* A = 0, B = arg[0], C = 0, Bias = arg[1]
    */
   case GL_ADD:
   case GL_SUBTRACT:
      if ( combine->ModeA == GL_SUBTRACT ) {
	 op |= HC_HTXnTBLAop_Sub;
      }

      alpha |= (HC_XTA_HTXnTBLRA << INPUT_A_SHIFT) |
	(alpha_arg[0] << INPUT_B_SHIFT) |
	(HC_XTA_HTXnTBLRA << INPUT_C_SHIFT);
      bias |= (alpha_arg[1] << INPUT_BiasA_SHIFT);
      break;

   /* A = 0, B = arg[0], C = arg[1], Bias = 0.5
    */
   case GL_ADD_SIGNED:
      op |= HC_HTXnTBLAop_Sub;

      alpha |= (alpha_arg[0] << INPUT_B_SHIFT)
	| (alpha_arg[1] << INPUT_C_SHIFT);
      bias |= (HC_XTA_HTXnTBLRA << INPUT_BiasA_SHIFT);

      bias_alpha = 0x00000080;
      break;

   /* A = arg[2], B = arg[0], C = arg[1], Bias = arg[1]
    */
   case GL_INTERPOLATE:
      op |= HC_HTXnTBLAop_Sub;

      alpha |= (alpha_arg[2] << INPUT_A_SHIFT) |
	(alpha_arg[0] << INPUT_B_SHIFT) |
	(alpha_arg[1] << INPUT_C_SHIFT);
      bias |= (alpha_arg[1] << INPUT_BiasA_SHIFT);
      break;
   }
   

   op |= c_shift_table[ c_shift ] | a_shift_table[ a_shift ];


   if ( unit == 0 ) {
      vmesa->regHTXnTBLMPfog_0 = HC_HTXnTBLMPfog_Fog;

      vmesa->regHTXnTBLCsat_0 = color;
      vmesa->regHTXnTBLAsat_0 = alpha;
      vmesa->regHTXnTBLCop_0 = op | bias;
      vmesa->regHTXnTBLRAa_0 = bias_alpha | (constant_alpha << 16);

      vmesa->regHTXnTBLRCa_0 = ordered_constant_color[0];
      vmesa->regHTXnTBLRCb_0 = ordered_constant_color[1];
      vmesa->regHTXnTBLRCc_0 = ordered_constant_color[2];
      vmesa->regHTXnTBLRCbias_0 = ordered_constant_color[3];
   }
   else {
      vmesa->regHTXnTBLMPfog_1 = HC_HTXnTBLMPfog_Fog;

      vmesa->regHTXnTBLCsat_1 = color;
      vmesa->regHTXnTBLAsat_1 = alpha;
      vmesa->regHTXnTBLCop_1 = op | bias;
      vmesa->regHTXnTBLRAa_1 = bias_alpha | (constant_alpha << 16);
   }
}

