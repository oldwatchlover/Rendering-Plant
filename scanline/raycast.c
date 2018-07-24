
/*
 * File:        raycast.c
 *
 * Takes the active edgepair list from the scanline agorithm and
 * casts a primary ray into the scene.
 *
 * Leverages much of ../ray/libray, EXCEPT:
 *
 *    - primary ray casting is simplified (one pixel in the screen at a time)
 *    - primary ray intersection is triangle only, from the edgepair list 
 *      (not objects in the scene)
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
#include "ray.h"
#include "scan.h"

extern int	epprocessed;

static float	tanfov, sinfov;

static rgba_t *
trace_primary_ray(Ray_t *ray, ep_t *eplist)
{
    ep_t	*ep = eplist;
    Object_t    *op;
    Material_t  *m;
    Tri_t	*tp;
    rgba_t      *color = (rgba_t *) NULL;
    xyz_t       surf, view, normal;
    float       t = MAX_RAY_T;
    int         cullthis = FALSE, found = FALSE;
    ray->depth++;

    color = (rgba_t *) malloc(sizeof(rgba_t));

        /* handle fog in the background */
    if (Flagged(RPScene.flags, FLAG_FOG)) {
        color->r = (u8) Clamp0255(RPScene.fog_color.r * 255.0);
        color->g = (u8) Clamp0255(RPScene.fog_color.g * 255.0);
        color->b = (u8) Clamp0255(RPScene.fog_color.b * 255.0);
        color->a = (u8) Clamp0255(RPScene.fog_color.a * 255.0);
    } else {
        color->r = RPScene.background_color.r;
        color->g = RPScene.background_color.g;
        color->b = RPScene.background_color.b;
        color->a = RPScene.background_color.a;
    }
   
        /* intersect ray with all of the edgepairs */

    while (ep != (ep_t *) NULL) {

	op = ep->op;
        tp = &(op->tris[ep->polyid]);
   
            /* do front/back face culling here */
        cullthis = FALSE;
        if (ray->type == PRIMARY_RAY &&
            Flagged(op->flags, FLAG_CULL_BACK)) {
            if (Flagged(tp->flags, FLAG_CULL_BACK))
                cullthis = TRUE;
        }
        if (ray->type == PRIMARY_RAY &&
            Flagged(op->flags, FLAG_CULL_FRONT)) {
            if (Flagged(tp->flags, FLAG_CULL_FRONT))
                cullthis = TRUE;
        }

	if (cullthis) {
	    found = FALSE;
	    RayStats.culled_polys++;
	} else {
            found = tri_intersect(ray, op, tp, &t, &surf, &normal);
        }

        /* if hit, calc shade */
        if (found && t < ray->t) {

            ray->t = t;
            m = op->material;
            vector_scale(&view, &(ray->dir), -1.0f); /* view vector is -ray.dir */


            RayStats.primary_ray_hit_count++;

            shade_tri_pixel(color, m, ray, &normal, &surf, &view, op);

	    if (ray->surf != (TriShade_t *) NULL)
	        free (ray->surf);
        } else {
	    if (found) {
	        if (ray->surf != (TriShade_t *) NULL)
	            free (ray->surf);
	    }
	}

	ep = ep->next;
	epprocessed++;
    }

    return (color);
}


void
cast_primary_ray(int x, int y, ep_t *eplist)
{
    rgba_t	*color;
    Ray_t	*eyeray;

    eyeray = NewRay(PRIMARY_RAY, -1);

    eyeray->depth = 0;
    eyeray->t = MAX_RAY_T;
    eyeray->surf = (TriShade_t *) NULL;

    eyeray->orig.x = RPScene.camera->eye.x;       /* orig is (0,0,0) */
    eyeray->orig.y = RPScene.camera->eye.y;
    eyeray->orig.z = RPScene.camera->eye.z;

        /* clean up this obfuscated code... */
    eyeray->dir.x = (2.0 * (x + 0.5) / (float)RPScene.xres - 1.0) * 
			RPScene.camera->aspect * tanfov;
    eyeray->dir.y = (1.0 - 2 * (y + 0.5) / (float)RPScene.yres) * tanfov;;
    eyeray->dir.z = RPScene.camera->dir.z;
    vector_normalize(&(eyeray->dir));

    color = trace_primary_ray(eyeray, eplist);

    if (color != NULL) {
        if (eyeray->t == MAX_RAY_T &&
            Flagged(RPScene.flags, FLAG_BACKGROUND_IMAGE)) {
                     /* miss, but background image was loaded */
         } else {
            RPColorFrameBuffer[y][x].r = color->r;
            RPColorFrameBuffer[y][x].g = color->g;
            RPColorFrameBuffer[y][x].b = color->b;
            RPColorFrameBuffer[y][x].a = color->a;
         }
        free(color); /* free color allocated in trace_ray() */
        color = (rgba_t *) NULL;
    } else {
        /* ray depth exceeded, no color returned */
    }
    RayStats.primary_ray_count++;

    free(eyeray);
}

void
raytracer_init(void)
{
        /* fov is actually fov/2.0 */
    tanfov = tanf(RPScene.camera->fovr/2.0);
    sinfov = sinf(RPScene.camera->fovr/2.0);
}


