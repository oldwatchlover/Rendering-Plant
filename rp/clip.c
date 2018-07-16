
/*
 * File:	clip.c
 *
 * Cohen-Sutherland clipping algorithm, extended to 3D triangles.
 * If you are familiar with the Cohen-Sutherland line clipping algorithm,
 * this implementation is similar, treating edge edge as a "line" and carefully
 * maintaining polygon structure.
 *
 * This code has a funky style - it was originally written as
 * an algorithm simulation for harware/firmware implementation, so
 * I'm sorry things are unrolled and it looks a little like an assembly
 * program translated to C... :-)
 *
 * Basically 2 parts: 
 *
 *   1) Clip code generation per vertex. This is normally implemented
 *      in the CPU with a special instruction. This information is all you
 *      need to clip points; it's most of what you need to clip lines, and
 *      it is just a start if you want to clip polygons.
 *
 *   2) RPClipTriangle() : clips a triangle (in projected space) against the
 *      view frustrum defined by a = -w and a = +w, where a is one of the
 *      x,y,z planes.
 *
 *      Note that for Z, we do not do a complete clip. Far plane (+z) clipping
 *      is useful in video games to reduce complex geometry on the horizon, but
 *      practically speaking other techniques are better at that.
 *
 *      -z clipping is very important because depending on the visible surface
 *      technique used, objects/portions of objects "behind the eye" can project
 *      to erroneously appear in front of it. We actually clip to (-z+Epsilon),
 *      moving the near clip plane forward just a bit to avoid nasty math problems
 *      when divide by zero can happen. 
 *
 * Note about the coordinate space:
 *
 * As mentioned briefly above, clipping must happen in "projected space", that
 * is after the model is transformed to world space (by MODEL matrix) and then
 * transformed to eye or camera space (by VIEW matrix), and then projected by 
 * the PROJECTION matrix.
 *
 * Clipping occurs in PROJECTION space, before the screen coordinates are 
 * generated by dividing by w. This is necessary to simplify the w plane tests 
 * and interpolation of attributes. 
 *
 * Therefore there must be close coordination between this code and the graphics
 * pipeline transformation code. The code near the bottom of RPClipTriangle() that 
 * finishes generating any new clipped geometry must match the perspective 
 * division and viewport mapping used elsewhere in the pipeline.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "rp.h"

#define		MASK_NEG_X	0x01
#define		MASK_POS_X	0x02
#define		MASK_NEG_Y	0x04
#define		MASK_POS_Y	0x08
#define		MASK_NEG_Z	0x10
#define		MASK_POS_Z	0x20

    /* as we clip each plane, we will alternate using these temp
     * buffers to output the points for that plane. After clipping
     * all planes in succession, final clipped vertices will be in temp1
     *
     * A triangle intersecting a box can intersect in at most 9 points,
     * if the triangle and box are sized and positioned appropriately.
     * (mathematical proof left as an exercise for the reader to google...)
     *
     */
#define 	MAX_NEW_VERTS	9
static Vtx_t	temp0[MAX_NEW_VERTS];
static Vtx_t	temp1[MAX_NEW_VERTS];

/* takes a 3D vertex and outputs a 6 bit clip code againt the view frustrum */
u8
RPGenerateVertexClipcodes(xyz_t *v, float w)
{   /*
     * Clip codes are packed as follows:
     *
     *   Bit 0:  On if x <= -w
     *   Bit 1:  On if x >= w
     *   Bit 2:  On if y <= -w
     *   Bit 3:  On if y >= w
     *   Bit 4:  On if z <= -w
     *   Bit 5:  On if z >= w
     */
    u8		cc = 0x0;

    if (v->x <= -w) cc |= MASK_NEG_X;
    if (v->x >=  w) cc |= MASK_POS_X;
    if (v->y <= -w) cc |= MASK_NEG_Y;
    if (v->y >=  w) cc |= MASK_POS_Y;
    if (v->z <= -(w+Epsilon)) cc |= MASK_NEG_Z;
	/* skip far plane clipping for now... */

    return (cc);
}

/* calc the d parameter, to find intersection of an edge for this plane: */
static float
calc_d(float plusminus, float inplane, float outplane, float inw, float outw)
{
    float	wi, wo, pi, po, d;

    wi = plusminus * inw;
    wo = plusminus * outw;
    pi = inplane;
    po = outplane;
    d = (pi - wi) - (po - wo);
    if (NearlyZero(d, Epsilon))
  	d = 0.0;
    else
	d = (pi - wi) / d;

    return (d);
}

/* interpolate all the attributes of a vertex */
static void 
plerp_all(float d, Vtx_t *v1, Vtx_t *v2, Vtx_t *out)
{
/* linear interpolation of a float parameter */
#define plerp(d, in, out)	((((out) - (in)) * (d)) + (in))

    out->pos.x  = plerp(d, v1->pos.x,  v2->pos.x);
    out->pos.y  = plerp(d, v1->pos.y,  v2->pos.y);
    out->pos.z  = plerp(d, v1->pos.z,  v2->pos.z);
    out->proj.x = plerp(d, v1->proj.x, v2->proj.x);
    out->proj.y = plerp(d, v1->proj.y, v2->proj.y);
    out->proj.z = plerp(d, v1->proj.z, v2->proj.z);
    out->n.x    = plerp(d, v1->n.x,    v2->n.x);
    out->n.y    = plerp(d, v1->n.y,    v2->n.y);
    out->n.z    = plerp(d, v1->n.z,    v2->n.z);
    out->w      = plerp(d, v1->w,      v2->w);
    out->r      = plerp(d, v1->r,      v2->r);
    out->g      = plerp(d, v1->g,      v2->g);
    out->b      = plerp(d, v1->b,      v2->b);
    out->a      = plerp(d, v1->a,      v2->a);
    out->s      = plerp(d, v1->s,      v2->s);
    out->t      = plerp(d, v1->t,      v2->t);

#undef plerp
}

/* Output one (clipped) triangle. This function assumes the output
 * buffer has been allocated - in our implementation, we use realloc() to extend
 * the objects vertex and triangle buffers and add them to the end.
 * 
 * The flaggs FLAG_TRI_CLIPPED and FLAG_TRI_CLIP_GEN are used in here and 
 * elsewhere in the renderer to identify clipped geometry and possibly handle 
 * them differently. FLAG_TRI_CLIPPED is an optimization that allows us
 * to skip rendering those triangles sine thier visible portions are
 * duplicated by the new clipped triangles. FLAG_TRI_CLIP_GEN is an optimization
 * that allows us to skip clipping for those triangles since we know they
 * were generated by the clipper, they are by definition trivially accepted.
 *
 */
static void
add_clipped_tri(Object_t *op, Tri_t *parenttri, Tri_t *newtri, int v0, int v1, int v2)
{
	/* copy parent tri to inherit most attributes: */
    bcopy(parenttri, newtri, sizeof(Tri_t));
    newtri->v0 = v0;
    newtri->v1 = v1;
    newtri->v2 = v2;
    RPProcessOneTriangle(op, newtri); /* compute normal, intersection parameters */
    UnFlag(newtri->flags, FLAG_TRI_CLIPPED);	/* disable this from parent */
    Flag(newtri->flags, FLAG_TRI_CLIP_GEN);	/* tri was generated by clip */
    op->tri_count++;
}

/*
 * clip a triangle to the view frustrum
 *
 * On input, each vertex has had a clipcode generated for it.
 *
 * Returns:
 *	CLIP_TRIVIAL_REJECT	0
 *	CLIP_TRIVIAL_ACCEPT	1
 *
 * 	or it returns the number of triangles added as a result of the clip.
 *
 * New vertices and triangles created are added to the end of the 
 * geometry lists in the object structure (and the counts are incremented).
 *
 * Adding new vertices isn't a problem for a one-way graphics pipeline
 * (most GPU style renderers) however it can bloat memory and complicate 
 * processing for other software algorithms (like scanline and ray tracers). 
 * Be careful.
 *
 * We use the flag FLAG_TRI_CLIPPED to identify "original" triangles that 
 * were clipped, and we don't draw them; we draw the new triangles generated
 * by the clip operation instead.
 *
 */
int 
RPClipTriangle(Object_t *op, Tri_t *tri)
{
    Vtx_t	*vp = op->verts;
    Tri_t	*tp;
    xyz_t	t;
    int		cc_clip, cc_rej, s, n, i, in, out, last, v1, v2, v3;
    int		new_tris = 0;
    float	d;

    /* if this poly was generated from a previous clip op, we know it's good */
    if (Flagged(tri->flags, FLAG_TRI_CLIP_GEN))
	return (CLIP_TRIVIAL_ACCEPT);

    v1 = tri->v0; v2 = tri->v1; v3 = tri->v2;

    /* test for trivial accept or trivial reject: */
    cc_clip  = vp[v1].cc; cc_clip |= vp[v2].cc; cc_clip |= vp[v3].cc;
    cc_rej  = vp[v1].cc; cc_rej &= vp[v2].cc; cc_rej &= vp[v3].cc;

    if (cc_clip == 0) {		/* entirely within view */
	return (CLIP_TRIVIAL_ACCEPT);
    }
    
    if (cc_rej != 0) {		/* entirely out of view */
	RPScene.trivial_rejected_polys++;
	return (CLIP_TRIVIAL_REJECT);
    }

    /* this triangle gets clipped, go for it... */

    RPScene.clipped_polys++;

    /* copy points to temp buffer */
    bcopy((void *)&(vp[v1]), (void *)&(temp0[0]), sizeof(Vtx_t));
    bcopy((void *)&(vp[v2]), (void *)&(temp0[1]), sizeof(Vtx_t));
    bcopy((void *)&(vp[v3]), (void *)&(temp0[2]), sizeof(Vtx_t));

	/* if we are going to rasterize perspective-correct textures,
         * temporarily un-do that while we interpolate the s,t during 
         * clip, then re-divide after the clip below
	 * (we know from vertex.c that inv_w can't be 0.0, so no check)
	 */
    if (Flagged(RPScene.flags, FLAG_PERSP_TEXTURE)) {
            temp0[0].s /= temp0[0].inv_w;
            temp0[0].t /= temp0[0].inv_w;
            temp0[1].s /= temp0[1].inv_w;
            temp0[1].t /= temp0[1].inv_w;
            temp0[2].s /= temp0[2].inv_w;
            temp0[2].t /= temp0[2].inv_w;
    }

/* this macro sets the buffer pointers, called for each edge test
 * for each of the clipping planes below */
#define CLIP_IN_TO_OUT(VBUFF, MASK) \
	if (VBUFF[i].cc & MASK) {in=last;out=i;} else {in=i;out=last;}

   	    /* -x clip: from temp0 to temp1 */
    n = 3;	    /* num of points to clip against this plane */
    s = 0;	    /* num of points after clipping against this plane */
    last = n - 1;   /* previous vertex to create the edge w/current vertex */

    for (i=0; i<n; i++) {
        if ((temp0[i].cc & MASK_NEG_X) != (temp0[last].cc & MASK_NEG_X)) { 

	    CLIP_IN_TO_OUT(temp0, MASK_NEG_X);

	    d = calc_d(-1.0, temp0[in].proj.x, temp0[out].proj.x,
			     temp0[in].w, temp0[out].w);

            /* lerp to get new point and attributes */
	    plerp_all(d, &(temp0[in]), &(temp0[out]), &(temp1[s]));
            temp1[s].cc = RPGenerateVertexClipcodes(&(temp1[s].proj), temp1[s].w);
            s++; 		/* output this new point */
        }
        if (!(temp0[i].cc & MASK_NEG_X)) {  /* original point inside, output it */
          bcopy((void *)&(temp0[i]), (void *)&(temp1[s]), sizeof(Vtx_t));
          s++;
	}
        last = i;
    }

    n = s; s = 0; last = n - 1; /* +x clip: from temp1 to temp0 */
    for (i=0; i<n; i++) {
        if ((temp1[i].cc & MASK_POS_X) != (temp1[last].cc & MASK_POS_X)) {

	    CLIP_IN_TO_OUT(temp1, MASK_POS_X);

	    d = calc_d(1.0, temp1[in].proj.x, temp1[out].proj.x,
			     temp1[in].w, temp1[out].w);

            /* lerp to get new point */
	    plerp_all(d, &(temp1[in]), &(temp1[out]), &(temp0[s]));
            temp0[s].cc = RPGenerateVertexClipcodes(&(temp0[s].proj), temp0[s].w);
            s++; 	/* output this new point */
        }
        if (!(temp1[i].cc & MASK_POS_X)) {  /* original point inside, output it */
            bcopy((void *)&(temp1[i]), (void *)&(temp0[s]), sizeof(Vtx_t));
            s++;
        }
        last = i;
    }

    n = s; s = 0; last = n - 1; /* -y clip: from temp0 to temp1 */
    for (i=0; i<n; i++) {
        if ((temp0[i].cc & MASK_NEG_Y) != (temp0[last].cc & MASK_NEG_Y)) {

	    CLIP_IN_TO_OUT(temp0, MASK_NEG_Y);

	    d = calc_d(-1.0, temp0[in].proj.y, temp0[out].proj.y,
			     temp0[in].w, temp0[out].w);

            /* lerp to get new point */
	    plerp_all(d, &(temp0[in]), &(temp0[out]), &(temp1[s]));
            temp1[s].cc = RPGenerateVertexClipcodes(&(temp1[s].proj), temp1[s].w);
            s++; 	/* output this new point */
        }
        if (!(temp0[i].cc & MASK_NEG_Y)) {  /* original point inside, output it */
            bcopy((void *)&(temp0[i]), (void *)&(temp1[s]), sizeof(Vtx_t));
            s++;
        }
        last = i;
    }

    n = s; s = 0; last = n - 1; /* +y clip: from temp1 to temp0 */
    for (i=0; i<n; i++) {
        if ((temp1[i].cc & MASK_POS_Y) != (temp1[last].cc & MASK_POS_Y)) {

	    CLIP_IN_TO_OUT(temp1, MASK_POS_Y);

	    d = calc_d(1.0, temp1[in].proj.y, temp1[out].proj.y,
			     temp1[in].w, temp1[out].w);

            /* lerp to get new point */
	    plerp_all(d, &(temp1[in]), &(temp1[out]), &(temp0[s]));
            temp0[s].cc = RPGenerateVertexClipcodes(&(temp0[s].proj), temp0[s].w);
            s++; 	/* output this new point */
        }
        if (!(temp1[i].cc & MASK_POS_Y)) {  /* original point inside, output it */
            bcopy((void *)&(temp1[i]), (void *)&(temp0[s]), sizeof(Vtx_t));
            s++;
        }
        last = i;
    }

    n = s; s = 0; last = n - 1; /* -z clip: from temp0 to temp 1 */
    for (i=0; i<n; i++) {
        if ((temp0[i].cc & MASK_NEG_Z) != (temp0[last].cc & MASK_NEG_Z)) {

	    CLIP_IN_TO_OUT(temp0, MASK_NEG_Z);

	    d = calc_d(-1.0, temp0[in].proj.z, temp0[out].proj.z,
			     temp0[in].w, temp0[out].w);

            /* lerp to get new point */
	    plerp_all(d, &(temp0[in]), &(temp0[out]), &(temp1[s]));
            temp1[s].cc = RPGenerateVertexClipcodes(&(temp1[s].proj), temp1[s].w);
            s++; 	/* output this new point */
        }
        if (!(temp0[i].cc & MASK_NEG_Z)) {  /* original point inside, output it */
            bcopy((void *)&(temp0[i]), (void *)&(temp1[s]), sizeof(Vtx_t));
            s++;
        }
        last = i;
    }

    n = s; 	/* skip +z clip... all clipped vertices are now in array temp1 */
    		/* project, then viewport scale + translate */

    for (i=0; i<n; i++) {
                /* project */
        if (NearlyZero(temp1[i].w,Epsilon)) {
            temp1[i].inv_w = 1.0;
        } else {
            temp1[i].inv_w = 1.0/temp1[i].w;
        }
        t.x = temp1[i].proj.x * temp1[i].inv_w;
        t.y = temp1[i].proj.y * temp1[i].inv_w;
        t.z = temp1[i].proj.z * temp1[i].inv_w;

	/* if we are going to rasterize perspective-correct textures,
         * then divide s,t by w for the rasterizer
	 */
        if (Flagged(RPScene.flags, FLAG_PERSP_TEXTURE)) {
            temp1[i].s *= temp1[i].inv_w;
            temp1[i].t *= temp1[i].inv_w;
        }
		/* flip y due to output BMP file orientation */
        temp1[i].sx = (t.x *  RPScene.viewport->sx) + RPScene.viewport->tx;
        temp1[i].sy = (t.y * -RPScene.viewport->sy) + RPScene.viewport->ty;
        temp1[i].sz = (t.z *  RPScene.viewport->sz) + RPScene.viewport->tz;
    }

    /* all done, now output new triangles stored in temp1: */

    if ((n < 3) || (n > MAX_NEW_VERTS)) {
	fprintf(stderr,"%s : ERROR : %s : CLIP : n = %d!\n",
		program_name,__FILE__,n);
	RPScene.trivial_rejected_polys++;
	return (CLIP_TRIVIAL_REJECT);
    }

	/* extend op->verts and op->tris with realloc */
    op->verts = (Vtx_t *) realloc(op->verts, (op->vert_count+n) * sizeof(Vtx_t));
    bcopy(&(temp1[0]), &(op->verts[op->vert_count]), n*sizeof(Vtx_t));

    if (Flagged(RPScene.flags, FLAG_VERBOSE2))
        fprintf(stderr,"clip : outputs %d vertices, %d triangles\n",
		n, (n-2));
    /* 
     * output new triangles
     * see how many points we ended up with and knit them
     * together as a triangle fan in the original order.
     */
    s = op->vert_count;
    new_tris = n - 2;
    op->tris = (Tri_t *) realloc(op->tris, (op->tri_count+new_tris) * sizeof(Tri_t));
    tp = &(op->tris[op->tri_count]);
    for (i=2; i<n; i++) {
        add_clipped_tri(op, tri, tp, s+0, s+(i-1), s+i);
	tp++;
    }
    op->vert_count += n;

	/* mark the original triangle as having been clipped */
    Flag(tri->flags, FLAG_TRI_CLIPPED);

    return (new_tris);
}

