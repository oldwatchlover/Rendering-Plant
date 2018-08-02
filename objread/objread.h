
/*
 * File:        objread.h
 *
 * include file for objread library
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

#ifndef __OBJ_READ_H__
#define __OBJ_READ_H__

#include "rp.h"

	/* program constants: */
#define MAX_LINE_LENGTH         256


typedef struct
{
    int		v0, v1, v2;
    int		t0, t1, t2;
    int		n0, n1, n2;
    xyz_t	normal;
    int		material_id;
} tri_t;

typedef struct
{
    xyz_t	pos;
    rgba_t	color;
    uv_t	tcoords;
    xyz_t	normal;
} vertex_t;

typedef struct
{
    char	name[64];

    int		first;
    int		last;
} group_t;

typedef struct
{
    char	name[256];

    int		vcount;		/* vertex count */
    int		pcount;		/* poly count */
    int		ncount;		/* normal count */
    int		tcount;		/* tex coord count */

    xyz_t	*verts;

    tri_t	*polys;

    xyz_t	*norms;

    uv_t	*tcoords;
    
    int		*smooth;	/* one per vertex for smoothing */

} object_t;


/* program external variables: */

extern int	_RPObjRead_current_material;

/* program external functions: */

	/* from objread.c: */
extern vertex_t		*VertexList;
extern int		input_vertex_count;

extern xyz_t		*NormalList;
extern int 		input_normal_count;

extern uv_t		*TexCoordList;
extern int		input_texcoord_count;

extern tri_t		*TriList;
extern int		input_tri_count;

extern void		debug_printf(FILE *out, const char *fmt, ...);

	/* from main.c: */
extern void		create_bounding_sphere(xyz_t *cen, float *rad);
extern void		move_bounding_sphere(xyz_t *cen, float *rad);

	/* from object.c: */
extern object_t		*input_object_new(void);
extern void		input_object_free(object_t *op);

	/* from objread.c: */
extern void		obj_read(FILE *in, char *filename);

	/* from vertex.c: */
extern void		add_vertex(float x, float y, float z, float w);
extern void		add_vtexcoords(int n, float u, float v, float w);
extern void		add_vnormal(float x, float y, float z);

	/* from face.c: */
extern void		add_face(int n, int v[], int vt[], int vn[]); 

	/* from state.c: */
extern char		current_object[];
extern group_t		grouplist[];
extern int		groupcount;
extern group_t		*current_group;
extern group_t		*last_group;
extern int		current_smooth;


extern void		state_init(void);
extern void		add_object(char *name);
extern void		add_group(int n, char *names[]);
extern void		add_smooth(int val);

	/* from material.c: */
extern void		add_mtllib(int n, char *names[]);
extern void		add_usemtl(char *name);

	/* from texture.c: */
extern void		add_maplib(int n, char *names[]);
extern void		add_usemap(char *name);

#endif
/* __OBJ_READ_H__ */

