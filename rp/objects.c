
/*
 * File:	objects.c
 *
 * This file holds the data structures and functions that manipulate objects.
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
#include <string.h>
#include <math.h>

#include "rp.h"

u32	_RPTempObjRenderFlags = 0x0;

static Material_t default_material = {
    {1.0, 1.0, 1.0, 1.0},	/* color */
    {0.2, 0.2, 0.2, 1.0},	/* amb */
    {1.0, 1.0, 1.0, 1.0},	/* diff */
    {1.0, 1.0, 1.0, 1.0},	/* spec */
    {1.0, 1.0, 1.0, 1.0},	/* highlight color */
    50.0,			/* shiny */
    0.0, 0.0,			/* Krefl, Krefr */
    {0x0, 0x0, 0x0, 0x0},	/* *texture[] */
};

static Sphere_t * create_bounding_sphere(Object_t *);

extern void    	RPCalculateVertexNormals(Object_t *op, int trinormals);


/* 
 * returns a generic object to the caller for them to fill in the details
 * also binds object to the currently defined material
 */
Object_t *
RPAddObject(int type)
{
    Object_t	*o;
    Material_t	*m, *cm;
    

    o = (Object_t *) calloc(1, sizeof(Object_t));
    m = (Material_t *) calloc(1, sizeof(Material_t));

    cm = RPGetCurrentMaterial();
    if (cm == (Material_t *) NULL) {
	memcpy(m, &default_material, sizeof(Material_t));
    } else {
	memcpy(m, cm, sizeof(Material_t));
    }

    o->flags = _RPTempObjRenderFlags;	/* copy current obj_render_flags to object */

    if (!Flagged(o->flags, FLAG_TEXTURE)) {
	m->texture[MATERIAL_COLOR] = (Texture_t *) NULL;
	m->texture[MATERIAL_AMBIENT] = (Texture_t *) NULL;
	m->texture[MATERIAL_DIFFUSE] = (Texture_t *) NULL;
	m->texture[MATERIAL_SPECULAR] = (Texture_t *) NULL;
    }

        /* store current top of model matrix stack with object for later xform */
    ident_mtx(o->mmtx);
    cat_matrix(o->mmtx, m_mtx,o->mmtx);

    o->type = type;
    o->id = RPScene.obj_count;
    o->material = m;
    o->mtl_count = 1;

    RPScene.obj_list[RPScene.obj_count++] = o;

    return (o);
}

/*
 * incorporating 2 instances of yacc/lex is a little complicated...
 *
 */
extern int	read_obj_from_file(char *filename, Object_t *op);

/* called from the parser, reads a Wavefront obj geometry */
void
RPReadObjectFromFile(char *fname)
{
    Object_t	*newobj;

    fprintf(stderr,"reading geometry from Wavefront .obj file [%s]\n",fname);

    newobj = RPAddObject(OBJ_TYPE_POLY);

    if (!read_obj_from_file(fname, newobj)) {
	/* un-allocate, back out */

	RPFreeObject(newobj);
	RPScene.obj_count--;
    } else {
	RPScene.input_polys += newobj->tri_count;
    } 

    if (Flagged(RPScene.flags, FLAG_VERBOSE)) {
        fprintf(stderr," +Added Object %d ... poly: %d verts, %d triangles\n",
                newobj->id,newobj->vert_count,newobj->tri_count);
    }
}


void
RPFreeObject(Object_t *op)
{
    if (op == (Object_t *) NULL)
	return;

    if (op->sphere != (Sphere_t *) NULL)
	free (op->sphere);

    if (op->verts != (Vtx_t *) NULL)
	free (op->verts);

    if (op->tris != (Tri_t *) NULL)
	free (op->tris);

    if (op->material != (Material_t *) NULL)
	free (op->material);

    free (op);
}

void
RPCleanupObjects(void)
{
    int         i;

    for (i=0; i<RPScene.obj_count; i++) {
	RPFreeObject(RPScene.obj_list[i]);
	RPScene.obj_list[i] = (Object_t *) NULL;
    }
}

/* transform all the objects from model to world space */
void
RPProcessObjects(int doProject)
{
    Object_t	*op;
    Sphere_t	*sp;
    int         i;

        /* transform lights */
    RPTransformLights();

        /* transform and process all objects in the scene */

    for (i=0; i<RPScene.obj_count; i++) {
	op = RPScene.obj_list[i];

	if (op->type == OBJ_TYPE_SPHERE) {

	    sp = op->sphere;

	    RPProcessSphere(op, sp);

	    if (Flagged(RPScene.flags, FLAG_VERBOSE2)) {
		fprintf(stderr,"final sphere at (%8.3f,%8.3f,%8.3f) radius %f\n",
			sp->center.x, sp->center.y, sp->center.z, sp->radius);
	    }
        } else if (op->type == OBJ_TYPE_POLY) {

		/* must do in model space: */
    	    if (Flagged(op->flags, FLAG_TEXGEN_CYLINDER)) {
                op->sphere = create_bounding_sphere(op);	/* need bounding volume */
        	RPGenerateCylindricalTexcoords(op);
		free(op->sphere);
		op->sphere = (Sphere_t *) NULL;
    	    }

    	    if (Flagged(op->flags, FLAG_TEXGEN_SPHERE)) {
        	RPGenerateSphericalTexcoords(op);
    	    }

  	    RPProcessAllTriangles(op, op->tri_count, op->tris, doProject);

		/* create after transform, to accelerate intersection testing: */
            op->sphere = create_bounding_sphere(op);

	 	/* if smooth shaded and no vertex normal, allocate and 
		 * compute vertex normals 
		 */
    	    if (!Flagged(op->flags, FLAG_FLATSHADE) &&
                !Flagged(op->flags, FLAG_VERTNORM)) {
        	    RPCalculateVertexNormals(op, TRUE);
	    }

	    if (Flagged(RPScene.flags, FLAG_VERBOSE2)) {
	        sp = op->sphere;
		fprintf(stderr, "final poly bounding sphere at (%8.3f,%8.3f,%8.3f) radius %f\n",
			sp->center.x, sp->center.y, sp->center.z, sp->radius);
	    }
        } else {     /* can't happen */
            fprintf(stderr,"%s : ERROR : %s : %d : unknown object type %d\n",
            		program_name, __FILE__, __LINE__, op->type);
        }
    }
}


/* 
 * calculate the bounding sphere for this object
 * returns NULL is object is a sphere (no need for bouding sphere)
 * stupid simple (and inefficient algorithm) but only called once
 * per object per scene, no big deal 
 */
static Sphere_t * 
create_bounding_sphere(Object_t *op)
{
    Sphere_t	*sp;
    Vtx_t	*vp;
    xyz_t	center;
    int		vcount, i;
    float	len2, maxlen;

    if (op == (Object_t *) NULL) {
	return ((Sphere_t *) NULL); 
    }
    if (op->type == OBJ_TYPE_SPHERE || op->vert_count == 0) {
	return ((Sphere_t *) NULL); 
    }

    vp = op->verts;;
    vcount = op->vert_count;
	
	/* compute center of bounding volume: */

    center.x = center.y = center.z = 0.0;
    for (i=0; i<vcount; i++) {
	center.x += vp[i].pos.x;
	center.y += vp[i].pos.y;
	center.z += vp[i].pos.z;
    }
    center.x /= vcount; center.y /= vcount; center.z /= vcount;

	/* compute radius of bounding volume: */

    maxlen = 0;
    for (i=0; i<vcount; i++) {
        len2 =  Sqr(vp[i].pos.x - center.x) +
        	Sqr(vp[i].pos.y - center.y) +
        	Sqr(vp[i].pos.z - center.z);
	if (len2 > maxlen)
	    maxlen = len2;
    }

    sp = (Sphere_t *) calloc(1, sizeof(Sphere_t));
    
    sp->center.x = center.x;
    sp->center.y = center.y;
    sp->center.z = center.z;
    sp->radius = sqrtf(maxlen);

    return (sp);
}


void
RPDumpObject(Object_t *op)
{
    fprintf(stderr,"Object: id = %d type = %d\n",
		op->id, op->type);
    if (op->type == OBJ_TYPE_SPHERE) {
	fprintf(stderr,"\tobject type SPHERE\n");
	fprintf(stderr,"\tsphere = %08x\n",(int)op->sphere);
    } else {
	fprintf(stderr,"\tobject type TRIANGLE\n");
	fprintf(stderr,"\tbounding sphere = %08x\n",(int)op->sphere);
	fprintf(stderr,"\tpoly data... %d verts, %d tris\n",
		op->vert_count, op->tri_count);
    }
}

