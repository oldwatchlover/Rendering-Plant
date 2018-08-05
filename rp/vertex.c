
/*
 * File:	vertex.c
 *
 * This file holds the data structures and functions that manipulate vertices.
 *
 */

/*
 * * MIT License
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

	/* temporary input vertex buffer, allocated before reading scene,
         * freed after reading the scene, before rendering that scene
 	 * These variables are accessed in polygon.c as well.
         */
Vtx_t	*_RPInputVertexBuffer = (Vtx_t *) NULL;
int	_RPInputVertexCount = 0;

/* init temp input vertex buffer: */
void
RPInitInputVertices(void)
{
    _RPInputVertexBuffer = (Vtx_t *) calloc(MAX_VERTS, sizeof(Vtx_t));
}

void
RPFreeInputVertices(void)
{
    if (_RPInputVertexBuffer != (Vtx_t *) NULL)
	free(_RPInputVertexBuffer);
}


/* add a vertex to the vertex list: (called from input parser) */
void
RPAddVertex(Vtx_t *vtx, int i)
{
        /* max exceeded, extend array */
    if (_RPInputVertexCount >= MAX_VERTS) {
        _RPInputVertexBuffer = (Vtx_t *) realloc(_RPInputVertexBuffer, 
		(_RPInputVertexCount + MAX_VERTS) * sizeof(Vtx_t));
    }
    
    bcopy((void *)vtx, (void *)&(_RPInputVertexBuffer[i]), sizeof(Vtx_t));
    _RPInputVertexCount++;
}

/* sets the vertex count to match the input: (called from the parser) */
void
RPCloseVertexList(int vcount)
{
    if (vcount != _RPInputVertexCount) {
        fprintf(stderr,"%s : WARNING : something wrong, %d != %d vertex count\n",
                program_name, vcount, _RPInputVertexCount);
    }

    _RPInputVertexCount = vcount;
}

/* transform a list of vertices by MODEL x VIEW matrices */
void
RPTransformAllVertices(Object_t *op, int count, Vtx_t *verts)
{
    int		i;
    xyz_t	outpt;
    float	mv[4][4], mvit[4][4], w;

	/* compute proper MV matrix for this object */
    cat_matrix(op->mmtx, v_mtx, mv);

	/* if we transform normals, we need inverse transpose of MV */
    if (Flagged(op->flags, FLAG_VERTNORM)) {
        invert_mtx(mv, mvit); 		/* compute inverse of MV */
	transpose_mtx(mvit, mvit);	/* compute transpose of MV^-1 */
    }

#ifdef DEBUG
    if (Flagged(RPScene.flags, FLAG_VERBOSE2)) {
	int	i;
	fprintf(stderr,"TRANSFORMING object %d : %d POINTS BY MV MATRIX :\n",
		op->id, count);
	for (i=0; i<4; i++) {
	    fprintf(stderr,"[ %10.4f, %10.4f, %10.4f, %10.4f ]\n",
		mv[i][0], mv[i][1], mv[i][2], mv[i][3]);
	}
	fprintf(stderr,"\tM MATRIX :\n");
	for (i=0; i<4; i++) {
	    fprintf(stderr,"\t\t[ %10.4f, %10.4f, %10.4f, %10.4f ]\n",
		op->mmtx[i][0], op->mmtx[i][1], op->mmtx[i][2], op->mmtx[i][3]);
	}
	fprintf(stderr,"V MATRIX :\n");
	for (i=0; i<4; i++) {
	    fprintf(stderr,"[ %10.4f, %10.4f, %10.4f, %10.4f ]\n",
		v_mtx[i][0], v_mtx[i][1], v_mtx[i][2], v_mtx[i][3]);
	}

	fprintf(stderr,"POINTS BEFORE TRANSFORM:\n");
	for (i=0; i<count; i++) {
	    fprintf(stderr,"\t( %10.4f %10.4f %10.4f, 1.0 )\n", 
			verts[i].pos.x, verts[i].pos.y, verts[i].pos.z);
	}

	fprintf(stderr,"POINTS AFTER TRANSFORM:\n");
    }
#endif

    for (i=0; i<count; i++) {

	outpt.x = (mv[0][0] * verts[i].pos.x + 
		   mv[1][0] * verts[i].pos.y +
		   mv[2][0] * verts[i].pos.z +
		   mv[3][0] * 1.0);
	outpt.y = (mv[0][1] * verts[i].pos.x + 
		   mv[1][1] * verts[i].pos.y +
		   mv[2][1] * verts[i].pos.z +
		   mv[3][1] * 1.0);
	outpt.z = (mv[0][2] * verts[i].pos.x + 
		   mv[1][2] * verts[i].pos.y +
		   mv[2][2] * verts[i].pos.z +
		   mv[3][2] * 1.0);
        w       = (mv[0][3] * verts[i].pos.x +
                   mv[1][3] * verts[i].pos.y +
                   mv[2][3] * verts[i].pos.z +
                   mv[3][3] * 1.0);

	verts[i].pos.x = outpt.x;
	verts[i].pos.y = outpt.y;
	verts[i].pos.z = outpt.z;
	verts[i].w = w;

	/* project and divide done in vertex_project() */

	/* transform normals */
	if (Flagged(op->flags, FLAG_VERTNORM)) {

		/* transform normals: */
	    outpt.x = (mvit[0][0] * verts[i].n.x + 
	               mvit[1][0] * verts[i].n.y +
	               mvit[2][0] * verts[i].n.z +
	               mvit[3][0] * 1.0);
	    outpt.y = (mvit[0][1] * verts[i].n.x + 
		       mvit[1][1] * verts[i].n.y +
		       mvit[2][1] * verts[i].n.z +
		       mvit[3][1] * 1.0);
	    outpt.z = (mvit[0][2] * verts[i].n.x + 
		       mvit[1][2] * verts[i].n.y +
		       mvit[2][2] * verts[i].n.z +
		       mvit[3][2] * 1.0);
	    verts[i].n.x = outpt.x;
	    verts[i].n.y = outpt.y;
	    verts[i].n.z = outpt.z;

	    vector_normalize(&(verts[i].n));
 	}

	/* compute e vector, if rasterizer/shader needs it */
	verts[i].e.x = 0.0 - verts[i].pos.x;
	verts[i].e.y = 0.0 - verts[i].pos.y;
	verts[i].e.z = 0.0 - verts[i].pos.z;
	vector_normalize(&(verts[i].e));

#ifdef DEBUG
	if (Flagged(RPScene.flags, FLAG_VERBOSE2)) {
	    fprintf(stderr,"\t( %10.4f %10.4f %10.4f, %10.4f )\n", 
		    verts[i].pos.x,verts[i].pos.y,verts[i].pos.z, verts[i].w);
	}
#endif
    }
}

/* project a list of vertices, multiply eye space vertex by P matrix 
 * then divide by w and do viewport mapping
 */
void
RPProjectAllVertices(int count, Vtx_t *vp)
{
    xyz_t	t;
    float	w;
    int 	j;

#ifdef DEBUG
    if (Flagged(RPScene.flags, FLAG_VERBOSE2)) {
	int	i;
	fprintf(stderr,"PROJECTING %d POINTS BY P MATRIX :\n", count);
	for (i=0; i<4; i++) {
	    fprintf(stderr,"[ %10.4f, %10.4f, %10.4f, %10.4f ]\n",
		p_mtx[i][0], p_mtx[i][1], p_mtx[i][2], p_mtx[i][3]);
	}
	fprintf(stderr,"POINTS AFTER PROJECTION:\n");
    }
#endif

    for (j=0; j<count; j++) {

        vp[j].proj.x = (p_mtx[0][0] * vp[j].pos.x +
                        p_mtx[1][0] * vp[j].pos.y +
                        p_mtx[2][0] * vp[j].pos.z +
                        p_mtx[3][0] * vp[j].w);
        vp[j].proj.y = (p_mtx[0][1] * vp[j].pos.x +
                        p_mtx[1][1] * vp[j].pos.y +
                        p_mtx[2][1] * vp[j].pos.z +
                        p_mtx[3][1] * vp[j].w);
        vp[j].proj.z = (p_mtx[0][2] * vp[j].pos.x +
                        p_mtx[1][2] * vp[j].pos.y +
                        p_mtx[2][2] * vp[j].pos.z +
                        p_mtx[3][2] * vp[j].w);
        w            = (p_mtx[0][3] * vp[j].pos.x +
                        p_mtx[1][3] * vp[j].pos.y +
                        p_mtx[2][3] * vp[j].pos.z +
                        p_mtx[3][3] * vp[j].w);
        vp[j].w = w;

	    /* must generate clip codes while in PROJ space */
        vp[j].cc = RPGenerateVertexClipcodes(&(vp[j].proj), w);

        if (NearlyZero(vp[j].w, Epsilon)) {	/* project */
            vp[j].inv_w = 1.0;
        } else {
            vp[j].inv_w = 1.0/vp[j].w;
        }
        t.x = vp[j].proj.x * vp[j].inv_w;
        t.y = vp[j].proj.y * vp[j].inv_w;
        t.z = vp[j].proj.z * vp[j].inv_w;

        /* some algorithms shade in world space but the default is to 
         * leave the texture coordinates un-corrected by perspective 
	 * the rasterizer code will do the divide if necessary
         */

        /* viewport mapping (flip y due to BMP image space orientation) */
        vp[j].sx = (int) (t.x *  RPScene.viewport->sx + RPScene.viewport->tx + 0.5);
        vp[j].sy = (int) (t.y * -RPScene.viewport->sy + RPScene.viewport->ty + 0.5);
        vp[j].sz = (int) (t.z *  RPScene.viewport->sz + RPScene.viewport->tz + 0.5);

#ifdef DEBUG
        if (Flagged(RPScene.flags, FLAG_VERBOSE2)) {
            fprintf(stderr,"\teye points:\t%8.3f, %8.3f, %8.3f, %8.3f\n",
                vp[j].pos.x, vp[j].pos.y, vp[j].pos.z, vp[j].w);
            fprintf(stderr,"\tproj points:\t%8.3f, %8.3f, %8.3f\n",
                vp[j].proj.x, vp[j].proj.y, vp[j].proj.z);
            fprintf(stderr,"\tproj/w points:\t%8.3f, %8.3f, %8.3f\n",
                t.x, t.y, t.z);
            fprintf(stderr,"\tscrn points:\t%d, %d, %d\n",
                vp[j].sx, vp[j].sy, vp[j].sz);
	    fprintf(stderr,"\ttex coords: %f %f\n",vp[j].s,vp[j].t);
        }
#endif
    } 
}


