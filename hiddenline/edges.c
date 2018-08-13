
/*
 * File:        edges.c
 *
 * builds and proceses list of edges to be drawn
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

/* enable this to print debug info about the edges: 
#define DEBUG_EDGES
*/

#define DEPTH_BIAS	(0.001)

/* this is compared with the cos() of angle between tris sharing a edge */
#define CREASE_TOLERANCE	(0.3)

/* tabel of edges for all the objects */
static ObjEdges_t	*objEdges[MAX_OBJS];


/* find an edge in the list, order of verts doesn't matter */
static int
find_edge(int id, int p0, int p1)
{
    Edge_t	*ep;
    int		i, ecnt;

    ep = (Edge_t *) objEdges[id]->edges;
    ecnt = objEdges[id]->num_edges;

    for (i=0; i<ecnt; i++) {
	if ((ep[i].v0 == p0 && ep[i].v1 == p1) ||
	    (ep[i].v0 == p1 && ep[i].v1 == p0)) {

	    return(i);		 /* found! */
	} 
    }

    return (-1);
}

/* add edge to the list... either as a shared edge or a new edge */
static void
add_edge(Object_t *op, int id, int thistri, int p0, int p1)
{
    Vtx_t	*vp;
    Edge_t	*ep;
    int		thisedge;

    ep = (Edge_t *) objEdges[id]->edges;
    vp = (Vtx_t *) op->verts;

    thisedge = find_edge(id, p0, p1);

    if (thisedge < 0) {		/* add new edge to the list */

	ep[objEdges[id]->num_edges].id = total_edges;
	ep[objEdges[id]->num_edges].tri0 = thistri;
	ep[objEdges[id]->num_edges].v0 = p0;
	ep[objEdges[id]->num_edges].v1 = p1;
	objEdges[id]->num_edges++;

	total_edges++;

#ifdef DEBUG_EDGES
	fprintf(stderr,"\tadding new edge {%d}: p0 = %d, p1 = %d from tri %d\n",
			ep[objEdges[id]->num_edges-1].id,
			p0,p1,thistri);
	fprintf(stderr,"\t\t(%d,%d) -- (%d,%d)\n",
		vp[p0].sx,vp[p0].sy, vp[p1].sx,vp[p1].sy);
#endif
    } else {			/* add tri to an existing edge */

	if (ep[thisedge].tri1 < 0) {

	    ep[thisedge].tri1 = thistri;

#ifdef DEBUG_EDGES
	fprintf(stderr,"\tadding tri (%d) to edge (%d) {%d}: p0 = %d, p1 = %d\n",
		thistri,thisedge, ep[objEdges[id]->num_edges-1].id, p0,p1);
	fprintf(stderr,"\t\t(%d,%d) -- (%d,%d)\n",
		vp[p0].sx,vp[p0].sy, vp[p1].sx,vp[p1].sy);
#endif
        } else {
	    	/* error, sholdn't happen except degenerate data */
#ifdef DEBUG_EDGES
	    fprintf(stderr,"%s : ERROR : tried to add edge to a full edge pair (%d)\n",
		program_name, thisedge);
	    fprintf(stderr,"\tv0 = %d, v1 = %d ... p0 = %d, p1 = %d... tri0 = %d tri1 = %d\n",
		ep[thisedge].v0, ep[thisedge].v1, p0, p1, ep[thisedge].tri0,ep[thisedge].tri1);
#endif
	}
    }
}

/* create edge list for this object */
void
create_obj_edges(Object_t *op)
{
    Tri_t	*tp;
    Edge_t	*ep;
    int		i;

    objEdges[op->id] = (ObjEdges_t *) calloc(1, sizeof(ObjEdges_t));
    ep = (Edge_t *) calloc(3*op->tri_count, sizeof(Edge_t));

	/* initialize edge list */
    for (i=0; i<3*op->tri_count; i++) {

	ep[i].flags = 0x0;
	ep[i].tri0 = -1;
	ep[i].tri1 = -1;
	ep[i].v0 = -1;
	ep[i].v1 = -1;
    }

    objEdges[op->id]->edges = ep;
    objEdges[op->id]->num_edges = 0;

	/* populate edge table */
    for (i=0; i<op->tri_count; i++) {

	tp = (Tri_t *) &(op->tris[i]);

	if (Flagged(tp->flags, FLAG_TRI_CLIPPED)) {

		/* do nothing, skip this triangle */
#ifdef DEBUG_EDGES
	    fprintf(stderr,"skipping edges for tri %d, it was clipped\n",i);
#endif

	} else {
#ifdef DEBUG_EDGES
	    fprintf(stderr,"adding edges for tri %d %08x\n",i,tp->flags);
#endif
	    add_edge(op, op->id, i, tp->v0, tp->v1); 
	    add_edge(op, op->id, i, tp->v1, tp->v2); 
	    add_edge(op, op->id, i, tp->v2, tp->v0); 
        }
    }
}

/* we've added all the edges, now go through and find creases, silhouettes, etc */
void
process_obj_edges(Object_t *op)
{
    Vtx_t	*vp;
    Tri_t	*tp, *t0, *t1;
    Edge_t	*ep;
    float	costheta;
    int		i, ecnt;

    ecnt = objEdges[op->id]->num_edges;
    ep = (Edge_t *) objEdges[op->id]->edges;

    vp = (Vtx_t *) op->verts;
    tp = (Tri_t *) op->tris;

	/* the backface culling logic below looks overly complicated because
	 * Renering Plant handles flexibilty of culling FRONT or BACK 
	 * similar ot OpenGL... but basically it is just:
	 *
         *         - if the edge is not shared, don't draw if tri is culled
	 *         - if both tris sharing an edge are culled, don't draw the edge
         *         - if one tri is culled and one not - edge is a silhouette (draw)
	 */

    for (i=0; i<ecnt; i++) {

	if (ep[i].tri0 >= 0) t0 = (Tri_t *) &(tp[ep[i].tri0]); else t0 = (Tri_t *) NULL;
	if (ep[i].tri1 >= 0) t1 = (Tri_t *) &(tp[ep[i].tri1]); else t1 = (Tri_t *) NULL;

	if (t0 == (Tri_t *) NULL || t1 == (Tri_t *) NULL) {	/* edge not shared */

	    if (t0 == (Tri_t *) NULL) t0 = t1;

	    if (Flagged(op->flags, FLAG_CULL_BACK)) {
		if (Flagged(t0->flags, FLAG_CULL_BACK)) {
	            Flag(ep[i].flags, FLAG_CULL_BACK);	/* don't draw */
		} else {
	            Flag(ep[i].flags, FLAG_EDGE_SILHOUETTE);
#ifdef DEBUG_EDGES
	    fprintf(stderr,"silhouette edge {%d}, t0 flags = %08x\n",
			ep[i].id,t0->flags);
#endif
		}
	    }

	    if (Flagged(op->flags, FLAG_CULL_FRONT)) {
		if (Flagged(t0->flags, FLAG_CULL_FRONT)) {
	            Flag(ep[i].flags, FLAG_CULL_FRONT);	/* don't draw */
		} else {
	            UnFlag(ep[i].flags, FLAG_ALL);
	            Flag(ep[i].flags, FLAG_EDGE_SILHOUETTE);
#ifdef DEBUG_EDGES
	    fprintf(stderr,"silhouette edge {%d}, t0 flags = %08x\n",
			ep[i].id, t0->flags);
#endif
		}
	    }
	} else {

	    costheta = vector_dot(t0->normal, t1->normal);

	        /* detect crease (also handles co-planar triangles sharing edge) */
	    if (costheta < CREASE_TOLERANCE) {
	        Flag(ep[i].flags, FLAG_EDGE_CREASE);
#ifdef DEBUG_EDGES
	    fprintf(stderr,"edge {%d} is a crease, t0 = %08x, t1 = %08x %f\n",
			ep[i].id,(int)t0,(int)t1,costheta);
#endif
	    }

		/* shared edge but different materials is a crease */
	    if (t0->material_id != t1->material_id) {
	        Flag(ep[i].flags, FLAG_EDGE_MATERIAL);
#ifdef DEBUG_EDGES
 		fprintf(stderr,"edge {%d} is a crease, shared edge, diff mtl: %d, %d\n",
			ep[i].id,t0->material_id, t1->material_id);
#endif
	    }

	    if (Flagged(op->flags, FLAG_CULL_BACK)) {
			/* both backfacing, don't draw */
		if (Flagged(t0->flags, FLAG_CULL_BACK) &&
		    Flagged(t1->flags, FLAG_CULL_BACK)) {

	            Flag(ep[i].flags, FLAG_CULL_BACK);	/* don't draw */
#ifdef DEBUG_EDGES
	    fprintf(stderr,"both tris BACKFACING, %08x, %08x\n",
			t0->flags,t1->flags);
#endif
		}
			/* one backfacing, one not - is a silhouette */
		if ((Flagged(t0->flags, FLAG_CULL_BACK) &&
		    !Flagged(t1->flags, FLAG_CULL_BACK)) ||
		    (Flagged(t1->flags, FLAG_CULL_BACK) &&
		    !Flagged(t0->flags, FLAG_CULL_BACK))) {

	            Flag(ep[i].flags, FLAG_EDGE_SILHOUETTE);
#ifdef DEBUG_EDGES
	    fprintf(stderr,"edge {%d} is silhouette:\n",
			ep[i].id);
	    fprintf(stderr,"\tone tri BACKFACING, %08x, %08x one not\n",
			t0->flags,t1->flags);
#endif
		}
            }

	    if (Flagged(op->flags, FLAG_CULL_FRONT)) {
			/* both backfacing, don't draw */
		if (Flagged(t0->flags, FLAG_CULL_FRONT) &&
		    Flagged(t1->flags, FLAG_CULL_FRONT)) {

	            Flag(ep[i].flags, FLAG_CULL_FRONT);	/* don't draw */
#ifdef DEBUG_EDGES
	    fprintf(stderr,"both tris BACKFACING, %08x, %08x\n",
			t0->flags,t1->flags);
#endif
		}
			/* one backfacing, one not - is a silhouette */
		if ((Flagged(t0->flags, FLAG_CULL_FRONT) &&
		    !Flagged(t1->flags, FLAG_CULL_FRONT)) ||
		    (Flagged(t1->flags, FLAG_CULL_FRONT) &&
		    !Flagged(t0->flags, FLAG_CULL_FRONT))) {

	            Flag(ep[i].flags, FLAG_EDGE_SILHOUETTE);
#ifdef DEBUG_EDGES
	    fprintf(stderr,"edge {%d} is silhouette:\n",
			ep[i].id);
	    fprintf(stderr,"\tone tri FRONTACING, %08x, %08x one not\n",
			t0->flags,t1->flags);
#endif
		}
            }
        }	/* else, both t0 and t1 non-NULL */
    }	/* for all edges */
}

/*
 * for best results, use FILTER5 (slightly fatter line)
 *
#define FILTER3
 */
#define FILTER5

#ifdef FILTER3
static int filter3[3][3] = 
{
    { 4, 2, 4 },
    { 2, 1, 2 },
    { 4, 2, 4 },
};
#endif

#if defined FILTER3 || defined FILTER5
static int filter5[5][5] = 
{
    {  32,  8,  4,  8,  32 },
    {  16,  4,  2,  4,  16 },
    {   8,  2,  1,  2,   8 },
    {  16,  4,  2,  4,  16 },
    {  32,  8,  4,  8,  32 },
};
#endif

/* simple DDA line function, handles z-buffer and slightly fat line */
static void draw_line(int force, int x0, int y0, int z0, int x1, int y1, int z1, rgba_t color)
{
    float	x, y, z, dx, dy, dz, step;
    int		i=1, tx, ty, tz;
#if (defined FILTER3 || defined FILTER5)
    int		j, k;
    float	zsum, weight;
#endif

    if (y0 > y1) {	/* swap */

        tx = x0; ty = y0; tz = z0;
	x0 = x1; y0 = y1; z0 = z1;
	x1 = tx; y1 = ty; z1 = tz;
    } 

    dx = (x1 - x0); dy = (y1 - y0); dz = (z1 - z0);

    if (fabs(dx) >= fabs(dy))	/* loop over major axis */
	step = fabs(dx);
    else
	step = fabs(dy);

    dx /= step; dy /= step; dz /= step;

    x = x0; y = y0; z = z0;
  
    while (i <= step) {

	tx = (int) (x+0.5); ty = (int) (y+0.5);

#if (defined FILTER3 || defined FILTER5)
	/* rather than use a DEPTH_BIAS as similar algorithms do, we use
	 * our filter kernal to sample z around the point to get a better
	 * picture of occlusion
	 */
	zsum = 0.0;

	for (j=-2; j<3; j++) {
	    for (k=-2; k<3; k++) {
		weight = 32.0/(float)filter5[k+2][j+2];
		zsum += (weight * RPTestDepthFB(tx+k, ty+j, z));
	    }
	}

	zsum /= 32.0;

	if (force) zsum = 1.0;

	if (zsum > 0.5) {
#else
	if (force || RPTestDepthFB(tx, ty, z - (z * DEPTH_BIAS))) {
#endif

		/* we manipulate alhpa for a nice filter effect */
#if (defined FILTER3)
		/* filter 3 gaussian */
	    for (j=-1; j<2; j++) {
	        for (k=-1; k<2; k++) {
	            RPPutColorFBPixel(tx+k, ty+j, 
			color.r, color.g, color.b, color.a/filter3[k+1][j+1]);
	        }
	    }
#elif (defined FILTER5)
		/* filter 5 gaussian */
	    for (j=-2; j<3; j++) {
	        for (k=-2; k<3; k++) {
	            RPPutColorFBPixel(tx+k, ty+j, 
			color.r, color.g, color.b, color.a/filter5[k+2][j+2]);
	        }
	    }
#else
	    RPPutColorFBPixel(tx,   ty,   color.r, color.g, color.b, color.a);
#endif
        }

	x += dx; y += dy; z += dz;
	i++;
    }
}

/* draw all the visible edges for this object */
void
draw_edges(Object_t *op)
{
    rgba_t	black = { 0, 0, 0, 255};
    Vtx_t	*vp;
    Edge_t	*ep;
    int		x0, y0, z0, x1, y1, z1, i, ecnt, force = FALSE;

    ecnt = objEdges[op->id]->num_edges;
    ep = (Edge_t *) objEdges[op->id]->edges;

    vp = (Vtx_t *) op->verts;

    for (i=0; i<ecnt; i++) {

	if (Flagged(ep[i].flags, FLAG_EDGE_SILHOUETTE) ||
	    Flagged(ep[i].flags, FLAG_EDGE_CREASE)     ||
	    Flagged(ep[i].flags, FLAG_EDGE_MATERIAL)) {

	    x0 = vp[ep[i].v0].sx;
	    y0 = vp[ep[i].v0].sy;
	    z0 = vp[ep[i].v0].sz;

	    x1 = vp[ep[i].v1].sx;
	    y1 = vp[ep[i].v1].sy;
	    z1 = vp[ep[i].v1].sz;

	    if (Flagged(ep[i].flags, FLAG_EDGE_MATERIAL))
		force = TRUE;	/* force draw regardless of depth buffer */

#ifdef DEBUG_EDGES
	if (Flagged(ep[i].flags, FLAG_EDGE_SILHOUETTE)) {
	    fprintf(stderr,"drawing silhouette edge {%d}:\t%d, %d\n",
		ep[i].id,ep[i].v0, ep[i].v1);
	    fprintf(stderr,"\t(%d,%d) -- (%d,%d)\n",
		x0,y0,x1,y1);
	}
	if (Flagged(ep[i].flags, FLAG_EDGE_CREASE)) {
	    fprintf(stderr,"drawing crease edge {%d}:\t\t%d, %d\n",
		ep[i].id,ep[i].v0, ep[i].v1);
	    fprintf(stderr,"\t(%d,%d) -- (%d,%d)\n",
		x0,y0,x1,y1);
	}
#endif

            draw_line(force, x0, y0, z0, x1, y1, z1, black);

	    drawn_edges++;
	}
    }
}


