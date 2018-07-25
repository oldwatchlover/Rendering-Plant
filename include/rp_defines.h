
/*
 * File:	rp_defines.h
 *
 * This include file holds all of the constants and macros used.
 *
 */

/*
 * 
 * MIT License
 * 
 * Copyright (c) 2018 Steve Anderson
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#ifndef __RP_DEFINES_H__
#define __RP_DEFINES_H__

/* program housekeeping: */
#define RP_VERSION 		"1.0"
#define DEFAULT_OUTPUT_FILE	"output.bmp"

/* some constants: */
#define REALLY_BIG_FLOAT        (2147483647.0f)
#define TRUE    		(1)
#define FALSE   		(0)

/* program constants: */
#define MAX_FILENAME_LENGTH     (256)
#define MAX_VERTS       	(32000)
#define MAX_TRIS        	(32000)
#define MAX_OBJS        	(1024)
#define MAX_SPHERES     	(1024)
#define MAX_TEXTURES    	(32)
#define MAX_OBJ_MATERIALS    	(32)
#define MAX_LIGHTS      	(32)
#define MAX_XRES        	(1920*2)
#define MAX_YRES        	(1080*2)
#define MAX_COLOR_VAL   	(255)

/* useful math: */
#define Pi                      ((float)(3.14159265f))
#define DegToRad                (Pi/(float)180.0f)
#define RadToDeg                ((float)180.0f/Pi)
#define Epsilon                 ((float)(0.0001f))
#define EpEpsilon               ((float)(0.000001f))
#define Min(a, b)               ((a) < (b) ? (a) : (b))
#define Min3(a, b, c)           ((a) < (b) ? Min(a, c) : Min(b, c))
#define Max(a, b)               ((a) > (b) ? (a) : (b))
#define Max3(a, b, c)           ((a) > (b) ? Max(a, c) : Max(b, c))
#define NearlyZero(x, E)        (fabs(x) < E)
#define NearlyEqual(x, y, E)    (fabs((x)-(y)) < E)
#define NearlyLess(x, y, E)     (((x) - E) < (y))
#define NearlyMore(x, y, E)     (((x) + E) > (y))
#define Clamp0x(c,x)            (((c) < 0.0) ? (0.0) : (((c) > (x)) ? (x) : (c)))
#define Clamp0255(c)            Clamp0x(c,255)
#define Floor(x)                ((int)(x))
#define Ceil(x)                 (((int)(x) == x) ? x : ((int)(x) + 1))
#define Abs(a)                  ((a) < 0.0 ? -(a) : (a))
#define Sqr(x)                  ((x)*(x))
#define Cube(x)                 ((x)*(x)*(x))

/* other useful macros: */
#define Flag(a, b)      	(a = ((a) | (b)))
#define UnFlag(a, b)    	(a = ((a) & ~(b)))
#define Flagged(a,b)    	(((a) & (b)) != 0)
#define UnFlagged(a, b) 	(((a) & (b)) == 0)

/* clear all flags (any 32-bit flags): */
#define FLAG_ALL		0xffffffff

/* matrix flags: */
#define MTX_FLAG_LOAD           0x00000000  /* default, for completeness */
#define MTX_FLAG_MULT           0x00000001
#define MTX_FLAG_PUSH           0x00000002
#define MTX_FLAG_POP            0x00000004
#define MTX_TYPE_MODEL          0x00001000
#define MTX_TYPE_VIEW           0x00002000
#define MTX_TYPE_PROJ           0x00004000

/* scene flags: */
#define FLAG_VERBOSE		0x00000001
#define FLAG_VERBOSE2		0x00000002
#define FLAG_NOSHADOW   	0x00000004
#define FLAG_FOG        	0x00000008
#define FLAG_ZBUFFER		0x00000010
#define FLAG_BACKGROUND_IMAGE	0x00000020
#define FLAG_SCENE_MULTISAMPLE	0x00000040
#define FLAG_PERSP_TEXTURE	0x00000080
 
/* triangle flags (clipping): */
#define FLAG_TRI_CLIPPED        0x0010
#define FLAG_TRI_CLIP_GEN       0x0020
#define CLIP_TRIVIAL_REJECT     (0)
#define CLIP_TRIVIAL_ACCEPT     (1)

/* per-object flags: */
#define FLAG_CULL_BACK  	0x00000001	/* also used for triangle flags */
#define FLAG_CULL_FRONT 	0x00000002	/* also used for triangle flags */
#define FLAG_TEXTURE    	0x00000004
#define FLAG_LIGHTING   	0x00000008
#define FLAG_FLATSHADE  	0x00000010	/* SMOOTHSHADE is !FLATSHADE */
#define FLAG_REFLECT    	0x00000020
#define FLAG_BUMP       	0x00000040
#define FLAG_RANDSHADE  	0x00000080
#define FLAG_POLYSHADE  	0x00000100
#define FLAG_VERTSHADE  	0x00000200
#define FLAG_VERTNORM   	0x00000400
#define FLAG_TEXGEN_SPHERE   	0x00000800
#define FLAG_TEXGEN_CYLINDER   	0x00001000

/* for materials: */
#define MATERIAL_COLOR		(0)
#define MATERIAL_AMBIENT	(1)
#define MATERIAL_DIFFUSE	(2)
#define MATERIAL_SPECULAR	(3)
#define MATERIAL_CHANNELS	(4)

/* texture flags: */
#define FLAG_TXT_CLAMP  	0x00000001
#define FLAG_TXT_WRAP   	0x00000002
#define FLAG_TXT_MIRROR 	0x00000004
#define FLAG_TXT_FILT   	0x00000008
#define FLAG_TXT_MOD    	0x00000010
#define FLAG_TXT_MIPMAP 	0x00000020    /* not implemented */
#define FLAG_TXT_MODULATE       FLAG_TXT_MOD

/* object flags: */
#define OBJ_TYPE_SPHERE 	0x00000001
#define OBJ_TYPE_POLY   	0x00000002

/* generic, renderer-definable flags... stored in Scene.generic_state", 
 * set/cleared in the input stream by scene command: genericflags();
 * a renderer can use them and assign their own meaning/purpose.
 */
#define FLAG_RENDER_01		0x00000001
#define FLAG_RENDER_02		0x00000002
#define FLAG_RENDER_03		0x00000004
#define FLAG_RENDER_04		0x00000008
#define FLAG_RENDER_05		0x00000010
#define FLAG_RENDER_06		0x00000020
#define FLAG_RENDER_07		0x00000040
#define FLAG_RENDER_08		0x00000080
/* can define up to 32... */

/* light types: */
#define POINT_LIGHT		0x00000001
#define SPOT_LIGHT		0x00000002

#endif
/* __RP_DEFINES_H__ */


