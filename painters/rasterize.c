
/*
 * File:	rasterize.c
 *
 * Triangle setup and rasterization. The algorithm uses the plane equation to compute
 * attributes per pixel.
 *
 * This polygon rasterize code was originally part of an "algorithm simulator"
 * used to develop GPU hardware, firmware and drivers.
 *
 * Accordingly, this code is really low-level, reading like assembly language,
 * with register-level operations (and variable naming that you can easily
 * imagine being converted to VHDL or GPU firmware. 
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
#include <math.h>
#include <string.h>

#include "rp.h"
#include "paint.h"

static void bary_tri_setup(Object_t *op, Tri_t *tri, 
			Vtx_t *p0, Vtx_t *p1, Vtx_t *p2, int usecfb);

/* compute attribute dx and dy from plane equation */
static void
pleq_dxdy(float inv_r, int ax, int ay, int bx, int by, float ac, float bc, 
	  float *cx, float *cy)
{
    *cx = ((float)by * ac) - ((float)ay * bc);
    *cy = ((float)ax * bc) - ((float)bx * ac);

    *cx *= inv_r;
    *cy *= inv_r;
}

/*
 * paint this triangle
 *
 * If usecfb == FALSE, then just update the depth buffer, not the
 * color frame buffer. (this is used by the hidden line renderer)
 *
 */
void
paint_tri(Object_t *op, Tri_t *tri, int usecfb)
{
    Vtx_t	*p0, *p1, *p2;

	/*
	 * if this triangle was clipped, then we don't want to draw it.
	 * We know the clip process generated new triangles for the visible
	 * portions, so drawing this one would be redundant (and potentially
	 * mathematically error-prone)
	 */
    if (Flagged(tri->flags, FLAG_TRI_CLIPPED))
	return;

    p0 = &(op->verts[tri->v0]);
    p1 = &(op->verts[tri->v1]);
    p2 = &(op->verts[tri->v2]);

    bary_tri_setup(op, tri, p0, p1, p2, usecfb);
}


/* setup and rasterize a triangle */

/* rasterizer variable naming conventions:
 *
 * "H" is the high edge, the long edge of the triangle that goes from top to bottom y.
 * "M" is the upper edge on the side with a vertex between top and bottom y. (minor side)
 * "L" is the lower edge on the side with a vertex between top and bottom y. (minor side)
 * "DxD*" are the deltas to walk across in the x direction for each scanline.
 * "DyD*" are the deltas to increment scanline to scanline (y direction) for each attribute)
 *
 */
static void
bary_tri_setup(Object_t *op, Tri_t *tri, Vtx_t *ip0, Vtx_t *ip1, Vtx_t *ip2, int usecfb)
{
    Material_t	*m;
    Vtx_t	*tmpp, tmp_buffer, point0, point1, point2, *p0, *p1, *p2;
    rgba_t	tex_samp;
    int		ydelh, ydelm, ydell, x, y;
    int		Hdx, Hdy, Mdx, Mdy;
    float	dhdy = 0.0, dmdy = 0.0, dldy = 0.0, xminor, xhigh, r, inv_r;
    Colorf_t	colorsum, polycolor;
    Colorf_t	Hdcol, Mdcol, DxDcol, DyDcol, thiscolor;
    float	Hds, Mds, DxDs, DyDs, thiss;
    float	Hdt, Mdt, DxDt, DyDt, thist;
    float	Hdw, Mdw, DxDw, DyDw, thisw;
    float	Hdz, Mdz, DxDz, DyDz, thisz;
    xyz_t	thispoint;
    xyz_t	Hdsurf, Mdsurf, DxDsurf, DyDsurf, thissurf;
    xyz_t	Hdnorm, Mdnorm, DxDnorm, DyDnorm, thisn;
    xyz_t	Hdeye, Mdeye, DxDeye, DyDeye, thiseye;


	/* copy the input points becuase they are shared and we may modify: */
    bcopy((void *) ip0, &point0, sizeof(Vtx_t)); p0 = &point0;
    bcopy((void *) ip1, &point1, sizeof(Vtx_t)); p1 = &point1;
    bcopy((void *) ip2, &point2, sizeof(Vtx_t)); p2 = &point2;

    	/* edge deltas: */
    Mdx = p1->sx - p0->sx;          Mdy = p1->sy - p0->sy;
    Hdx = p2->sx - p0->sx;          Hdy = p2->sy - p0->sy;
    
    r = Hdx*Mdy - Hdy*Mdx;	/* denom of plane equation */
 
    	/* reject degenerate triangles: */
    if (NearlyZero(r, EpEpsilon)) {
	RPScene.tiny_rejected_polys++;
	return;
    }

    	/* cull backfacing: */
    if (Flagged(op->flags, FLAG_CULL_BACK) && r < 0.0) {
	culled_polys++;
	return;
    }

    	/* cull frontfacing: */
    if (Flagged(op->flags, FLAG_CULL_FRONT) && r > 0.0) {
	culled_polys++;
	return;
    }
	/* choose base pixel color...
 	 * 
         * either MATERIAL, RANDSHADE, or POLYSHADE
	 *
	 * (cannot combine these terms for now)
	 *
	 */

    m = &(op->materials[tri->material_id]);
    polycolor.r = m->color.r; 	/* start with material color: */
    polycolor.g = m->color.g;
    polycolor.b = m->color.b;
    polycolor.a = m->color.a;

    if (Flagged(op->flags, FLAG_RANDSHADE)) {
	/* useful for debugging */
	polycolor.r = RPRandom();
	polycolor.g = RPRandom();
	polycolor.b = RPRandom();
	polycolor.a = 1.0;
    } else if (Flagged(op->flags, FLAG_POLYSHADE)) {
	polycolor.r = tri->color.r;
	polycolor.g = tri->color.g;
	polycolor.b = tri->color.b;
	polycolor.a = tri->color.a;
    } else if (Flagged(op->flags, FLAG_VERTSHADE)) {
	/* do nothing, rasterizer loop will interp vertex colors */
    }

	/* fix texture coordinates, we want to rasterize in perspected space: */
    if (Flagged(tri->flags, FLAG_TRI_CLIP_GEN)) {
		/* texture coord already in s,t */
	p0->s *= p0->inv_w;
	p0->t *= p0->inv_w;
	p1->s *= p1->inv_w;
	p1->t *= p1->inv_w;
	p2->s *= p2->inv_w;
	p2->t *= p2->inv_w;
    } else {
        if (op->tcoords != (uv_t *) NULL) {
            p0->s = op->tcoords[tri->t0].u * p0->inv_w;
            p0->t = op->tcoords[tri->t0].v * p0->inv_w;
            p1->s = op->tcoords[tri->t1].u * p1->inv_w;
            p1->t = op->tcoords[tri->t1].v * p1->inv_w;
            p2->s = op->tcoords[tri->t2].u * p2->inv_w;
            p2->t = op->tcoords[tri->t2].v * p2->inv_w;
        }
    }

    	/* y-sort the 3 vertices of the triangle: */
    tmpp = &(tmp_buffer);
    if (p0->sy > p1->sy) {
	tmpp = p0; p0 = p1; p1 = tmpp;
    }
    if (p0->sy > p2->sy) {
	tmpp = p0; p0 = p2; p2 = tmpp;
    }
    if (p1->sy > p2->sy) {
	tmpp = p1; p1 = p2; p2 = tmpp;
    }

    	/* edge deltas: */
    Mdx = p1->sx - p0->sx;          Mdy = p1->sy - p0->sy;
    Hdx = p2->sx - p0->sx;          Hdy = p2->sy - p0->sy;
    r = Hdx*Mdy - Hdy*Mdx;
    inv_r = 1.0/r;

    	/* edge slopes: */
    ydelh = p2->sy - p0->sy;
    ydelm = p1->sy - p0->sy;
    ydell = p2->sy - p1->sy;

    if (ydelh != 0) dhdy = (float)(p2->sx - p0->sx)/(float) ydelh;
    if (ydelm != 0) dmdy = (float)(p1->sx - p0->sx)/(float) ydelm;
    if (ydell != 0) dldy = (float)(p2->sx - p1->sx)/(float) ydell;

    	/* attribute deltas: */
    Hdcol.r = p2->r - p0->r;   		Mdcol.r = p1->r - p0->r;
    Hdcol.g = p2->g - p0->g;   		Mdcol.g = p1->g - p0->g;
    Hdcol.b = p2->b - p0->b;   		Mdcol.b = p1->b - p0->b;
    Hdcol.a = p2->a - p0->a;   		Mdcol.a = p1->a - p0->a;

    Hds = p2->s - p0->s;    		Mds = p1->s - p0->s;
    Hdt = p2->t - p0->t;    		Mdt = p1->t - p0->t;
    Hdw = p2->inv_w - p0->inv_w;    	Mdw = p1->inv_w - p0->inv_w;
    Hdz = p2->sz - p0->sz;  		Mdz = p1->sz - p0->sz;
    Hdsurf.x = p2->pos.x - p0->pos.x;	Mdsurf.x = p1->pos.x - p0->pos.x;
    Hdsurf.y = p2->pos.y - p0->pos.y;	Mdsurf.y = p1->pos.y - p0->pos.y;
    Hdsurf.z = p2->pos.z - p0->pos.z;	Mdsurf.z = p1->pos.z - p0->pos.z;

    Hdnorm.x = p2->n.x - p0->n.x;  	Mdnorm.x = p1->n.x - p0->n.x;
    Hdnorm.y = p2->n.y - p0->n.y;  	Mdnorm.y = p1->n.y - p0->n.y;
    Hdnorm.z = p2->n.z - p0->n.z;  	Mdnorm.z = p1->n.z - p0->n.z;

    Hdeye.x = p2->e.x - p0->e.x;  	Mdeye.x = p1->e.x - p0->e.x;
    Hdeye.y = p2->e.y - p0->e.y;  	Mdeye.y = p1->e.y - p0->e.y;
    Hdeye.z = p2->e.z - p0->e.z;  	Mdeye.z = p1->e.z - p0->e.z;

    	/* attribute slopes: */
    pleq_dxdy(inv_r, Hdx, Hdy, Mdx, Mdy, Hdcol.r, Mdcol.r, &DxDcol.r, &DyDcol.r);
    pleq_dxdy(inv_r, Hdx, Hdy, Mdx, Mdy, Hdcol.g, Mdcol.g, &DxDcol.g, &DyDcol.g);
    pleq_dxdy(inv_r, Hdx, Hdy, Mdx, Mdy, Hdcol.b, Mdcol.b, &DxDcol.b, &DyDcol.b);
    pleq_dxdy(inv_r, Hdx, Hdy, Mdx, Mdy, Hdcol.a, Mdcol.a, &DxDcol.a, &DyDcol.a);
    pleq_dxdy(inv_r, Hdx, Hdy, Mdx, Mdy, Hds, Mds, &DxDs, &DyDs);
    pleq_dxdy(inv_r, Hdx, Hdy, Mdx, Mdy, Hdt, Mdt, &DxDt, &DyDt);
    pleq_dxdy(inv_r, Hdx, Hdy, Mdx, Mdy, Hdw, Mdw, &DxDw, &DyDw);
    pleq_dxdy(inv_r, Hdx, Hdy, Mdx, Mdy, Hdz, Mdz, &DxDz, &DyDz);

    pleq_dxdy(inv_r, Hdx, Hdy, Mdx, Mdy, Hdsurf.x, Mdsurf.x, &DxDsurf.x, &DyDsurf.x);
    pleq_dxdy(inv_r, Hdx, Hdy, Mdx, Mdy, Hdsurf.y, Mdsurf.y, &DxDsurf.y, &DyDsurf.y);
    pleq_dxdy(inv_r, Hdx, Hdy, Mdx, Mdy, Hdsurf.z, Mdsurf.z, &DxDsurf.z, &DyDsurf.z);

    pleq_dxdy(inv_r, Hdx, Hdy, Mdx, Mdy, Hdnorm.x, Mdnorm.x, &DxDnorm.x, &DyDnorm.x);
    pleq_dxdy(inv_r, Hdx, Hdy, Mdx, Mdy, Hdnorm.y, Mdnorm.y, &DxDnorm.y, &DyDnorm.y);
    pleq_dxdy(inv_r, Hdx, Hdy, Mdx, Mdy, Hdnorm.z, Mdnorm.z, &DxDnorm.z, &DyDnorm.z);

    pleq_dxdy(inv_r, Hdx, Hdy, Mdx, Mdy, Hdeye.x, Mdeye.x, &DxDeye.x, &DyDeye.x);
    pleq_dxdy(inv_r, Hdx, Hdy, Mdx, Mdy, Hdeye.y, Mdeye.y, &DxDeye.y, &DyDeye.y);
    pleq_dxdy(inv_r, Hdx, Hdy, Mdx, Mdy, Hdeye.z, Mdeye.z, &DxDeye.z, &DyDeye.z);

    y = p0->sy;
    xhigh = (float)p0->sx;

    if (y == p1->sy)		/* degenerate case */
	xminor = (float)p1->sx;
    else 
	xminor = (float)p0->sx;


	/* rasterize from ymin to ymax: */
    while (y <= p2->sy) {

	x = (int)xhigh;

	do { 		/* walk the x's */
	    /* barycentric evaluation of vertex color, normal, and texcoord: */
	    thiscolor.r = p0->r     + (x - p0->sx)*DxDcol.r  + (y - p0->sy)*DyDcol.r;
	    thiscolor.g = p0->g     + (x - p0->sx)*DxDcol.g  + (y - p0->sy)*DyDcol.g;
	    thiscolor.b = p0->b     + (x - p0->sx)*DxDcol.b  + (y - p0->sy)*DyDcol.b;
	    thiscolor.a = p0->a     + (x - p0->sx)*DxDcol.a  + (y - p0->sy)*DyDcol.a;
	    thiss       = p0->s     + (x - p0->sx)*DxDs      + (y - p0->sy)*DyDs;
	    thist       = p0->t     + (x - p0->sx)*DxDt      + (y - p0->sy)*DyDt;
	    thisw       = p0->inv_w + (x - p0->sx)*DxDw      + (y - p0->sy)*DyDw;
	    thisz       = p0->sz    + (x - p0->sx)*DxDz      + (y - p0->sy)*DyDz;
	    thissurf.x  = p0->pos.x + (x - p0->sx)*DxDsurf.x + (y - p0->sy)*DyDsurf.x; 
	    thissurf.y  = p0->pos.y + (x - p0->sx)*DxDsurf.y + (y - p0->sy)*DyDsurf.y; 
	    thissurf.z  = p0->pos.z + (x - p0->sx)*DxDsurf.z + (y - p0->sy)*DyDsurf.z; 

	    if (NearlyZero(thisw, Epsilon)) {
		thispoint.x = p0->sx;
		thispoint.y = p0->sy;
		thispoint.z = p0->sz;
  	    } else {
	        thispoint.x = p0->sx * thisw;
	        thispoint.y = p0->sy * thisw;
		thispoint.z = p0->sz * thisw;
	    }

	    if (Flagged(op->flags, FLAG_FLATSHADE)) {
		thisn.x = tri->normal.x;
		thisn.y = tri->normal.y;
		thisn.z = tri->normal.z;
	    } else {
	        thisn.x = p0->n.x + (x - p0->sx)*DxDnorm.x + (y - p0->sy)*DyDnorm.x;
		thisn.y = p0->n.y + (x - p0->sx)*DxDnorm.y + (y - p0->sy)*DyDnorm.y;
		thisn.z = p0->n.z + (x - p0->sx)*DxDnorm.z + (y - p0->sy)*DyDnorm.z;
	    }

	    thiseye.x = p0->e.x + (x - p0->sx)*DxDeye.x + (y - p0->sy)*DyDeye.x;
	    thiseye.y = p0->e.y + (x - p0->sx)*DxDeye.y + (y - p0->sy)*DyDeye.y;
	    thiseye.z = p0->e.z + (x - p0->sx)*DxDeye.z + (y - p0->sy)*DyDeye.z;

	    if (Flagged(op->flags, FLAG_VERTSHADE)) {	
		colorsum.r = thiscolor.r;
		colorsum.g = thiscolor.g;
		colorsum.b = thiscolor.b;
		colorsum.a = thiscolor.a;
	    } else {
		colorsum.r = polycolor.r;
		colorsum.g = polycolor.g;
		colorsum.b = polycolor.b;
		colorsum.a = polycolor.a;
	    }

	    if (m->texture[MATERIAL_COLOR] != (Texture_t *) NULL) {
		Colorf_t	texcolor;

		if (Flagged(m->texture[MATERIAL_COLOR]->flags, FLAG_TXT_FILT)) {
		    tex_samp = RPFilterSampleTexture(m->texture[MATERIAL_COLOR],
						   thiss, thist, thisw,
						   DxDs, DyDs, DxDt, DyDt,
						   DxDw, DyDw);
		} else {
		    tex_samp = RPPointSampleTexture(m->texture[MATERIAL_COLOR],
						    thiss, thist, thisw);
		}

		texcolor.r = (float)tex_samp.r / MAX_COLOR_VAL;
		texcolor.g = (float)tex_samp.g / MAX_COLOR_VAL;
		texcolor.b = (float)tex_samp.b / MAX_COLOR_VAL;
		texcolor.a = (float)tex_samp.a / MAX_COLOR_VAL;

		if (Flagged(m->texture[MATERIAL_COLOR]->flags, FLAG_TXT_MODULATE)) {
		    colorsum.r *= texcolor.r;
		    colorsum.g *= texcolor.g;
		    colorsum.b *= texcolor.b;
		    colorsum.a *= texcolor.a;
		} else {	/* decal, replace vert/poly color with tex color */
		    colorsum.r = texcolor.r;
		    colorsum.g = texcolor.g;
		    colorsum.b = texcolor.b;
		    colorsum.a = texcolor.a;
		}
 	    }

	    if (Flagged(op->flags, FLAG_LIGHTING)) {
		Colorf_t	shadeval;

		shade_pixel(op, tri, &thisn, &thissurf, &thiseye, &shadeval);

		/* mult colorsum by shade values */
		colorsum.r *= shadeval.r;
		colorsum.g *= shadeval.g;
		colorsum.b *= shadeval.b;
		colorsum.a *= shadeval.a;
  	    }

		/* we interpolate the world space z (not quite correct) so
		 * we can simplify the fog calculations matching the librp
		 * math... that's why we are using thissurf.z (fog z) instead of
		 * thisz, which we use for the zbuffer.
		 */
	    if (Flagged(RPScene.flags, FLAG_FOG) && thissurf.z < RPScene.fog_start) {
                float           f;
                Colorf_t        new;

                if (thissurf.z < RPScene.fog_end) {
                    new.r = RPScene.fog_color.r;
                    new.g = RPScene.fog_color.g;
                    new.b = RPScene.fog_color.b;
                } else {
                    f = (thissurf.z - RPScene.fog_start) / 
			(RPScene.fog_end - RPScene.fog_start);
                    new.r = f * RPScene.fog_color.r + (1.0 - f) * colorsum.r;
                    new.g = f * RPScene.fog_color.g + (1.0 - f) * colorsum.g;
                    new.b = f * RPScene.fog_color.b + (1.0 - f) * colorsum.b;
                }

                colorsum.r = new.r;
                colorsum.g = new.g;
                colorsum.b = new.b;
	    }

	    thiscolor.r = Clamp0255(colorsum.r * MAX_COLOR_VAL);
	    thiscolor.g = Clamp0255(colorsum.g * MAX_COLOR_VAL);
	    thiscolor.b = Clamp0255(colorsum.b * MAX_COLOR_VAL);
	    thiscolor.a = Clamp0255(colorsum.a * MAX_COLOR_VAL);

	    if (usecfb) {

	        if (Flagged(RPScene.flags, FLAG_ZBUFFER)) {
		    if (RPTestDepthFB(x, y, thisz)) {
		        RPPutDepthFBPixel(x, y, thisz);
		        RPPutColorFBPixel(x, y, (int)thiscolor.r, (int)thiscolor.g,
				  (int)thiscolor.b, (int)thiscolor.a);
		    }
	        } else {
		    RPPutColorFBPixel(x, y, (int)thiscolor.r, (int)thiscolor.g,
			      (int)thiscolor.b, (int)thiscolor.a);
	        }

	    } else { 	/* write zbuffer only */

		if (RPTestDepthFB(x, y, thisz)) {
		    RPPutDepthFBPixel(x, y, thisz);
		}
	    }
	    
	    if (x == (int)(xminor))
		break;	/* exit from inner (x) loop */
	    
	    if (x < (int)(xminor)) {	/* walk direction changes when we hit low edge */
		x++;
	    } else {
		x--;
	    }

	} while (TRUE);		/* walk the x's */

	if (y < p1->sy) {	/* swap minor edge slopes */
	    xminor += dmdy;
	} else {
	    xminor += dldy;
	}

	xhigh += dhdy;
	y++;
    } 			/* rasterize from ymin to ymax: */

    /* optionally outline triangle, useful for debugging: */
    if (Flagged(RPScene.generic_flags, FLAG_RENDER_02) && usecfb) {
        rgba_t	red = {MAX_COLOR_VAL, 0, 0, MAX_COLOR_VAL};
	RPDrawColorFBLine(p0->sx, p0->sy, p1->sx, p1->sy, red, TRUE);
	RPDrawColorFBLine(p1->sx, p1->sy, p2->sx, p2->sy, red, TRUE);
	RPDrawColorFBLine(p2->sx, p2->sy, p0->sx, p0->sy, red, TRUE);
    }

    drawn_polys++;
}

