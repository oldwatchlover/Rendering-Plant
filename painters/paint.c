

/*
 * File:        paint.c
 *
 * The top of the painter's algorithm hardware algorithm simulator rasterizer
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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "rp.h"
#include "paint.h"

int     culled_polys = 0;
int     drawn_polys = 0;

/*
 * paint the entire scene.
 *
 */
void
paint_scene(void)
{
    Object_t	*op;
    Tri_t	*tri;
    float       progress = 0.0;
    int         i, j, retval = CLIP_TRIVIAL_ACCEPT;

    fprintf(stderr,"Painting Scene:\n");
    fprintf(stderr,"\tResolution %d x %d\n",RPScene.xres,RPScene.yres);
    fprintf(stderr,"\t[%d] objects...\n",RPScene.obj_count);
    fprintf(stderr,"\t[%d] lights...\n",RPScene.light_count);

	/* init frame buffer and z-buffer: */
    RPClearColorFB(NULL);
    RPClearDepthFB(NULL);
    RPLoadBackgroundImage();
    RPSetSceneFlags(FLAG_PERSP_TEXTURE); /* tell pipeline to persp correct tex coords */
    RPProcessObjects(TRUE); 		/* tranform objects to camera space */

    if (Flagged(RPScene.flags, FLAG_FOG)) {
	rgba_t	temp;
	temp.r = (u8) Clamp0255(RPScene.fog_color.r * MAX_COLOR_VAL);
	temp.g = (u8) Clamp0255(RPScene.fog_color.g * MAX_COLOR_VAL);
	temp.b = (u8) Clamp0255(RPScene.fog_color.b * MAX_COLOR_VAL);
	temp.a = (u8) Clamp0255(RPScene.fog_color.a * MAX_COLOR_VAL);
	RPClearColorFB(&temp);
    }

    fprintf(stderr,"Progress:  %5.2f %%",progress*100.0);

    for (i=0; i<RPScene.obj_count; i++) {

	op = RPScene.obj_list[i];

        if (op->type == OBJ_TYPE_SPHERE) {

		/* can't handle these yet */

	} else if (op->type == OBJ_TYPE_POLY) {

	    for (j=0; j<op->tri_count; j++) {
                tri = &(op->tris[j]);

		/* we use this user-defined scene flat to turn off clipping: */
	        if (!Flagged(RPScene.generic_flags, FLAG_RENDER_01)) 
        	    retval = RPClipTriangle(op, tri);

		if (retval > CLIP_TRIVIAL_REJECT)
		    paint_tri(op, tri);
            }


    	} else {     /* can't happen */
            fprintf(stderr,"ERROR : %s : %d : unknown object type %d\n",
                        __FILE__,__LINE__,op->type);
        }

        progress = (float)i/(float)RPScene.obj_count;
        fprintf(stderr,"\b\b\b\b\b\b\b%5.2f %%",progress*100.0);
    }

    fprintf(stderr,"\b\b\b\b\b\b\b\b100 %% ... done!\n");

    fprintf(stderr,"\n%s : Rendering Summary:\n",program_name);
    fprintf(stderr,"------------------------------------------------------\n");
    fprintf(stderr,"%s : [%'16d] input polygons\n",
            program_name, RPScene.input_polys);
    fprintf(stderr,"%s : [%'16d] culled polygons\n",
            program_name, culled_polys);
    fprintf(stderr,"%s : [%'16d] tiny rejected polygons\n",
            program_name, RPScene.tiny_rejected_polys);
    fprintf(stderr,"%s : [%'16d] trivially rejected polygons\n",
            program_name, RPScene.trivial_rejected_polys);
    fprintf(stderr,"%s : [%'16d] clipped polygons\n",
            program_name, RPScene.clipped_polys);
    fprintf(stderr,"%s : [%'16d] drawn polygons\n",
            program_name, drawn_polys);
    fprintf(stderr,"\n");
}

