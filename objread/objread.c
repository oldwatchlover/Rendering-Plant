
/*
 * File:        objread.c
 *
 * main function.
 *
 * uses yacc/lexx to parse a Wavefront .obj file
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
#include <stdarg.h>
#include <string.h>
#include <errno.h>

#include "objread.h"

/* for parser */
int obj_mylineno;
char _RPObjline_buffer[MAX_FILENAME_LENGTH];
char _RPObjinput_file[MAX_FILENAME_LENGTH];

/* from yacc: */
extern int zzparse(void);

/* variables from the parser: */
extern FILE *zzin;
extern int zzdebug;

/* geometry data */

vertex_t	*VertexList = (vertex_t *) NULL;
int		input_vertex_count = 0;

xyz_t		*NormalList = (xyz_t *) NULL;
int		input_normal_count = 0;

uv_t		*TexCoordList = (uv_t *) NULL;
int		input_texcoord_count = 0;

tri_t		*TriList = (tri_t *) NULL;
int		input_tri_count = 0;

int
read_obj_from_file(char *filename, Object_t *op)
{
    Vtx_t	*vp;
    vertex_t	*ivp;
    Tri_t	*tp;
    tri_t	*itp;
    int		i;

    if ((zzin=fopen(filename, "r")) == (FILE *) NULL) {
        fprintf(stderr,"%s : ERROR : can't open .obj file [%s] (1) errno = %d.\n",
		program_name,filename,errno);
	return(FALSE);
    }

    strcpy(_RPObjinput_file, filename);

    input_vertex_count = 0;
    input_normal_count = 0;
    input_texcoord_count = 0;
    input_tri_count = 0;

    VertexList   = (vertex_t *) calloc(MAX_VERTS, sizeof(vertex_t));
    NormalList   = (xyz_t *)    calloc(MAX_VERTS, sizeof(xyz_t));
    TexCoordList = (uv_t *)     calloc(MAX_VERTS, sizeof(uv_t));
    TriList      = (tri_t *)    calloc(MAX_TRIS, sizeof(tri_t));

	/* do any initialization: */

    state_init();

	/* read in .obj file, convert to internal data structure */

    if (zzparse()) {
        fclose(zzin);
	return(FALSE);
    }

    fclose(zzin);

	/* fill Object_t structure with the data */

    op->sphere = NULL;
    op->verts = (Vtx_t *) malloc(input_vertex_count * sizeof(Vtx_t));
    op->tris = (Tri_t *) malloc(input_tri_count * sizeof(Tri_t));
    op->tri_count = input_tri_count;
    op->vert_count = input_vertex_count;

        /* copy geometry data */

    vp = op->verts;
    ivp = VertexList;

    for (i=0; i<input_vertex_count; i++) {

	vp->pos.x = ivp->pos.x;
	vp->pos.y = ivp->pos.y;
	vp->pos.z = ivp->pos.z;

	vp->n.x = ivp->normal.x;
	vp->n.y = ivp->normal.y;
	vp->n.z = ivp->normal.z;

	vp->s = ivp->tcoords.u;
	vp->t = ivp->tcoords.v;

	vp++;
	ivp++;
    }

    tp = op->tris;
    itp = TriList;

    for (i=0; i<input_tri_count; i++) {
	
	tp->v0 = itp->v0;
	tp->v1 = itp->v1;
	tp->v2 = itp->v2;

 	tp->material_id = itp->material_id;

	tp++;
	itp++;
    }

	/* free temp input memory */
    free(VertexList);
    free(NormalList);
    free(TexCoordList);
    free(TriList);

    return(TRUE);
}


void
debug_printf(FILE *out, const char *fmt, ...)
{
    va_list	args;

    if (out == (FILE *) NULL)
	return;

    va_start(args, fmt);

#ifdef DEBUG_LIBOBJ
    vfprintf(out, fmt, args);
#endif

    va_end(args);
}

