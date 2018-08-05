
/*
 * File:        vertex.c
 *
 * process vertex commands from the .obj file
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

void
add_vertex(float x, float y, float z, float w)
{
    vertex_t	*vp;

    debug_printf(stderr,"adding v : \t%8.3f, %8.3f, %8.3f, %8.3f\n",x,y,z,w);

    if (input_vertex_count >= MAX_VERTS) {
        VertexList = (vertex_t *) realloc(VertexList, 
			(input_vertex_count + MAX_VERTS) * sizeof(Tri_t));
    }

    vp = &(VertexList[input_vertex_count]);

    vp->pos.x = x;
    vp->pos.y = y;
    vp->pos.z = z;

    vp->color.r = 1.0;
    vp->color.g = 1.0;
    vp->color.b = 1.0;
    vp->color.a = 1.0;

    vp->tcoords.u = 0.0;
    vp->tcoords.v = 0.0;

    vp->normal.x = 0.0;
    vp->normal.y = 0.0;
    vp->normal.z = 0.0;

    input_vertex_count++;
}

void
add_vtexcoords(int n, float u, float v, float w)
{
    uv_t	*tp;

    debug_printf(stderr,"adding vt %d : \t%8.3f, %8.3f, %8.3f\n",n,u,v,w);

    if (input_texcoord_count >= MAX_VERTS) {
        TexCoordList = (uv_t *) realloc(TexCoordList, 
			(input_texcoord_count + MAX_VERTS) * sizeof(uv_t));
    }

    tp = &(TexCoordList[input_texcoord_count]);

    tp->u = u;
    tp->v = v;

    input_texcoord_count++;
}

void
add_vnormal(float x, float y, float z)
{
    xyz_t	*np;

    debug_printf(stderr,"adding vn : \t%8.3f, %8.3f, %8.3f\n",x,y,z);

    if (input_normal_count >= MAX_VERTS) {
        NormalList = (xyz_t *) realloc(NormalList, 
			(input_normal_count + MAX_VERTS) * sizeof(xyz_t));
    }

    np = &(NormalList[input_normal_count]);

    np->x = x;
    np->y = y;
    np->z = z;

    input_normal_count++;
}



