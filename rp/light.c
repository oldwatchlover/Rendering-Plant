
/*
 * File:	light.c
 *
 * this file holds the data structures and functions to manage the lights
 *
 * Only local lights implemented. No point, spot, area, attenuation, etc.
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

#include "rp.h"


/* set light (called from parser) */
void
RPSetLight(xyz_t pos, Colorf_t color)
{
    Light_t	*lp;

    if (RPScene.light_list[RPScene.light_count] == (Light_t *) NULL) {
	lp = (Light_t *) calloc(1, sizeof(Light_t));
    } else {
	lp = RPScene.light_list[RPScene.light_count];
    }

    lp->type = POINT_LIGHT;
    lp->pos.x = pos.x;   /* turn position into vector */
    lp->pos.y = pos.y;
    lp->pos.z = pos.z;
    lp->color.r = color.r; 
    lp->color.g = color.g;
    lp->color.b = color.b;
    lp->color.a = 1.0;
    lp->value = 1.0;

    if (Flagged(RPScene.flags, FLAG_VERBOSE)) {
	fprintf(stderr," +Added Light %d at (%8.3f,%8.3f,%8.3f) color (%4.2f,%4.2f,%4.2f)\n",
		RPScene.light_count, pos.x, pos.y, pos.z, color.r, color.g, color.b);
    }

    RPScene.light_list[RPScene.light_count++] = lp;
}

/* set spot light (called from parser) */
void
RPSetSpotLight(xyz_t pos, xyz_t coi, float fov, float focus, float range, float value, Colorf_t color)
{
    Light_t	*lp;

    if (RPScene.light_list[RPScene.light_count] == (Light_t *) NULL) {
	lp = (Light_t *) calloc(1, sizeof(Light_t));
    } else {
	lp = RPScene.light_list[RPScene.light_count];
    }

    lp->type = SPOT_LIGHT;
    lp->pos.x = pos.x;   /* turn position into vector */
    lp->pos.y = pos.y;
    lp->pos.z = pos.z;

    lp->coi.x = coi.x;  
    lp->coi.y = coi.y;
    lp->coi.z = coi.z;

    lp->color.r = color.r; 
    lp->color.g = color.g;
    lp->color.b = color.b;
    lp->color.a = 1.0;

    lp->fov = fov;
    lp->focus = focus;
    lp->range = range;
    lp->value = value;

    if (Flagged(RPScene.flags, FLAG_VERBOSE)) {
	fprintf(stderr," +Added Spot Light %d at (%8.3f,%8.3f,%8.3f) ...\n",
		RPScene.light_count, lp->pos.x, lp->pos.y, lp->pos.z);
	fprintf(stderr,"\tcoi = (%8.3f, %8.3f, %8.3f)\n",
		lp->coi.x, lp->coi.y, lp->coi.z);
	fprintf(stderr,"\tfov = %f, focus = %f, range = %f, value = %f\n",
		lp->fov, lp->focus, lp->range, lp->value);
	fprintf(stderr,"\tcolor = %4.2f, %4.2f, %4.2f, %4.2f\n",
		lp->color.r, lp->color.g, lp->color.b, lp->color.a);
    }

    RPScene.light_list[RPScene.light_count++] = lp;
}

/* transform the lights by the V matrix before rendering */
void
RPTransformLights(void)
{
    Light_t	*lp;
    int		i;
    xyz_t	outpt;

    for (i=0; i<RPScene.light_count; i++) {
	lp = RPScene.light_list[i];

	outpt.x = (v_mtx[0][0] * lp->pos.x +
                   v_mtx[1][0] * lp->pos.y +
                   v_mtx[2][0] * lp->pos.z +
                   v_mtx[3][0] * 1.0);
        outpt.y = (v_mtx[0][1] * lp->pos.x +
                   v_mtx[1][1] * lp->pos.y +
                   v_mtx[2][1] * lp->pos.z +
                   v_mtx[3][1] * 1.0);
        outpt.z = (v_mtx[0][2] * lp->pos.x +
                   v_mtx[1][2] * lp->pos.y +
                   v_mtx[2][2] * lp->pos.z +
                   v_mtx[3][2] * 1.0);
	/* ignore w term... */

	lp->pos.x = outpt.x;
	lp->pos.y = outpt.y;
	lp->pos.z = outpt.z;
    }
}


