

/*
 * File:        hidden.c
 *
 * The top of the hidden line renderer
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
#include "hidden.h"

int	total_edges = 0;
int	drawn_edges = 0;

/* from libpaint: */
extern void     paint_tri(Object_t *op, Tri_t *tri, int usecfb);

/*
 * draw the entire scene.
 *
 */
void
draw_scene(void)
{
    rgba_t	white = {255, 255, 255, 0};
    Object_t	*op;
    Tri_t	*tri;
    float       progress = 0.0;
    int         i, j, retval = CLIP_TRIVIAL_ACCEPT, paintshade = FALSE;

    fprintf(stderr,"Drawing Scene:\n");
    fprintf(stderr,"\tResolution %d x %d\n",RPScene.xres,RPScene.yres);
    fprintf(stderr,"\t[%d] objects...\n",RPScene.obj_count);
    fprintf(stderr,"\t[%d] lights...\n",RPScene.light_count);

	/* init frame buffer and z-buffer: */
    RPClearColorFB(&white);
    RPClearDepthFB(NULL);
    RPLoadBackgroundImage();
    RPSetSceneFlags(FLAG_PERSP_TEXTURE); /* tell pipeline to persp correct tex coords */
    RPProcessObjects(TRUE); 		/* tranform objects to camera space */
					/* TRUE flag also does projection */

	/* use the generic flags to decide if we want to paint the objects into
         * the color frame buffer (as well as the zbuffer) during pre-processing.
         * this permits an effective "toon shade" effect to be relalized.
	 */

    if (Flagged(RPScene.generic_flags, FLAG_RENDER_03)) {
	paintshade = TRUE;
    }

    fprintf(stderr,"Progress:  %5.2f %%",progress*33.0);

	/* fill the z-buffer first */
    for (i=0; i<RPScene.obj_count; i++) {

	op = RPScene.obj_list[i];

        if (op->type == OBJ_TYPE_SPHERE) {

		/* can't handle these yet */

	} else if (op->type == OBJ_TYPE_POLY) {

	    for (j=0; j<op->tri_count; j++) {
                tri = &(op->tris[j]);

		/* we use this user-defined scene flag to turn off clipping: */
	        if (!Flagged(RPScene.generic_flags, FLAG_RENDER_01)) 
        	    retval = RPClipTriangle(op, tri);

		if (retval > CLIP_TRIVIAL_REJECT)
		    paint_tri(op, tri, paintshade);
            }

    	} else {     /* can't happen */
            fprintf(stderr,"ERROR : %s : %d : unknown object type %d\n",
                        __FILE__,__LINE__,op->type);
        }

        progress = (float)i/(float)RPScene.obj_count;
        fprintf(stderr,"\b\b\b\b\b\b\b%5.2f %%",progress*33.0);
    }

	/* process edges */
    for (i=0; i<RPScene.obj_count; i++) {

	op = RPScene.obj_list[i];

        if (op->type == OBJ_TYPE_SPHERE) {

		/* can't handle these yet */

	} else if (op->type == OBJ_TYPE_POLY) {

	    create_obj_edges(op);
	    process_obj_edges(op);
        }

        progress = (float)i/(float)RPScene.obj_count;
        fprintf(stderr,"\b\b\b\b\b\b\b%5.2f %%",progress*33.0+33.0);
    }

	/* paint edges */
    for (i=0; i<RPScene.obj_count; i++) {

	op = RPScene.obj_list[i];

        if (op->type == OBJ_TYPE_SPHERE) {

		/* can't handle these yet */

	} else if (op->type == OBJ_TYPE_POLY) {

	    draw_edges(op);
	}

        progress = (float)i/(float)RPScene.obj_count;
        fprintf(stderr,"\b\b\b\b\b\b\b%5.2f %%",(progress*33.0)+66.0);
    }

    fprintf(stderr,"\b\b\b\b\b\b\b\b100 %% ... done!\n");

    fprintf(stderr,"\n%s : Rendering Summary:\n",program_name);
    fprintf(stderr,"------------------------------------------------------\n");
    fprintf(stderr,"%s : [%'16d] input polygons\n",
            program_name, RPScene.input_polys);
    fprintf(stderr,"%s : [%'16d] trivially rejected polygons\n",
            program_name, RPScene.trivial_rejected_polys);
    fprintf(stderr,"%s : [%'16d] clipped polygons\n",
            program_name, RPScene.clipped_polys);
    fprintf(stderr,"%s : [%'16d] total edges\n",
            program_name, total_edges);
    fprintf(stderr,"%s : [%'16d] drawn edges\n",
            program_name, drawn_edges);
    fprintf(stderr,"\n");
}

