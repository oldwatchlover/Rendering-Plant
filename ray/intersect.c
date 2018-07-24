
/*
 * File:        intersect.c
 *
 * This file holds the ray/primitive intersection functions.
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
#include "ray.h"


/* high level object intersection function, called from trace_ray() */
int
object_intersect(Ray_t *ray, Object_t *op, float *t, xyz_t *p, xyz_t *normal)
{
    int         retval = FALSE;

        /* For these types of rays we want to avoid self-interesections.
         * Some ray tracers uses a "bias" to offset the origin of the ray
         * to prevent this, but that solution can be problematic as the correct
  	 * value for the bias depends on view parameters, surface curvature, etc.
         *
         * I opt for a simpler (but more restrictive) approach - prevent
         * self-intersections for all secondary rays. The code is less messy,
         * the images have fewer artifacts, but at the cost of no self-shadowing
         * or self-reflection. That's ok with me, I probably won't ever render
         * geometry complicated enough for that to be obvious...
         */
    if (ray->type == SHADOW_RAY ||
        ray->type == REFRACTION_RAY ||
        ray->type == REFLECTION_RAY) {

	/* HINT: for polygonal objects, if we saved the triangle ID as well
 	 * as the object ID, we could avoid artifacts but allow self shadowing
	 * of polygonal geometry (a triangle cannot cast a shadow on itself,	
	 * but other triangles in the object could cast shadows on that tri) 
  	 */
        if (ray->origid == op->id)
            return (FALSE);
    }

    if (op->type == OBJ_TYPE_SPHERE) {
        Sphere_t        *sp = op->sphere;

        retval = sphere_intersect(ray, sp, t, p, normal);

    } else if (op->type == OBJ_TYPE_POLY) {

        retval = poly_intersect(ray, op, t, p, normal);

    } else {     /* can't happen */
        fprintf(stderr,"%s : ERROR : unknown object type %d (%s, %d)\n",
                        program_name, op->type, __FILE__, __LINE__);
    }

    return (retval);
}

/* ray-sphere intersection */
int
sphere_intersect(Ray_t *ray, Sphere_t *s, float *t, xyz_t *p, xyz_t *n)
{
    xyz_t       e_c, s_c;
    float       b, discr, t0, t1;

    s_c.x = s->center.x; s_c.y = s->center.y; s_c.z = s->center.z;
    vector_sub(&e_c, &(ray->orig), &s_c);

    b = -1.0f * vector_dot(e_c, ray->dir);
    discr = Sqr(b) - vector_dot(e_c, e_c) + Sqr(s->radius);

    if (discr < 0.0f)
        return (FALSE);

    discr = sqrtf(discr);

    t0 = b - discr;
    t1 = b + discr;

    if (t1 < 0.0f)
        return (FALSE);

    if (t0 < 0.0)
        t0 = 0.0;

    *t = Min(t0, t1);

        /* calc point of interesection and normal for shader */
    vector_scale(p, &(ray->dir), *t);
    vector_add(p, &(ray->orig), p);

    vector_sub(n, p, &(s->center));
    vector_normalize(n);

    return (TRUE);
}


/* poly object intersect is a little more complicated, due to bounding spheres,
 * back/front face culling, multiple triangles to test, and possibly 
 * multiple triangles of the object hit...
 */
int
poly_intersect(Ray_t *ray, Object_t *op, float *t, xyz_t *p, xyz_t *normal)
{
    TriShade_t  *mints = (TriShade_t *) NULL;
    Tri_t       *tri;
    xyz_t       minp, minn, tmp_n, tmp_p;
    float       tmp_t = MAX_RAY_T, mint = MAX_RAY_T;
    int         i, found = FALSE, shadow = FALSE, cullthis = FALSE, retval = FALSE;

    if (op->sphere != (Sphere_t *) NULL) {
        found = sphere_intersect(ray, op->sphere, &tmp_t, &tmp_p, &tmp_n);
    }

    if (found) {        /* ray hit bounding sphere, must test each triangle */
        for (i=0; i<op->tri_count && !shadow; i++) {
            tri = &(op->tris[i]);

            /* do front/back face culling here */
            cullthis = FALSE;
            if (ray->type == PRIMARY_RAY &&
                Flagged(op->flags, FLAG_CULL_BACK)) {
                if (Flagged(tri->flags, FLAG_CULL_BACK))
                    cullthis = TRUE;
            }
            if (ray->type == PRIMARY_RAY &&
                Flagged(op->flags, FLAG_CULL_FRONT)) {
                if (Flagged(tri->flags, FLAG_CULL_FRONT))
                    cullthis = TRUE;
            }

                /* use tmp t,p,n in case ray hits multiple tris in this obj */
            if (cullthis) {
                found = FALSE;
		RayStats.culled_polys++;
            } else {
                found = tri_intersect(ray, op, tri, &tmp_t, &tmp_p, &tmp_n);
 	    }

            if (found && (tmp_t < mint)) {
                retval = TRUE;
                mint = tmp_t;
                minp.x = tmp_p.x; minp.y = tmp_p.y; minp.z = tmp_p.z;
                minn.x = tmp_n.x; minn.y = tmp_n.y; minn.z = tmp_n.z;

		if (mints != (TriShade_t *) NULL)
		    free (mints);	/* free un-needed TriShade_t */

		mints = ray->surf; /* preserve the TriShade_t hanging off ray */

                if (ray->type == SHADOW_RAY) {
                    shadow = TRUE;  /* exit early, any hit for a shadow counts */
                }
	    } else {
		if (found) {
		    if (ray->surf != (TriShade_t *) NULL) {
			free (ray->surf);
			ray->surf = (TriShade_t *) NULL;
		    }
		}
	    }
        }
                /* hit a tri in the obj, update calling parameters */
        if (retval) {
            *t = mint;
            p->x = minp.x; p->y = minp.y; p->z = minp.z;
            normal->x = minn.x; normal->y = minn.y; normal->z = minn.z;
	    ray->surf = mints;
        }
    }

    return (retval);
}


/* geometric solution to ray-tri intersection with barycentric coordinates */
int
tri_intersect(Ray_t *ray, Object_t *op, Tri_t *tri, float *t, xyz_t *p, xyz_t *n)
{
    TriShade_t  *tsp;
    xyz_t       tvec, Q_a, Q_b, Q_c;
    float       u, v, t0, NdotD;

    NdotD = vector_dot(tri->pN, ray->dir);
    if (NdotD > -EpEpsilon && NdotD < EpEpsilon) {
        return (FALSE);    /* ray parallel to plane of triangle, miss */
    }

    t0 = (tri->d - vector_dot(tri->pN, ray->orig)) / NdotD;

    if (t0 < 0.0) 	/* intersection is behind us */
        return (FALSE);

        /* calc point of interesection of plane along ray */
    vector_scale(p, &(ray->dir), t0);
    vector_add(p, &(ray->orig), p);

        /* test inside/out of point p with edges of triangle. any outside is miss  */

    vector_sub(&Q_a, p, &(op->verts[tri->v0].pos)); /* edge 0 */
    vector_cross(&tvec, &(tri->v1_v0), &Q_a);
    if (vector_dot(tri->pN, tvec) < 0.0) /* this is bary w, calc later from u and v */
        return (FALSE);

    vector_sub(&Q_b, p, &(op->verts[tri->v1].pos)); /* edge 1 */
    vector_cross(&tvec, &(tri->v2_v1), &Q_b);
    u = vector_dot(tri->pN, tvec);
    if (u < 0.0) 
        return (FALSE);

    vector_sub(&Q_c, p, &(op->verts[tri->v2].pos)); /* edge 2 */
    vector_cross(&tvec, &(tri->v0_v2), &Q_c);
    v = vector_dot(tri->pN, tvec);
    if (v < 0.0) 
        return (FALSE);

        /* ray hits the triangle; update data for recursion and shading */

    tsp = (TriShade_t *) malloc(sizeof(TriShade_t));
    tsp->u = u / vector_dot(tri->pN, tri->pN);
    tsp->v = v / vector_dot(tri->pN, tri->pN);
    tsp->w = 1.0 - tsp->u - tsp->v;
    tsp->op = op;
    tsp->tri = tri;

    *t = t0;
    n->x = tri->normal.x; n->y = tri->normal.y; n->z = tri->normal.z;
    ray->surf = tsp;

    return (TRUE);
}

#if 0
/*  
 * Möller-Trumbore intersection algorithm
 * implemented from the original paper...
 *
 * BUG: not really working, barycentric coords u/v/w don't seem right 
 * when interploating normals/textures
 *
 * (note: if you wish to debug/use this, you must add v2_v0 to 
 * the tri_t structure and save it when it gets calculated in ../rp/polygons.c)
 *
 */
int
tri_intersectMT(Ray_t *ray, Object_t *op, Tri_t *tri, float *t, xyz_t *p, xyz_t *n)
{
    TriShade_t  *tsp;
    double      det, u, v;
    xyz_t       pvec, tvec, qvec;

    vector_cross(&pvec, &(ray->dir), &(tri->v2_v0));
    det = vector_dot(tri->v1_v0, pvec);

        /* if det < EPSILON, triangle is back-facing */

    if (det > -Epsilon && det < Epsilon)  /* parallel, if near zero */
        return (FALSE);

    det = 1.0f / det;
    vector_sub(&tvec, &(ray->orig), &(op->verts[tri->v0].pos));
    u = det * vector_dot(tvec, pvec);
    if (u < 0.0 || u > 1.0)
        return (FALSE);

    vector_cross(&qvec, &tvec, &(tri->v1_v0));
    v = det * vector_dot(ray->dir, qvec);
    if (v < 0.0 || (u+v) > 1.0)
        return (FALSE);

    *t = det * vector_dot(tri->v2_v0, qvec);
    if (*t < Epsilon) 
        return (FALSE); /* a line intersection but not a ray intersection */

         /* calc point of interesection */
    vector_scale(p, &(ray->dir), *t);
    vector_add(p, &(ray->orig), p);

    tsp = (TriShade_t *) calloc(1, sizeof(TriShade_t));
    tsp->op = op;
    tsp->tri = tri;
    tsp->u = u; tsp->v = v; tsp->w = 1.0 - tsp->u - tsp->v;
    n->x = tri->normal.x; n->y = tri->normal.y; n->z = tri->normal.z;
    ray->surf = tsp;

    return (TRUE);
}

#endif

