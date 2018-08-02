
/*
 * File:        ray.h
 *
 * This include file holds all of the ray-tracer specific stuff
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

#ifndef __RAY_H__
#define __RAY_H__

	/* defines: */

#define MAX_RAY_DEPTH           10
#define MAX_RAY_T               REALLY_BIG_FLOAT

/* ray types: */
#define PRIMARY_RAY             0x01
#define SHADOW_RAY              0x02
#define REFLECTION_RAY          0x03
#define REFRACTION_RAY          0x04

	/* data types: */

typedef struct { 	/* extra data if the ray intersection is with a polygon */
    float       u, v, w;        /* barycentric coordinates */
    Object_t    *op;            /* which object we hit */
    Tri_t       *tri;           /* which triangle we hit */
} TriShade_t;

typedef struct {
    int         origid; /* object from which this ray was spawned (-1) for primary */
    int         type;
    int         depth;
    xyz_t       orig;
    xyz_t       dir;
    float       t;
    TriShade_t  *surf;  /* if the ray hit a polygon, attach extra shading data */
} Ray_t;

typedef struct {
    int		culled_polys;
    int		primary_ray_count;
    int		primary_ray_hit_count;
    int		reflection_ray_count;
    int		reflection_ray_hit_count;
    int		refraction_ray_count;
    int		refraction_ray_hit_count;
    int		shadow_ray_count;
    int		shadow_ray_hit_count;
} RayStats_t;

	/* extern variables/functions: */

/* from raytrace.c */
extern RayStats_t	RayStats;

extern Ray_t    *NewRay(int type, int origid);
extern void     FreeRay(Ray_t *ray);
extern void     DumpRay(Ray_t *ray);
extern rgba_t   *trace_ray(Ray_t *ray);
extern int      trace_shadow_ray(int id, xyz_t *origin, Light_t *light);
extern void     raytrace_scene(void);

/* from intersect.c */
extern int      poly_intersect(Ray_t *ray, Object_t *op, float *t, xyz_t *p, xyz_t *n);
extern int      sphere_intersect(Ray_t *ray, Sphere_t *s, float *t, xyz_t *p, xyz_t *n);
extern int      tri_intersect(Ray_t *ray, Object_t *op, Tri_t *tri, float *t, xyz_t *p, xyz_t *n);
extern int      object_intersect(Ray_t *ray, Object_t *op, float *t, xyz_t *p, xyz_t *n);

/* from rayshade.c */
extern void     shade_sphere_pixel(rgba_t *color, Material_t *m, Ray_t *ray,
                        xyz_t *normal, xyz_t *surf, xyz_t *view, Object_t *op);
extern void     shade_tri_pixel(rgba_t *color, Ray_t *ray,
                        xyz_t *normal, xyz_t *surf, xyz_t *view, Object_t *op);
#endif
/* __RAY_H__ */

