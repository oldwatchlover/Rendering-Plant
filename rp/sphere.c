
/*
 * File:	sphere.c
 *
 * This file holds the data structures and functions that manipulate spheres.
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
#include <string.h>

#include "rp.h"

/*
 * Polygonal sphere, centered at 0,0,0 with radius of 1.0:
 * Use if the renderer does not support sphere primitives.
 *
 * static Vtx_t unit_sphere_vtx[642];
 * static Tri_t unit_sphere_tri[1280];
 *
 */
#include "unit_sphere.h"

extern int	_RPInputVertexCount;
extern void    	RPCalculateVertexNormals(Object_t *op, int trinormals);

static int	renderer_supports_sphere = TRUE;

/* callback for the renderer to ask for poly sphere instead of geom sphere: */
void
RPEnableSphereSupport(int val)
{
    renderer_supports_sphere = val;
}


/* adds a sphere to the scene. called from the parser */
void
RPAddSphere(xyz_t center, float radius)
{
    Object_t	*op;
    Sphere_t	*sp;

    if (renderer_supports_sphere) {
        op = RPAddObject(OBJ_TYPE_SPHERE);

        sp = (Sphere_t *) calloc(1, sizeof(Sphere_t));
        op->sphere = sp;

        sp->center.x = center.x; sp->center.y = center.y; sp->center.z = center.z;
        sp->radius = radius;

        if (Flagged(RPScene.flags, FLAG_VERBOSE)) {
	    fprintf(stderr," +Added Object %d ... sphere: c = (%8.3f,%8.3f,%8.3f) radius = %8.3f\n",
		    op->id,center.x, center.y, center.z, radius);
        }
    } else {	/* add a polygonal sphere */
        Vtx_t	*vp, tmp_vert;
        Tri_t	*tp;
        int	i;

	if (_RPInputVertexCount != 0) {
	    fprintf(stderr,"%s : ERROR : trying to add a sphere in the middle of a poly geom sequence\n",
			program_name);
	    return;
	}

	vp = &(unit_sphere_vtx[0]);
        for (i=0; i<unit_sphere_vtx_count; i++) {

		/* use center & radius to scale and translate fake sphere to the right place/size */
	    bcopy((void *)vp, (void *)&tmp_vert, sizeof(Vtx_t));
	    tmp_vert.pos.x *= radius;
	    tmp_vert.pos.y *= radius;
	    tmp_vert.pos.z *= radius;

	    tmp_vert.pos.x += center.x;
	    tmp_vert.pos.y += center.y;
	    tmp_vert.pos.z += center.z;

	    RPAddVertex(&tmp_vert, i);
	    vp++;
	}
        RPCloseVertexList(unit_sphere_vtx_count);

	tp = &(unit_sphere_tri[0]);
        for (i=0; i<unit_sphere_tri_count; i++) {
	    RPAddTriangle(tp);
	    tp++;
	}
        RPCloseTriangleList(unit_sphere_tri_count);

		/* bit of a hack, we know fake sphere was last object added to scene...
		 * grab it and make sure SMOOTH shading, CULL_BACK are on:
		 */
        op = RPScene.obj_list[RPScene.obj_count-1];
	UnFlag(op->material->flags, FLAG_FLATSHADE);
	UnFlag(op->material->flags, FLAG_CULL_FRONT);
	Flag(op->material->flags, FLAG_CULL_BACK);

		/* calculate surface normals: */
	RPCalculateVertexNormals(op, FALSE);

	       /* generate some basic texture coordinates
 		* (need to do this in MODEL space, before transform) 
		*/
	RPGenerateSphericalTexcoords(op);

        if (Flagged(RPScene.flags, FLAG_VERBOSE)) {
	    fprintf(stderr," +Added Object %d ... sphere: c = (%8.3f,%8.3f,%8.3f) radius = %8.3f\n",
		    op->id,center.x, center.y, center.z, radius);
	    fprintf(stderr,"\tWARNING - native sphere not supported, using polygonal mesh.\n");
        }
    }
}


/* apply top of MV stack to a sphere object: */
void
RPProcessSphere(Object_t *op, Sphere_t *sp)
{
    xyz_t       tmp_vtx;
    float       m[4][4], w;
    int		i, j;


	/* transform center: */
    cat_matrix(op->mmtx, v_mtx, m);
    transform_xyz(m, &(sp->center), &(sp->center), &w);

        /* to "transform" the radius, we must decompose object's model matrix: */
    ident_mtx(m);
    for (i=0; i<3; i++) {	/* skip copying translate terms */
        for (j=0; j<4; j++) {
	    m[i][j] = op->mmtx[i][j];
        }
    }
	/* techncally, we should decompose rotation from scale terms... however, 
         * this is a lot of ugly math and for a sphere, rotation doesn't 
         * matter (a sphere looks the same when rotated), so we'll just 
 	 * leave them combined as "scale"... (this will create undesirable
         * effects if the sphere is scaled non-uniformly, which is only
         * possible if we are using the polygonal sphere approximation)
	 */
    tmp_vtx.x = sp->radius;	/* make a test point to calc new radius */
    tmp_vtx.y = 0.0;
    tmp_vtx.z = 0.0;
    transform_xyz(m, &tmp_vtx, &tmp_vtx, &w);

	/* length of xformed vector is the new scaled radius: */
    sp->radius =  sqrtf(Sqr(tmp_vtx.x) + Sqr(tmp_vtx.y) + Sqr(tmp_vtx.z));
}


