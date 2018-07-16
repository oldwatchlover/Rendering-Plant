
/*
 * File:        face.c
 *
 * handles face / polygons
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
#include <string.h>
#include <math.h>

#include "objread.h"


static void
add_tri(int v0, int v1, int v2,
        int t0, int t1, int t2,
        int n0, int n1, int n2)
{
    tri_t	*tp;
    uv_t	*texp;
    vertex_t	*vp;

    if (input_tri_count >= MAX_TRIS) {
	fprintf(stderr,"ERROR : too many triangles! %d\n",input_tri_count);
	exit(-1);
    }

    tp = &(TriList[input_tri_count]);

	/* handle negative references from .obj file */
	/* obj file refs start at one, we want to start at 0 */

    tp->v0 = (v0 < 0) ? (input_vertex_count + v0) : (v0 - 1);
    tp->v1 = (v1 < 0) ? (input_vertex_count + v1) : (v1 - 1);
    tp->v2 = (v2 < 0) ? (input_vertex_count + v2) : (v2 - 1);

    tp->t0 = (t0 < 0) ? (input_texcoord_count + t0) : (t0 - 1);;
    tp->t1 = (t1 < 0) ? (input_texcoord_count + t1) : (t1 - 1);;
    tp->t2 = (t2 < 0) ? (input_texcoord_count + t2) : (t2 - 1);;

    tp->n0 = (n0 < 0) ? (input_normal_count + n0) : (n0 - 1);;
    tp->n1 = (n1 < 0) ? (input_normal_count + n1) : (n1 - 1);;
    tp->n2 = (n2 < 0) ? (input_normal_count + n2) : (n2 - 1);;

    tp->normal.x = 0.0;
    tp->normal.y = 0.0;
    tp->normal.z = 0.0;

    input_tri_count++;

	/* update tex coord for the vertices: */

    vp = &(VertexList[0]);
    texp = &(TexCoordList[0]);

    vp[tp->v0].tcoords.u = texp[tp->t0].u;
    vp[tp->v0].tcoords.v = texp[tp->t0].v;
    
    vp[tp->v1].tcoords.u = texp[tp->t1].u;
    vp[tp->v1].tcoords.v = texp[tp->t1].v;
    
    vp[tp->v2].tcoords.u = texp[tp->t2].u;
    vp[tp->v2].tcoords.v = texp[tp->t2].v;
    
}



void
add_face(int n, int v[], int vt[], int vn[])
{
    int		i;

    debug_printf(stderr, "add face : \t");
    for (i=0; i<n; i++) {
	debug_printf(stderr, "[%d %d %d] ",v[i],vt[i],vn[i]);
    }
    debug_printf(stderr, "\n");

        /* make a trifan out of poly with > 3 verts */

    for (i=3; i<=n; i++) {
        add_tri( v[0],  v[i-2],  v[i-1], 
                vt[0], vt[i-2], vt[i-1], 
                vn[0], vn[i-2], vn[i-1]);
    }
}


