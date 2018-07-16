
/*
 * File:	polygons.c
 *
 * this file holds the variables and functions to manipulate polygons.
 *
 * (only triangles supported)
 *
 * There are no begin/end objects in the input... when a trilist[] is read in,
 * that assumes to be the "close" of the object, creating an object using the 
 * current vertex list, material, textures, etc.
 *
 * That means all vertices, materials, etc. must come before the tri list 
 * in the input scene description.
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

/* from vertex.c, temp input data: */
extern Vtx_t	*_RPInputVertexBuffer;
extern int	_RPInputVertexCount;

        /* temporary input triangle buffer, allocated before reading scene,
         * freed after reading the scene, before rendering that scene
         */
static Tri_t   *tri_buffer = (Tri_t *) NULL;
static int     tri_count = 0;

void	RPCalculateVertexNormals(Object_t *op, int trinormals);

/* init input poly buffer: */
void
RPInitInputPolygons(void)
{
    RPScene.input_polys = 0;
    tri_buffer = (Tri_t *) calloc(MAX_TRIS, sizeof(Tri_t));
}

void
RPFreeInputPolygons(void)
{
    if (tri_buffer != (Tri_t *)NULL)
	free(tri_buffer);
}


/* take the current vertices and triangles and make an object */
void
RPCloseTriangleList(int tcount)
{
    Object_t	*op;
    
    if (tcount != tri_count) {
	fprintf(stderr,"%s : WARNING : something wrong, %d != %d tri count\n",
		program_name, tcount, tri_count);
    }

	/* create new object, set type, flags, id, material, texture */
    op = RPAddObject(OBJ_TYPE_POLY);

    op->sphere = NULL;	/* bounding sphere put here later */
    op->verts = (Vtx_t *) malloc(_RPInputVertexCount * sizeof(Vtx_t));
    op->tris = (Tri_t *) malloc(tri_count * sizeof(Tri_t));
    op->tri_count = tri_count;
    op->vert_count = _RPInputVertexCount;

    RPScene.input_polys += tri_count;

	/* copy geometry data */
    memcpy((void *)op->verts, (void *)&(_RPInputVertexBuffer[0]), 
		sizeof(Vtx_t)*_RPInputVertexCount);
    memcpy((void *)op->tris, (void *)&(tri_buffer[0]), sizeof(Tri_t)*tri_count);

    _RPInputVertexCount = 0;	/* reset input buffer for the next input object */
    tri_count = 0;

    if (Flagged(RPScene.flags, FLAG_VERBOSE)) {
	fprintf(stderr," +Added Object %d ... poly: %d verts, %d triangles\n",
		op->id,op->vert_count,op->tri_count);
    }
}

/* add a triangle to the triangle list: */
void
RPAddTriangle(Tri_t *tri)
{
    if (tri != (Tri_t *) NULL) {
        bcopy((char *)tri, (char *)&(tri_buffer[tri_count]), sizeof(Tri_t));
        tri_count++;
    }
}

/* separate this out to handle one tri - clipping funcs may need it */
void
RPProcessOneTriangle(Object_t *op, Tri_t *tri)
{
    Vtx_t	*vp;
    xyz_t	v2_v0;
    int		v0, v1, v2;

    vp = op->verts;

    v0 = tri->v0;
    v1 = tri->v1;
    v2 = tri->v2;

    if (Flagged(op->material->flags, FLAG_RANDSHADE)) {
        vp[v0].r = RPRandom();
        vp[v0].g = RPRandom();
        vp[v0].b = RPRandom();
        vp[v1].r = RPRandom();
        vp[v1].g = RPRandom();
        vp[v1].b = RPRandom();
        vp[v2].r = RPRandom();
        vp[v2].g = RPRandom();
        vp[v2].b = RPRandom();
	vp[v0].a = vp[v1].a = vp[v2].a = 1.0;
    }

    vector_sub(&(tri->v1_v0), &(vp[v1].pos), &(vp[v0].pos));
    vector_sub(&(tri->v2_v1), &(vp[v2].pos), &(vp[v1].pos));
    vector_sub(&(tri->v0_v2), &(vp[v0].pos), &(vp[v2].pos));
    vector_sub(&v2_v0, &(vp[v2].pos), &(vp[v0].pos));

    vector_cross(&(tri->normal), &(tri->v1_v0), &v2_v0);
    tri->pN.x = tri->normal.x;
    tri->pN.y = tri->normal.y;
    tri->pN.z = tri->normal.z;
    tri->d = vector_dot(tri->pN, vp[v0].pos);/* d = normal . (any_vertex) */

    vector_normalize(&(tri->normal));/* normalize for shading calc */

    if (vector_dot(vp[v0].pos, tri->normal) > 0.0) {	/* eye at 0,0,0 */
        Flag(tri->flags, FLAG_CULL_BACK);
    } else {
        Flag(tri->flags, FLAG_CULL_FRONT);
    }
}

/* compute normal, diff vectors, and d for intersection/shading */
void
RPProcessAllTriangles(Object_t *op, int count, Tri_t *tp)
{
    int		i;

    if ((op == (Object_t *) NULL) || (tp == (Tri_t *) NULL)) {
	fprintf(stderr,"%s : ERROR : bad object. (%s, %d)\n",
		program_name, __FILE__, __LINE__);
	return;
    }

    if ((op->verts == (Vtx_t *) NULL) || (op->tris  == (Tri_t *) NULL) || 
	(op->material == (Material_t *) NULL)) {
	fprintf(stderr,"%s : ERROR : bad object. (%s, %d)\n",
		program_name, __FILE__, __LINE__);
	return;
    }

    RPTransformAllVertices(op, op->vert_count, op->verts);

    for (i=0; i<count; i++) {
	tp[i].flags = 0x0;
	RPProcessOneTriangle(op, &(tp[i]));
    }
}

/* averages vertex normals, depending on how may triangle faces share
 * that vertex
 */ 
void
RPCalculateVertexNormals(Object_t *op, int trinormals)
{
    Vtx_t	*vp;
    Tri_t	*tp, *tri;
    xyz_t	*np, v1_v0, v2_v0;
    float	denom;
    int		i, v0, v1, v2, *ncounts;

    vp = op->verts;
    tp = op->tris;

    if (!trinormals) {	/* need to gen poly normal first */
	for (i=0; i<op->tri_count; i++) {

	    tri = &(tp[i]);
    	    vector_sub(&v1_v0, &(vp[tri->v1].pos), &(vp[tri->v0].pos));
    	    vector_sub(&v2_v0, &(vp[tri->v2].pos), &(vp[tri->v0].pos));
    	    vector_cross(&(tri->normal), &v1_v0, &v2_v0);
    	    vector_normalize(&(tri->normal));/* normalize for shading calc */
	}
    }

    np = (xyz_t *) calloc(op->vert_count, sizeof(xyz_t));
    ncounts = (int *) calloc(op->vert_count, sizeof(int));

	/* sum normal for each vert that uses this triangle */
    for (i=0; i<op->tri_count; i++) {
	v0 = tp[i].v0;
	v1 = tp[i].v1;
	v2 = tp[i].v2;

	vector_add(&(np[v0]), &(np[v0]), &(tp[i].normal));
	ncounts[v0]++;

	vector_add(&(np[v1]), &(np[v1]), &(tp[i].normal));
	ncounts[v1]++;

	vector_add(&(np[v2]), &(np[v2]), &(tp[i].normal));
	ncounts[v2]++;
    }

	/* div by that vtx norm count (average) */
    for (i=0; i<op->vert_count; i++) {
	if (ncounts[i] != 0) {
	    denom = 1.0 / (float)ncounts[i];
	    vector_scale(&(np[i]), &(np[i]), denom);
	}
	vector_normalize(&(np[i]));

	vp[i].n.x = np[i].x;
	vp[i].n.y = np[i].y;
	vp[i].n.z = np[i].z;
    }

    free(np);
    free(ncounts);
}


