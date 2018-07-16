
/*
 * File:	texture.c
 *
 * This file holds the data structures and functions to manipulate textures.
 *
 * Right now, textures are not completely implemented...
 *
 *     - scene can have max 32 textures (program limit)
 *     - no/poor delete/re-use functionality
 *     - an object can only have 1 texture
 *     - sphere objects have limited texture mapping support
 *     - multisampling is available for screen-space rasterizers 
 *     - reflection and bump mapping are partially implemented
 *     - no mip-mapping
 *
 * Texture files are .BMP files (PPM format was removed...)
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


/* texture list is held in the Scene_t structure...
 * the extra texture in the array is a hack... 
 * we use that texture slot to read in a background image if provided
 */


/* load a texture that is in the BMP format */
static int
texture_bmp_load(int texnum, char *filename, u32 flags, float sscale, float tscale, float soff, float toff)
{
    Texture_t	*tex;
    int		i;

    tex = RPScene.texture_list[texnum];

    /* if texture was in use, destroy it: */
    if (tex != (Texture_t *) NULL) {
        if (tex->tmem != (rgba_t **)NULL) {
	    for (i=0; i<tex->yres; i++) {
	        if (tex->tmem[i] != (rgba_t *)NULL)
		    free((char *)tex->tmem[i]);
	    }
	}
	free((char *)tex->tmem);
        free(tex);
    }

    tex = (Texture_t *) calloc(1, sizeof(Texture_t));

    i = read_bmp(filename, tex);    /* read_bmp sets xres and yres and the pixels */

    if (i == FALSE) {
	fprintf(stderr,"%s : ERROR : could not load BMP texture file %s\n",
		program_name,filename);
	return (FALSE);
    }

    tex->filename = (char *) malloc(strlen(filename)+1);
    strcpy(tex->filename, filename);
    tex->inuse = TRUE;
    tex->flags = flags;
    tex->sscale = sscale;
    tex->tscale = tscale;
    tex->soff = soff;
    tex->toff = toff;

    RPScene.texture_list[texnum] = tex;

    return (TRUE);
}

/* this wrapper function exists to allow multiple image formats, even though we 
 * currently only support one...
 */
int
RPLoadTextureFromFile(int texnum, char *filename, u32 flags, 
	float sscale, float tscale, float soff, float toff)
{
    int		tmptexnum, retval;
    char	*file_ext;

    if (texnum < 0 && RPScene.texture_count < MAX_TEXTURES) {	
	tmptexnum = RPScene.texture_count; 	/* assumes the"next texture" slot */
    } else if (texnum <= MAX_TEXTURES) {
	tmptexnum = texnum;
    } else {
	fprintf(stderr,"%s : ERROR : too many textures! can't load [%s] into slot %d\n",
		program_name,filename,texnum);
	return (FALSE);
    }

    file_ext = strstr(filename,".");
    if (file_ext != (char *) NULL)
	file_ext++;

    retval = texture_bmp_load(tmptexnum, filename, flags, sscale, tscale, soff, toff);
    if (!retval)
	return (FALSE);

    if (texnum < 0) {
       RPScene.texture_count++;
    }

    return (TRUE);
}

/* free a specified texture slot */
void
RPFreeTexture(int texnum)
{
    Texture_t	*tp;
    int		j;

    if (texnum > (MAX_TEXTURES+1)) {
	fprintf(stderr,"%s : ERROR : attempt to free texture slot %d\n",
		program_name, texnum);
	return;
    }

    tp = RPScene.texture_list[texnum];

    if (tp->filename != (char *) NULL) {
	free (tp->filename);
	tp->filename = (char *) NULL;
    }

    if (tp->tmem != (rgba_t **) NULL) {
	for (j=0; j<tp->yres; j++) {
	    free (tp->tmem[j]);
	}
	free (tp->tmem);
    }

    tp->inuse = FALSE;
    tp->flags = 0x0;
    tp->xres = tp->yres = 0;
    tp->sscale = tp->tscale = 0.0;
    tp->soff = tp->toff = 0.0;

    bzero(tp, sizeof(Texture_t));
}

/* release allocated data and reset texture array to empty */
void
RPCleanupTextures(void)
{
    int 	i;

    for (i=0; i<RPScene.texture_count; i++) {

	RPFreeTexture(i);
    }

    RPScene.texture_count = 0;
}

/* return the flags for this texture */
u32
RPGetTextureFlags(int texnum)
{
    Texture_t	*tex;

    tex = RPScene.texture_list[texnum];
    return(tex->flags);
}

/* returns the texture num of the texture named "name" */
int
RPFindTexture(char *name)
{
    Texture_t	*tex;
    int 	i, index = -1;

    if (name == (char *) NULL)
	return (index);

    for (i=0; i<RPScene.texture_count && index<0; i++) {
	tex = RPScene.texture_list[i];

	if (tex != (Texture_t *) NULL) {
	    if (tex->filename != (char *) NULL) {
	        if (strcmp(tex->filename, name) == 0) {
	            index = i;
	        }
	    }
	}
    }

    return (index);
}

/* turn a floating point s,t into an integer x,y index into the
 * texture, accounting for mirror, wrap, etc.
 */
static void
texel_address(Texture_t *tex, float s, float t, float inv_w, int *x, int *y)
{
    float	xf, yf;
    int		scnt, tcnt;

    /* undo perspective correction: */
    if (!NearlyZero(inv_w, EpEpsilon)) {
	s /= inv_w;
	t /= inv_w;
    }

    /* scale texture coordinate: */
    xf = (s + tex->soff) * (float)tex->xres * tex->sscale;
    yf = (t + tex->toff) * (float)tex->yres * tex->tscale;

    /* compute texel, accounting for wrap/mirror/clamp... */
    scnt = Floor(xf)/tex->xres;
    tcnt = Floor(yf)/tex->yres;

    if (Flagged(tex->flags, FLAG_TXT_WRAP)) {
	xf = xf - (scnt * tex->xres);
	yf = yf - (tcnt * tex->yres);
    } else if (Flagged(tex->flags, FLAG_TXT_MIRROR)) {
	if ((scnt % 2) == 0) {
	    xf = xf - (scnt * tex->xres);
	} else {
	    xf = ((scnt + 1) * tex->xres) - xf;
	}

	if ((tcnt % 2) == 0) {
	    yf = yf - (tcnt * tex->yres);
	} else {
	    yf = ((tcnt + 1) * tex->yres) - yf;
	}
    }

    xf = Clamp0x(xf, tex->xres-1);
    yf = Clamp0x(yf, tex->yres-1);

    *x = Floor(xf);
    *y = Floor(yf);
}

/* sample a texture */
rgba_t
RPPointSampleTexture(int texnum, float s, float t, float inv_w)
{
    Texture_t	*tex;
    rgba_t	samp, black = {0, 0, 0, MAX_COLOR_VAL};
    int		x, y;


    tex = RPScene.texture_list[texnum];
    if (tex == (Texture_t *) NULL) {
	fprintf(stderr,"%s : ERROR : %s : %d : NULL texture (%d) in sample func\n",
		program_name, __FILE__,__LINE__,texnum);
	return(black);
    }

    if (!tex->inuse) {	/* texture not loaded */
	fprintf(stderr,"%s : ERROR : %s : %d : texture (%d) not loaded in sample func\n",
		program_name, __FILE__,__LINE__,texnum);
	return(black);
    }

    texel_address(tex, s, t, inv_w, &x, &y);

    samp.r = tex->tmem[y][x].r;
    samp.g = tex->tmem[y][x].g;
    samp.b = tex->tmem[y][x].b;
    samp.a = tex->tmem[y][x].a;

    return(samp);
}

/* filter sample a texture:
 *
 * this is coded needing projection-space DxDs/DyDs and DxDt/DyDt, so it only
 * works for some rendering algorithms
 *
 */
rgba_t
RPFilterSampleTexture(int texnum, float s, float t, float inv_w,
 		      float DxDs, float DyDs, float DxDt, float DyDt,
		      float DxDw, float DyDw)
{
    rgba_t	samp, tx0, tx1, tx2, tx3;

    tx0 = RPPointSampleTexture(texnum, 
			       s-(DxDs*0.5)-(DyDs*0.5), 
			       t-(DxDt*0.5)-(DyDt*0.5), 
			       inv_w-(DxDw*0.5)-(DyDw*0.5));
    tx1 = RPPointSampleTexture(texnum, 
			       s+(DxDs*0.5)-(DyDs*0.5), 
			       t+(DxDt*0.5)-(DyDt*0.5), 
			       inv_w+(DxDw*0.5)-(DyDw*0.5));
    tx2 = RPPointSampleTexture(texnum, 
			       s+(DxDs*0.5)+(DyDs*0.5), 
			       t+(DxDt*0.5)+(DyDt*0.5), 
			       inv_w+(DxDw*0.5)+(DyDw*0.5));
    tx3 = RPPointSampleTexture(texnum, 
			       s-(DxDs*0.5)+(DyDs*0.5), 
			       t-(DxDt*0.5)+(DyDt*0.5), 
			       inv_w-(DxDw*0.5)+(DyDw*0.5));
    
    samp.r = (u8) ((tx0.r + tx1.r + tx2.r + tx3.r) / 4);
    samp.g = (u8) ((tx0.g + tx1.g + tx2.g + tx3.g) / 4);
    samp.b = (u8) ((tx0.b + tx1.b + tx2.b + tx3.b) / 4);
    samp.a = (u8) ((tx0.a + tx1.a + tx2.a + tx3.a) / 4);

    return(samp);
}

/* sample a reflection texture */
rgba_t
RPSampleReflectionTexture(xyz_t *r)
{
    float	major, ts, tt;
    int		texnum = 0;

    if ((Abs(r->x) >= Abs(r->y)) && (Abs(r->x) >= Abs(r->z))) {
	major = r->x;
	ts = r->z;
	tt = r->y;
    } else if ((Abs(r->y) >= Abs(r->x)) && (Abs(r->y) >= Abs(r->z))) {
	major = r->y;
	ts = r->x;
	tt = r->z;
    } else {
	major = r->z;
	ts = r->x;
	tt = r->y;
    }

    if (major != 0.0) {
	ts = ts / major;
	tt = tt / major;
    }

    return(RPPointSampleTexture(texnum, ts, tt, 1.0));
}

/* sample a bumpmap texture */
/*
 * This code is not used, bump mapping not supported
 */
rgba_t
RPSampleBumpTexture(int texnum, float nx, float ny, float nz, float s, float t, float inv_w)
{
    Texture_t	*tex;
    rgba_t	black = {0, 0, 0, 255};
    xyz_t	tn;
    int		x, y;
    float	sgrad, tgrad;

    tex = RPScene.texture_list[texnum];

    if (tex == (Texture_t *) NULL) 	/* texture not loaded */
	return(black);

    if (!tex->inuse) 			/* texture not loaded */
	return(black);

    texel_address(tex, s, t, inv_w, &x, &y);

    /* avoid edge cases (for now) */
    if (x == 0 || x == (tex->xres-1) || y == 0 || y == (tex->yres-1)) {
	sgrad = 0.0;
	tgrad = 0.0;
	tn.x = nx + sgrad;
	tn.y = ny + tgrad;
	tn.z = nz;
    } else {
	/* use only red channel, assume monochrome bumpmap: */

	sgrad = (tex->tmem[y-1][x+1].r +
		 tex->tmem[y][x+1].r +
		 tex->tmem[y+1][x+1].r -
		 tex->tmem[y-1][x-1].r -
		 tex->tmem[y][x-1].r -
		 tex->tmem[y+1][x-1].r)/(6.0f*((float)MAX_COLOR_VAL));

	tgrad = (tex->tmem[y-1][x-1].r +
		 tex->tmem[y-1][x].r +
		 tex->tmem[y-1][x+1].r -
		 tex->tmem[y+1][x-1].r -
		 tex->tmem[y+1][x].r -
		 tex->tmem[y+1][x+1].r)/(6.0f*((float)MAX_COLOR_VAL));
	tn.x = nx + sgrad;
	tn.y = ny + tgrad;
	tn.z = nz;
	vector_normalize(&tn);
    }

/* need to fix this
    return(shade_pixel(tn.x, tn.y, tn.z));
*/
    return (black);
}


/* 
 * take an object and generate spherical tex coords based on the normal 
 *
 * originally written to support simple texture mapping for implicit spheres,
 * you can also apply this to any other polygonal geometry (similar to 
 * OpenGL glTexGen() with GL_SPHERE_MAP
 *
 * (implement other glTexGen() modes TBD...)
 *
 */
void
RPGenerateSphericalTexcoords(Object_t *op)
{
    Vtx_t	*vp;
    float	s, t;
    int		i;

    vp = op->verts;

    for (i=0; i<op->vert_count; i++) {

	s = 0.5 + (atan2f(vp[i].n.z, -vp[i].n.x) / (2.0 * Pi));
        t = 0.5 + (asinf(-vp[i].n.y) / Pi);

	vp[i].s = s;
	vp[i].t = t;
    }
}

/* 
 * take an object and generate cylindrical tex coords 
 *
 */
void
RPGenerateCylindricalTexcoords(Object_t *op)
{
    Sphere_t	*sp = op->sphere;
    Vtx_t	*vp;
    float	s, t;
    int		i;

    if (sp == (Sphere_t *) NULL ||
	sp->radius == 0.0)
	return;

    vp = op->verts;

    for (i=0; i<op->vert_count; i++) {

	s = 0.5 + (atan2f(vp[i].pos.z, -vp[i].pos.x) / (2.0 * Pi));
        t = (vp[i].pos.y - (sp->center.y - sp->radius)) / (2.0*sp->radius);

	vp[i].s = Clamp0x(s, 1.0);
	vp[i].t = Clamp0x(t, 1.0);
    }
}


