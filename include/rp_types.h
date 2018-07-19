
/*
 * File:	rp_types.h
 *
 * This file holds all of the data types used by the "Rendering Plant" framework.
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

#ifndef __RP_TYPES_H__
#define __RP_TYPES_H__

/* useful types: */
typedef unsigned char   	u8;
typedef unsigned short int   	u16;
typedef unsigned int    	u32;

/* common, useful 3D graphics types: */

typedef struct {
    float       x, y, z;
} xyz_t;

typedef struct {
    float       u, v;
} uv_t;

typedef struct {
    u8          r, g, b, a;
} rgba_t;

typedef struct {
    float       r, g, b, a;
} Colorf_t;


/* more complex structures: */

typedef struct {
    xyz_t	pos;		/* world space points, after MV xform  */
    xyz_t	proj;		/* project space points, save for clipping */
    float       r, g, b, a;	/* vertex color */
    float       s, t;		/* vertex texture coordinates */
    xyz_t       n;      	/* vertex normal */
	/* used by scanline and hw paint algorithm: */
    int		sx, sy, sz;	/* screen position */
    xyz_t       e;      	/* eye vector */
    float	w, inv_w;	/* save for persp div and tex correct */
    u8		cc;		/* clip codes from proj points and frustrum */
    u8		pad[3];		/* unused */
} Vtx_t;

typedef struct {
    int         v0, v1, v2;	/* index into vertex array */
    int 	flags;	/* for back/front facing identity and clipping use */
    int		material_id;	/* future use */
    Colorf_t	color;		/* poly color, if desired */
                        /* precompute these for ray-tri intersection tests: */
    xyz_t       normal;	/* tri normal (normalized for shading) */
    xyz_t       pN;	/* plane normal (un-normalized) */
    float       d;      /* vector_dot(normal, v0) ... or any vert */
    xyz_t       v1_v0;  /* for plane equation: v0 = A, v1 = B, v2 = C */
    xyz_t       v2_v1;  /* must fill these in after xform (world space) */
    xyz_t       v0_v2;	/* ... */
} Tri_t;

/* a sphere geometry primitive for ray-tracing: */
typedef struct {
    xyz_t       center;
    float       radius;
} Sphere_t;

/* internal texture structure: */
typedef struct {
    char        *filename;
    char        inuse;
    u32         flags;
    int         xres, yres;
    float       sscale, tscale;
    float       soff, toff;
    rgba_t      **tmem; /* pointer to texels */
} Texture_t;

/* materials belong to objects, get passed to shader */
typedef struct {
    u32		flags;
    Colorf_t    color;
	/* add emission? */
    Colorf_t	amb;
    Colorf_t	diff;
    Colorf_t	spec;
    Colorf_t	highlight;	/* color of highlight */
    float	shiny;
	/* additional ray tracing properties */
    float       Krefl;
    float       Krefr;
	/* should add multiple textures, reflection and bump maps here... */
    char	*texname;
    int		texid;
    Texture_t	*texture;
} Material_t;

/* high level object structure */
typedef struct {
    int         id;
    int         type;
    Sphere_t    *sphere;        /* sphere, or bounding sphere */
    Vtx_t       *verts;
    Tri_t       *tris;
    int         vert_count;
    int         tri_count;
    Material_t  *material;
    float	mmtx[4][4];
} Object_t;

typedef struct {
    xyz_t       pos;
    xyz_t       coi;
    xyz_t       up;
    float       fov;    /* fov/2 in degrees */
    float	aspect;
/* internal or generated data: */
    xyz_t       eye;
    xyz_t       dir;
    float       fovr;   /* fov/2 in radians */
} Camera_t;

typedef struct {
    int		type;
    xyz_t       pos;
    Colorf_t    color;
	/* additonal properties for directed lights: */
    xyz_t	coi;		/* direction spot light is pointing */
    float	fov;		/* field of view */
    float	focus;		/* focus (0.0 - 1.0) */
    float	range;		/* distance attenuation */
    float	value;		/* intensity (0.0 - 1.0) mult by color */
} Light_t; 

typedef struct {
    float       sx, sy, sz;
    float       tx, ty, tz;
} Viewport_t;

typedef struct {
    int		ulx, uly, lrx, lry;
} Scissor_t;

/* main scene structure, can access from your renderer */ 
typedef struct {
    u32		flags;
    u32		generic_flags;
    Camera_t	*camera;
    Viewport_t	*viewport;
    Scissor_t	*scissor_box;
    float	hither, yon;
    Object_t	*obj_list[MAX_OBJS];
    int		obj_count;
    Light_t	*light_list[MAX_LIGHTS];
    int		light_count;
    Texture_t	*texture_list[MAX_TEXTURES+1];
    int		texture_count;
    int		xres, yres;
    int		num_samples;
    rgba_t	background_color;
    Colorf_t	fog_color;
    float	fog_start, fog_end;
    float	ambient;
    char	*output_file;
    char	*background_file;
    int		input_polys;
    int		clipped_polys;
    int		trivial_rejected_polys;
    int		tiny_rejected_polys;
} Scene_t;

/* for frame buffer line drawing (useful for debugging) */
typedef void (*PixelPlotProc) (int x, int y, int color);

#endif
/* __RP_TYPES_H__ */


