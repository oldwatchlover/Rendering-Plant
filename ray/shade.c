
/*
 * File:	shade.c
 *
 * Pixel shading functions for the "moray" ray tracer.
 *
 * There are enough differences / optimizations between shading a sphere point
 * and a point on a triangle that I have coded separate functions.
 *
 * As the ray pixel shader is the absolute inner loop of the ray tracer, the extra
 * code is worth the optimiztions.
 *
 * A note about the math... all the material, colors, lights, etc. are in the 
 * range of 0.0 - 1.0. They are not clamped until the final output, in case 
 * the user wishes to "mis-use" the equations for artistic effects.
 *
 * Since the image is stored in 24 bit pixels, colors are converted to 0-255 right
 * before we return.
 *
 * In this version, only triangles can be textured... (someone should write 
 * more code)
 *
 * Most raytrace algorithms use an "offset bias" for secondary rays (shadow, 
 * reflected, refracted rays) so they ray starts just a bit off of the 
 * originating surface to avoid "self-intersetions" due to math precision 
 * errors. This alorithm does not do that - rather it uses a more simplified 
 * mechanism to avoid ALL self-intersections of secondary rays (and therefore 
 * an object cannot reflect or cast a shadow upon itsself)
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

static float	one255 = (1.0 / 255.0);

/* helper functions to spawn off secondary rays */
static rgba_t	*spawn_reflection(Ray_t *ray, int origid, xyz_t *surf, xyz_t *N);
static rgba_t	*spawn_refraction(Ray_t *ray, int origid, Material_t *m, 
                                  xyz_t *surf, xyz_t *N);

/* helper function, called in multiple places;
 * calculates L and H vectors as well as diffuse and specular terms.
 */
static void
calc_N_L_H(float *NdotL, float *NdotH, xyz_t *N, 
           xyz_t *lpos, xyz_t *surf, xyz_t *view)
{
    xyz_t	H, L;

         /* calculate L vector */
    vector_sub(&L, lpos, surf); /* light direction towards surface */
    vector_normalize(&L);

         /* calculate H vector */
    vector_add(&H, &L, view);
    vector_normalize(&H);
     
    *NdotL = Clamp0x(vector_dot(*N,L), 1.0f);
    *NdotH = Clamp0x(vector_dot(*N,H), 1.0f);
}

static void
calc_sphere_texcontrib(Colorf_t *pointcolor, xyz_t *N, Object_t *op)
{
    Texture_t	*tex;
    rgba_t	tex_samp;
    float	s, t;

    s = 0.5 + (atan2f(N->z, -N->x) / (2.0*Pi));
    t = 0.5 + (asinf(N->y) / Pi);

    tex = op->materials[0].texture[MATERIAL_COLOR];

    tex_samp = RPPointSampleTexture(tex, s, t, 1.0);

    if (Flagged(tex->flags, FLAG_TXT_MODULATE)) {

	    pointcolor->r *= (float)tex_samp.r * one255;
	    pointcolor->g *= (float)tex_samp.g * one255;
	    pointcolor->b *= (float)tex_samp.b * one255;
	    pointcolor->a *= (float)tex_samp.a * one255;

    } else {
	    pointcolor->r = (float)tex_samp.r * one255;
	    pointcolor->g = (float)tex_samp.g * one255;
	    pointcolor->b = (float)tex_samp.b * one255;
	    pointcolor->a = (float)tex_samp.a * one255;
    }
}

/* helper function, calculates texture contribution for polygons */
static void
calc_tri_texcontrib(Colorf_t *pointcolor, TriShade_t *tsp, Object_t *op, Material_t *m)
{
    Texture_t	*tex;
    Vtx_t	*vp = tsp->op->verts;
    Tri_t	*tp = tsp->tri;
    uv_t	*tcp = tsp->op->tcoords;
    rgba_t	tex_samp;
    float	s, t;

    if (vp == (Vtx_t *) NULL || tp == (Tri_t *) NULL) {
		/* missing data, force material shade */
		/* pointcolor already properly set to material */
        fprintf(stderr,"ERROR : trying to do TEXTURE with missing data (3)\n");
    } else {
	/* use barycentric coords to calc interp texture coords */
 	if (Flagged(tp->flags, FLAG_TRI_CLIP_GEN)) {
                /* texture coord already in s,t */
            s = tsp->u * vp[tp->v0].s + tsp->v * vp[tp->v1].s + tsp->w * vp[tp->v2].s;
            t = tsp->u * vp[tp->v0].t + tsp->v * vp[tp->v1].t + tsp->w * vp[tp->v2].t;
	} else if (tcp != (uv_t *) NULL) {	/* use tcoord array */
            s = tsp->u * tcp[tp->t0].u + tsp->v * tcp[tp->t1].u + tsp->w * tcp[tp->t2].u;
            t = tsp->u * tcp[tp->t0].v + tsp->v * tcp[tp->t1].v + tsp->w * tcp[tp->t2].v;
	} else {
            s = tsp->u * vp[tp->v0].s + tsp->v * vp[tp->v1].s + tsp->w * vp[tp->v2].s;
            t = tsp->u * vp[tp->v0].t + tsp->v * vp[tp->v1].t + tsp->w * vp[tp->v2].t;
	}

        tex = m->texture[MATERIAL_COLOR];

        tex_samp = RPPointSampleTexture(tex, s, t, 1.0);

        if (Flagged(tex->flags, FLAG_TXT_MODULATE) &&
	    Flagged(op->flags, FLAG_VERTSHADE)) {
			/* use barycentric coords to calc interp vertex colors */
    	    pointcolor->r = tsp->u * vp[tp->v0].r +
			    tsp->v * vp[tp->v1].r +
			    tsp->w * vp[tp->v2].r;
	    pointcolor->g = tsp->u * vp[tp->v0].g +
			    tsp->v * vp[tp->v1].g +
			    tsp->w * vp[tp->v2].g;
	    pointcolor->b = tsp->u * vp[tp->v0].b +
			    tsp->v * vp[tp->v1].b +
			    tsp->w * vp[tp->v2].b;
	    pointcolor->a = tsp->u * vp[tp->v0].a +
			    tsp->v * vp[tp->v1].a +
			    tsp->w * vp[tp->v2].a;
	    pointcolor->r = pointcolor->r * (float)tex_samp.r * one255;
	    pointcolor->g = pointcolor->g * (float)tex_samp.g * one255;
	    pointcolor->b = pointcolor->b * (float)tex_samp.b * one255;
	    pointcolor->a = pointcolor->a * (float)tex_samp.a * one255;
	} else {
	    pointcolor->r = (float)tex_samp.r * one255;
	    pointcolor->g = (float)tex_samp.g * one255;
	    pointcolor->b = (float)tex_samp.b * one255;
	    pointcolor->a = (float)tex_samp.a * one255;
	}
    }
}



/* shade a point located on an sphere's surface */
void
shade_sphere_pixel(rgba_t *color, Material_t *m, Ray_t *ray, xyz_t *N, 
	           xyz_t *surf, xyz_t *view, Object_t *op)
{
    rgba_t	*reflcolor = (rgba_t *) NULL, *refrcolor = (rgba_t *) NULL;
    Colorf_t	colorsum, pointcolor;
    Light_t	*light;
    float	NdotL, NdotH;
    int		i, inshadow = FALSE;

	/* start with point color set to material color */
    pointcolor.r = m->color.r;
    pointcolor.g = m->color.g;
    pointcolor.b = m->color.b;
    pointcolor.a = m->color.a;

    if (m->texture[MATERIAL_COLOR] != NULL) {
	calc_sphere_texcontrib(&pointcolor, N, op);
    }

    if (!Flagged(op->flags, FLAG_LIGHTING)) {
	color->r = (u8) Clamp0255(pointcolor.r * MAX_COLOR_VAL);
	color->g = (u8) Clamp0255(pointcolor.g * MAX_COLOR_VAL);
	color->b = (u8) Clamp0255(pointcolor.b * MAX_COLOR_VAL);
	color->a = (u8) Clamp0255(pointcolor.a * MAX_COLOR_VAL);
	return;
    }

    colorsum.r = 0.0f; colorsum.g = 0.0f; colorsum.b = 0.0f; colorsum.a = 0.0f;

	/* sum contributions for all of the lights: */
    for (i=0; i<RPScene.light_count; i++) {

	light = RPScene.light_list[i];

	/* check shadow, see if we can avoid the shading work */
	if (!Flagged(RPScene.flags, FLAG_NOSHADOW))
	    inshadow = trace_shadow_ray(op->id, surf, light);
        else
	    inshadow = FALSE;

        if (inshadow) { /* in shadow of this light, ambient only */
            colorsum.r +=  m->amb.r * pointcolor.r; 
            colorsum.g +=  m->amb.g * pointcolor.g; 
            colorsum.b +=  m->amb.b * pointcolor.b; 
            colorsum.a +=  m->amb.a * pointcolor.a; 
	} else {		/* not in shadow, full lighting */
            calc_N_L_H(&NdotL, &NdotH, N, &(light->pos), surf, view);
	    NdotH = powf(NdotH, m->shiny);

	    colorsum.r += (m->amb.r * pointcolor.r * light->color.r) + 
			  (m->diff.r * NdotL * pointcolor.r * light->color.r) + 
			  (m->spec.r * NdotH * m->highlight.r * light->color.r);
	    colorsum.g += (m->amb.g * pointcolor.g * light->color.g) + 
			  (m->diff.g * NdotL * pointcolor.g * light->color.g) + 
			  (m->spec.g * NdotH * m->highlight.g * light->color.g);
	    colorsum.b += (m->amb.b * pointcolor.b * light->color.b) + 
			  (m->diff.b * NdotL * pointcolor.b * light->color.b) + 
			  (m->spec.b * NdotH * m->highlight.b * light->color.b);
	    colorsum.a += (m->amb.a * pointcolor.a * light->color.a) +
			  (m->diff.a * NdotL * pointcolor.a * light->color.a) + 
			  (m->spec.a * NdotH * m->highlight.a * light->color.a);
        }
    }

    if (m->Krefl > 0.0) {
	reflcolor = spawn_reflection(ray, op->id, surf, N); 
		/* add reflection contribution */
        if (reflcolor != (rgba_t *) NULL) {
	    colorsum.r = (m->Krefl * (float)reflcolor->r * one255) + 
			 ((1.0 - m->Krefl) * colorsum.r);
	    colorsum.g = (m->Krefl * (float)reflcolor->g * one255) + 
			 ((1.0 - m->Krefl) * colorsum.g);
	    colorsum.b = (m->Krefl * (float)reflcolor->b * one255) + 
			 ((1.0 - m->Krefl) * colorsum.b);
		/* reflection doesn't modify alpha */
	    free (reflcolor);
	}
    }

    if (m->Krefr > 0.0) {
	refrcolor = spawn_refraction(ray, op->id, m, surf, N); 
        if (refrcolor != (rgba_t *) NULL) {
		/* add refraction contribution, using alpha (transparency) */
            float	alpha; 
	    alpha = (float)colorsum.a * one255;
	    colorsum.r = (alpha * colorsum.r) + 
			 ((1.0 - alpha) * (float)refrcolor->r * one255);
	    colorsum.g = (alpha * colorsum.g) + 
			 ((1.0 - alpha) * (float)refrcolor->g * one255);
	    colorsum.b = (alpha * colorsum.b) + 
			 ((1.0 - alpha) * (float)refrcolor->b * one255);
	    colorsum.a += refrcolor->a;	/* accumulate alpha */
	    free (refrcolor);
	}
    }

	/* add fog contribution */
    if (Flagged(RPScene.flags, FLAG_FOG) && surf->z < RPScene.fog_start) { 
	float		f;
	Colorf_t	new;

        f = (surf->z - RPScene.fog_start) / (RPScene.fog_end - RPScene.fog_start); 

	if (surf->z < RPScene.fog_end) {
	    new.r = RPScene.fog_color.r;
	    new.g = RPScene.fog_color.g;
	    new.b = RPScene.fog_color.b;
	} else {
	    new.r = f * RPScene.fog_color.r + (1.0 - f) * colorsum.r; 
	    new.g = f * RPScene.fog_color.g + (1.0 - f) * colorsum.g; 
	    new.b = f * RPScene.fog_color.b + (1.0 - f) * colorsum.b; 
 	}

	colorsum.r = new.r;
	colorsum.g = new.g;
	colorsum.b = new.b;
    }

    color->r = (u8) Clamp0255(colorsum.r * MAX_COLOR_VAL);
    color->g = (u8) Clamp0255(colorsum.g * MAX_COLOR_VAL);
    color->b = (u8) Clamp0255(colorsum.b * MAX_COLOR_VAL);
    color->a = (u8) Clamp0255(colorsum.a * MAX_COLOR_VAL);
}


/*
 * hate to duplicate code, but we want to use barycentric coordinates to shade
 * polygon geometry... too messy to put it all in one function.
 *
 */
void
shade_tri_pixel(rgba_t *color, Ray_t *ray, xyz_t *N, 
		xyz_t *surf, xyz_t *view, Object_t *op)
{
    Material_t	*tm;
    TriShade_t	*tsp = ray->surf;
    Colorf_t	colorsum, pointcolor;
    Light_t	*light;
    rgba_t	*reflcolor = (rgba_t *) NULL, *refrcolor = (rgba_t *) NULL;
    Vtx_t	*vp = tsp->op->verts;
    Tri_t	*tp = tsp->tri;
    float	NdotL, NdotH;
    int		i, inshadow = FALSE;


	/* start with point color set to material color */
    tm = &(op->materials[tp->material_id]);
    pointcolor.r = tm->color.r;
    pointcolor.g = tm->color.g;
    pointcolor.b = tm->color.b;
    pointcolor.a = tm->color.a;

    if (!Flagged(op->flags, FLAG_LIGHTING)) { 

		/* handle VERTSHADE or POLYSHADE with no lighting */
	if (Flagged(op->flags, FLAG_VERTSHADE)) {

	    if (vp == (Vtx_t *) NULL || tp == (Tri_t *) NULL) {
		/* missing data, force material shade */
		/* pointcolor already properly set to material */
	    } else { /* use barycentric coords to calc interp vertex colors */
		pointcolor.r = tsp->u * vp[tp->v0].r +
			       tsp->v * vp[tp->v1].r +
			       tsp->w * vp[tp->v2].r;
		pointcolor.g = tsp->u * vp[tp->v0].g +
			       tsp->v * vp[tp->v1].g +
			       tsp->w * vp[tp->v2].g;
		pointcolor.b = tsp->u * vp[tp->v0].b +
			       tsp->v * vp[tp->v1].b +
			       tsp->w * vp[tp->v2].b;
		pointcolor.a = tsp->u * vp[tp->v0].a +
			       tsp->v * vp[tp->v1].a +
			       tsp->w * vp[tp->v2].a;
	    }
	} else if (Flagged(op->flags, FLAG_POLYSHADE)) {

	    if (tp == (Tri_t *) NULL) {
		/* missing data, force material shade */
		/* pointcolor already properly set to material */
	    } else { /* use triangle color */
	        tp = tsp->tri;
	        pointcolor.r = tp->color.r;
	        pointcolor.g = tp->color.g;
	        pointcolor.b = tp->color.b;
	        pointcolor.a = tp->color.a;
	    }
	} else { /* use material color */
		/* pointcolor already properly set to material */
        }

        if (tm->texture[MATERIAL_COLOR] != NULL) {
	    calc_tri_texcontrib(&pointcolor, tsp, op, tm);
        }

	color->r = (u8) Clamp0255(pointcolor.r * MAX_COLOR_VAL);
	color->g = (u8) Clamp0255(pointcolor.g * MAX_COLOR_VAL);
	color->b = (u8) Clamp0255(pointcolor.b * MAX_COLOR_VAL);
	color->a = (u8) Clamp0255(pointcolor.a * MAX_COLOR_VAL);
	return;
    }

    /* coming into this function, the normal N has been set to the triangle normal
     * by the intersection routine... 
     */

	/* polygon shading supports a few of the render state flags... */
    if (!Flagged(op->flags, FLAG_FLATSHADE)) { 
		/* smooth shade, interpolate a normal */
	if (tsp == (TriShade_t *) NULL) { /* force flatshade, no bary coords */ 
		/* do nothing */
	} else { 

		/* use barycentric coords to calc interp normal */
	    N->x = tsp->u * vp[tp->v0].n.x +
		   tsp->v * vp[tp->v1].n.x +
		   tsp->w * vp[tp->v2].n.x;
	    N->y = tsp->u * vp[tp->v0].n.y +
		   tsp->v * vp[tp->v1].n.y +
		   tsp->w * vp[tp->v2].n.y;
	    N->z = tsp->u * vp[tp->v0].n.z +
		   tsp->v * vp[tp->v1].n.z +
		   tsp->w * vp[tp->v2].n.z;
	    vector_normalize(N);
        }
    }

    if (Flagged(op->flags, FLAG_VERTSHADE) || Flagged(op->flags, FLAG_RANDSHADE)) {
		/* both use same mechanism of vertex colors */
	if (tsp == (TriShade_t *) NULL) { 
		/* missing data, force material shade */
		/* pointcolor already properly set to material */
  	} else {

	    if (vp == (Vtx_t *) NULL || tp == (Tri_t *) NULL) {
		/* missing data, force material shade */
		/* pointcolor already properly set to material */
	    } else { /* use barycentric coords to calc interp vertex colors */
		pointcolor.r = tsp->u * vp[tp->v0].r +
			       tsp->v * vp[tp->v1].r +
			       tsp->w * vp[tp->v2].r;
		pointcolor.g = tsp->u * vp[tp->v0].g +
			       tsp->v * vp[tp->v1].g +
			       tsp->w * vp[tp->v2].g;
		pointcolor.b = tsp->u * vp[tp->v0].b +
			       tsp->v * vp[tp->v1].b +
			       tsp->w * vp[tp->v2].b;
		pointcolor.a = tsp->u * vp[tp->v0].a +
			       tsp->v * vp[tp->v1].a +
			       tsp->w * vp[tp->v2].a;
	    }
	}
    }

    if (Flagged(op->flags, FLAG_POLYSHADE)) {
	if (tsp == (TriShade_t *) NULL) { 
		/* missing data, force material shade */
		/* pointcolor already properly set to material */
  	} else {
	    if (tp == (Tri_t *) NULL) {
		/* missing data, force material shade */
		/* pointcolor already properly set to material */
	    } else { /* use triangle color */
	        pointcolor.r = tp->color.r;
	        pointcolor.g = tp->color.g;
	        pointcolor.b = tp->color.b;
	        pointcolor.a = tp->color.a;
	    }
	}
    }

    if (tm->texture[MATERIAL_COLOR] != NULL) {
	calc_tri_texcontrib(&pointcolor, tsp, op, tm);
    }

    colorsum.r = 0.0; colorsum.g = 0.0; colorsum.b = 0.0; colorsum.a = 0.0;

	/* sum the contribution of each light */

    for (i=0; i<RPScene.light_count; i++) {

	light = RPScene.light_list[i];

	/* check shadow, see if we can avoid the shading work */
	if (!Flagged(RPScene.flags, FLAG_NOSHADOW))
	    inshadow = trace_shadow_ray(op->id, surf, light);
        else
	    inshadow = FALSE;

        if (inshadow) { /* in shadow of this light, ambient only */
            colorsum.r += (tm->amb.r * pointcolor.r); 
            colorsum.g += (tm->amb.g * pointcolor.g); 
            colorsum.b += (tm->amb.b * pointcolor.b); 
            colorsum.a += (tm->amb.a * pointcolor.a); 
	} else {				/* not in shadow, full lighting */

            calc_N_L_H(&NdotL, &NdotH, N, &(light->pos), surf, view);
            NdotH = powf(NdotH, tm->shiny);

	    colorsum.r += (tm->amb.r * pointcolor.r * light->color.r) + 
			  (tm->diff.r * NdotL * pointcolor.r * light->color.r) + 
			  (tm->spec.r * NdotH * tm->highlight.r * light->color.r);
	    colorsum.g += (tm->amb.g * pointcolor.g * light->color.g) + 
			  (tm->diff.g * NdotL * pointcolor.g * light->color.g) + 
			  (tm->spec.g * NdotH * tm->highlight.g * light->color.g);
	    colorsum.b += (tm->amb.b * pointcolor.b * light->color.b) + 
			  (tm->diff.b * NdotL * pointcolor.b * light->color.b) + 
			  (tm->spec.b * NdotH * tm->highlight.b * light->color.b);
	    colorsum.a += (tm->amb.a * pointcolor.a * light->color.a) +
			  (tm->diff.a * NdotL * pointcolor.a * light->color.a) + 
			  (tm->spec.a * NdotH * tm->highlight.a * light->color.a);
        }
    }

    if (tm->Krefl > 0.0) {
	reflcolor = spawn_reflection(ray, op->id, surf, N); 
		/* add reflection contribution */
        if (reflcolor != (rgba_t *) NULL) {
	    colorsum.r = (tm->Krefl * (float)reflcolor->r * one255) + 
			 ((1.0 - tm->Krefl) * colorsum.r);
	    colorsum.g = (tm->Krefl * (float)reflcolor->g * one255) + 
			 ((1.0 - tm->Krefl) * colorsum.g);
	    colorsum.b = (tm->Krefl * (float)reflcolor->b * one255) + 
			 ((1.0 - tm->Krefl) * colorsum.b);
		/* reflection doesn't modify alpha */
	    free (reflcolor);
	}
    }

    if (tm->Krefr > 0.0) {
	refrcolor = spawn_refraction(ray, op->id, tm, surf, N); 
        if (refrcolor != (rgba_t *) NULL) {
            float	alpha; 
		/* add refraction contribution, using alpha (transparency) */
	    alpha = (float)colorsum.a * one255;
	    colorsum.r = (alpha * colorsum.r) + 
			 ((1.0 - alpha) * (float)refrcolor->r * one255);
	    colorsum.g = (alpha * colorsum.g) + 
			 ((1.0 - alpha) * (float)refrcolor->g * one255);
	    colorsum.b = (alpha * colorsum.b) + 
			 ((1.0 - alpha) * (float)refrcolor->b * one255);
	    colorsum.a += refrcolor->a;	/* accumulate alpha */
	    free (refrcolor);
	}
    }

	/* add fog contribution */
    if (Flagged(RPScene.flags, FLAG_FOG) && surf->z < RPScene.fog_start) { 
	float		f;
	Colorf_t	new;

        f = (surf->z - RPScene.fog_start) / (RPScene.fog_end - RPScene.fog_start); 

	if (surf->z < RPScene.fog_end) {
	    new.r = RPScene.fog_color.r;
	    new.g = RPScene.fog_color.g;
	    new.b = RPScene.fog_color.b;
	} else {
	    new.r = f * RPScene.fog_color.r + (1.0 - f) * colorsum.r; 
	    new.g = f * RPScene.fog_color.g + (1.0 - f) * colorsum.g; 
	    new.b = f * RPScene.fog_color.b + (1.0 - f) * colorsum.b; 
 	}

	colorsum.r = new.r;
	colorsum.g = new.g;
	colorsum.b = new.b;
    }

    color->r = (u8) Clamp0255(colorsum.r * MAX_COLOR_VAL);
    color->g = (u8) Clamp0255(colorsum.g * MAX_COLOR_VAL);
    color->b = (u8) Clamp0255(colorsum.b * MAX_COLOR_VAL);
    color->a = (u8) Clamp0255(colorsum.a * MAX_COLOR_VAL);
}

/* these functions calculate the direction and spawn the secondary rays */

static rgba_t *
spawn_reflection(Ray_t *ray, int origid, xyz_t *surf, xyz_t *N)
{
    Ray_t	*reflray;
    rgba_t	*color = (rgba_t *) NULL;
    float	reflect;
    xyz_t	tmpvec;

    reflray = NewRay(REFLECTION_RAY, origid);
    reflray->depth = ray->depth + 1;

    reflray->orig.x = surf->x; 
    reflray->orig.y = surf->y; 
    reflray->orig.z = surf->z; 

    reflect = 2.0 * vector_dot(ray->dir, *N);
    vector_scale(&tmpvec, N, reflect);
    vector_sub(&(reflray->dir), &(ray->dir), &tmpvec);
    vector_normalize(&(reflray->dir));

        /* recursively calculate reflection  */
    color = trace_ray(reflray);

    FreeRay(reflray);

    RayStats.reflection_ray_count++;

    return (color);
}

static rgba_t *
spawn_refraction(Ray_t *ray, int origid, Material_t *m, xyz_t *surf, xyz_t *N)
{
    Ray_t	*refrray;
    rgba_t	*color = (rgba_t *) NULL;
    xyz_t	tvec;
    float	n, cosI, sinT2, term3;

    refrray = NewRay(REFRACTION_RAY, origid);
    refrray->depth = ray->depth + 1;

    refrray->orig.x = surf->x; refrray->orig.y = surf->y; refrray->orig.z = surf->z;

    n = 1.0 / m->Krefr;
    cosI = vector_dot(ray->dir, *N);
    sinT2 = Sqr(n) * (1.0 - Sqr(cosI));
    if (sinT2 > 1.0) {
	/* total internal reflection */
	color = (rgba_t *) malloc(sizeof(rgba_t));
	color->r = color->g = color->b = color->a = MAX_COLOR_VAL;
    } else {
    
 	term3 = n + sqrtf(1.0 - sinT2);
        vector_scale(&(refrray->dir), &(ray->dir), n);
        vector_scale(&tvec, N, term3);
        vector_sub(&(refrray->dir), &(refrray->dir), &tvec);
        vector_normalize(&(refrray->dir));

        /* recursively calculate refraction  */
        color = trace_ray(refrray);
    }

    FreeRay(refrray);

    RayStats.refraction_ray_count++;

    return (color);
}



