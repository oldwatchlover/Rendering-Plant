
/*
 * File:        scan.c
 *
 * This top level of the scanline renderer 
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
#include <locale.h>

#include "rp.h"
#include "ray.h"
#include "scan.h"

int		epprocessed = 0;
int		avg_epp = 0;

static ep_t	**buckets;
static int	epcount = 0;

static void	fill_buckets(void);

void     
scan_scene(void)
{
    float		progress = 0.0;
    int			i;

    setlocale(LC_ALL,"");

    RPLoadBackgroundImage();

    fprintf(stderr,"Rendering Scene with Scanline Algorithm:\n");
    fprintf(stderr,"\tResolution %d x %d\n",RPScene.xres,RPScene.yres);
    fprintf(stderr,"\t[%d] objects...\n",RPScene.obj_count);
    fprintf(stderr,"\t[%d] lights...\n",RPScene.light_count);

        /* tranform objects to camera space */
    RPProcessObjects(TRUE);

    /* build edgepair structure */

    buckets = (ep_t **) calloc(RPScene.yres, sizeof(ep_t *));

    fill_buckets();

	/* set up primary camera ray paramters */
    raytracer_init();

    fprintf(stderr,"Progress:  %5.2f %%",progress*100.0);

    for (i=0; i<RPScene.yres; i++) {

/*
	fprintf(stderr,"bucket %d : ",i);
	print_edgepairs(buckets[i]);
*/

	/* process edgepairs */

	process_edgepairs(i, &(buckets[i]));

	free_eplist(&(buckets[i]));

        progress = (float)i/(float)RPScene.yres;
        fprintf(stderr,"\b\b\b\b\b\b\b%5.2f %%",progress*100.0);
    }

    for (i=0; i<RPScene.obj_count; i++) {
        Object_t	*op;
        Tri_t		*tp;
        Vtx_t		*v0, *v1, *v2;
        rgba_t		red = {255, 0, 0, 255};
        int		j;

	op = RPScene.obj_list[i];
	for (j=0; j<op->tri_count; j++) {
	    tp = &(op->tris[j]);

	    v0  = &(op->verts[tp->v0]);
	    v1  = &(op->verts[tp->v1]);
	    v2  = &(op->verts[tp->v2]);

	    if ( Flagged(RPScene.generic_flags, FLAG_RENDER_02) &&
		!Flagged(tp->flags, FLAG_TRI_CLIPPED)) {

	        RPDrawColorFBLine(v0->sx, v0->sy, v1->sx, v1->sy, red, TRUE);
	        RPDrawColorFBLine(v1->sx, v1->sy, v2->sx, v2->sy, red, TRUE);
	        RPDrawColorFBLine(v2->sx, v2->sy, v0->sx, v0->sy, red, TRUE);
            }
        }
    }

    RPCleanupObjects();
    RPCleanupTextures();
    RPCleanupMaterials();

    fprintf(stderr,"\b\b\b\b\b\b\b\b100 %% ... done!\n");

    fprintf(stderr,"\n%s : Rendering Summary:\n",program_name);
    fprintf(stderr,"------------------------------------------------------\n");
    fprintf(stderr,"%s : [%'16d]\tinput polygons\n",
            program_name, RPScene.input_polys);
    fprintf(stderr,"%s : [%'16d]\tintersections avoided with culled polygons\n",
            program_name, RayStats.culled_polys);
    fprintf(stderr,"%s : [%'16d]\ttrivially rejected polygons\n",
            program_name, RPScene.trivial_rejected_polys);
    fprintf(stderr,"%s : [%'16d]\tclipped polygons\n",
            program_name, RPScene.clipped_polys);

    fprintf(stderr,"%s : [%'16d]\tedge pairs created\n",
	    program_name, epcount);
    fprintf(stderr,"%s : [%'16d]\tedge pairs processed\n",
	    program_name, epprocessed);
    fprintf(stderr,"%s : [%16.2f]\tavg edge pairs per pixel\n",
	    program_name, (float)avg_epp/(float)RayStats.primary_ray_count);

    fprintf(stderr,"%s : [%'16d]\tprimary rays cast\t(%'d hits)\n",
	    program_name, RayStats.primary_ray_count, RayStats.primary_ray_hit_count);
    fprintf(stderr,"%s : [%'16d]\treflection rays cast\t(%'d hits)\n",
	    program_name, RayStats.reflection_ray_count, RayStats.reflection_ray_hit_count);
    fprintf(stderr,"%s : [%'16d]\trefraction rays cast\t(%'d hits)\n",
	    program_name, RayStats.refraction_ray_count, RayStats.refraction_ray_hit_count);
    fprintf(stderr,"%s : [%'16d]\tshadow rays cast\t(%'d hits)\n", 
	    program_name, RayStats.shadow_ray_count, RayStats.shadow_ray_hit_count);
    fprintf(stderr,"\n");
}

static void
insert_poly(Object_t *op, int poly)
{
    Tri_t	*tp;
    Vtx_t	*v0, *v1, *v2;
    ep_t	*ep;
    int		i, miny, maxy, clipped = CLIP_TRIVIAL_ACCEPT;;

    tp = &(op->tris[poly]);

    v0  = &(op->verts[tp->v0]);
    v1  = &(op->verts[tp->v1]);
    v2  = &(op->verts[tp->v2]);

    if (!Flagged(RPScene.generic_flags, FLAG_RENDER_01)) /* turn off clipping */
        clipped = RPClipTriangle(op, tp); 

    if (clipped < CLIP_TRIVIAL_ACCEPT) {
	return;
    } 

    tp = &(op->tris[poly]);
    if (Flagged(tp->flags, FLAG_TRI_CLIPPED)) {
	/* poly was clipped, generating new data, so skip the old tri */
	/* if poly was clipped, any new polys added to the end of object trilist */
	/* we will get to them in their own time... */
	return;
    } 

/*
    fprintf(stderr,"insert poly (%d):\n",poly);
    fprintf(stderr,"\tflags = %08x\n",tp->flags);
    fprintf(stderr,"\tv indx= %d %d %d\n",tp->v0,tp->v1,tp->v2);
    fprintf(stderr,"\tv0 : %d, %d : %8.4f, %8.4f, %8.4f\n",
		v0->sx,v0->sy, v0->pos.x, v0->pos.y, v0->pos.z);
    fprintf(stderr,"\tv1 : %d, %d : %8.4f, %8.4f, %8.4f\n",v1->sx,v1->sy,
		v1->pos.x, v1->pos.y, v1->pos.z);
    fprintf(stderr,"\tv2 : %d, %d : %8.4f, %8.4f, %8.4f\n",v2->sx,v2->sy,
		v2->pos.x, v2->pos.y, v2->pos.z);

    fprintf(stderr,"\tcolor = %f,%f,%f,%f\n",tp->color.r,tp->color.g,tp->color.b,tp->color.a);
    fprintf(stderr,"\tnorm = %f,%f,%f\n",tp->normal.x,tp->normal.y,tp->normal.z);
    fprintf(stderr,"\tpN   = %f,%f,%f\n",tp->pN.x,tp->pN.y,tp->pN.z);
    fprintf(stderr,"\td    = %f\n",tp->d);
    fprintf(stderr,"\tv1v0 = %f,%f,%f\n",tp->v1_v0.x,tp->v1_v0.y,tp->v1_v0.z);
    fprintf(stderr,"\tv2v1 = %f,%f,%f\n",tp->v2_v1.x,tp->v2_v1.y,tp->v2_v1.z);
    fprintf(stderr,"\tv0v2 = %f,%f,%f\n",tp->v0_v2.x,tp->v0_v2.y,tp->v0_v2.z);
*/

	/* compute min and max */
    miny = Min3(v0->sy, v1->sy, v2->sy);
    maxy = Max3(v0->sy, v1->sy, v2->sy);

	/* insert into all the right y-buckets */
    for (i=miny; i<=maxy; i++) {
	if (i >= 0 && i < RPScene.yres) {

            ep = new_ep();
            ep->min.x = Min3(v0->sx, v1->sx, v2->sx);
            ep->min.y = miny;
            ep->max.x = Max3(v0->sx, v1->sx, v2->sx);
            ep->max.y = maxy;
            ep->id = epcount++;
            ep->op = op;
            ep->polyid = poly;
            ep->next = (ep_t *) NULL;

	    insert_edgepair(&(buckets[i]), ep);
        }
    }
}

/* put all the polygons in the scene into the right bucket(s) */
static void
fill_buckets(void)
{
    Object_t	*op;
    int		i, j;

    for (i=0; i<RPScene.yres; i++) {
	buckets[i] = (ep_t *) NULL;
    }

    for (i=0; i<RPScene.obj_count; i++) {
	op = RPScene.obj_list[i];
	for (j=0; j<op->tri_count; j++) {
	    insert_poly(op, j);
	}
    }
}


