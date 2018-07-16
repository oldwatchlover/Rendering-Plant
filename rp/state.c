
/*
 * File:	state.c
 *
 * This file manages the renderer state and performs other useful functions.
 *
 * Most of these functions are called from the input parser
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

#include "rp.h"
#include "line.h"

	    /* current render state for current object, from objects.c */
extern u32	_RPTempObjRenderFlags;	

	/* camera / projection paramters; save for complicated order of calls */
static float	fov = 20.0, 
		aspect = MAX_XRES/MAX_YRES, 
		hither = 100.0, yon = 10000.0;
static xyz_t	camera_pos = {0.0, 0.0, 500.0};
static xyz_t	camera_coi = {0.0, 0.0, 0.0};
static xyz_t	camera_up =  {0.0, 1.0, 0.0};


/* set the output filename and resolution of the frame buffer */
void
RPSetOutput(char *fname, int txres, int tyres)
{
    if (fname == (char *) NULL) {
	fprintf(stderr,"%s : bad output filename [%s].\n",
		program_name, fname);
	return;
    }

    if (RPScene.output_file != (char *) NULL)
        free(RPScene.output_file);

    RPScene.output_file = (char *) malloc(strlen(fname)+1);
        strcpy(RPScene.output_file, fname);
    if ((txres < 0) || (txres > MAX_XRES) || 
	(tyres < 0) || (tyres > MAX_YRES)) {
        fprintf(stderr,"%s : invalid output resolution requested (%dx%d).\n",
		    program_name, txres, tyres);
        RPScene.xres = MAX_XRES;
        RPScene.yres = MAX_YRES;
    } else {
        RPScene.xres = txres;
        RPScene.yres = tyres;
    }
	/* use new resolution to set default viewport and scissor */
    RPSetViewport(RPScene.xres/2.0, RPScene.yres/2.0, yon,
    	          RPScene.xres/2.0, RPScene.yres/2.0, 0.0);
    RPSetScissor(0, 0, (RPScene.xres-1), (RPScene.yres-1));
}

/* set frame buffer to some color value */
void
RPSetBackgroundColor(rgba_t *color)
{
    if (color != (rgba_t *) NULL) {
        RPScene.background_color.r = color->r;
        RPScene.background_color.g = color->g;
        RPScene.background_color.b = color->b;
        RPScene.background_color.a = color->a;
    }
}

/* clear the frame buffer to a color */
void
RPClearColorFB(rgba_t *color)
{
    int		i, j;

	/* if input parameter is NULL, clear to current BG color */
    if (color == (rgba_t *)NULL) {
        for (i=0; i<RPScene.yres; i++) {
	    for (j=0; j<RPScene.xres; j++) {
	        RPColorFrameBuffer[i][j].r = RPScene.background_color.r;
	        RPColorFrameBuffer[i][j].g = RPScene.background_color.g;
	        RPColorFrameBuffer[i][j].b = RPScene.background_color.b;
	        RPColorFrameBuffer[i][j].a = RPScene.background_color.a;
	    }
        }
    } else {
        for (i=0; i<RPScene.yres; i++) {
	    for (j=0; j<RPScene.xres; j++) {
	        RPColorFrameBuffer[i][j].r = color->r;
	        RPColorFrameBuffer[i][j].g = color->g;
	        RPColorFrameBuffer[i][j].b = color->b;
	        RPColorFrameBuffer[i][j].a = color->a;
	    }
        }
	RPSetBackgroundColor(color);
    }
}

/* clear the z-buffer to a value */
void
RPClearDepthFB(float *zval)
{
    float	newval;
    int		i, j;

    if (zval == (float *) NULL) {
	newval = REALLY_BIG_FLOAT;
    } else {
	newval = *zval;
    }

    for (i=0; i<RPScene.yres; i++) {
        for (j=0; j<RPScene.xres; j++) {
            RPDepthFrameBuffer[i][j] = newval;
        }
    }
}

/* test a z value with the depth buffer
 *
 * z buffer is positive floating point values, increasing as we get
 * further from the eye (these are the *projected* z values, not the
 * camera space values, which are negative (looking down -z axis) 
 */
int
RPTestDepthFB(int x, int y, float z)
{
    if ((x >= RPScene.scissor_box->ulx) && (x <= RPScene.scissor_box->lrx) &&
        (y >= RPScene.scissor_box->uly) && (y <= RPScene.scissor_box->lrx)) {
        if (z < RPDepthFrameBuffer[y][x])
            return TRUE;
        else
            return FALSE;
    } else {
        return FALSE;
    }
}

/* put a z value into the depth buffer */
void
RPPutDepthFBPixel(int x, int y, float z)
{
    if ((x >= RPScene.scissor_box->ulx) && (x <= RPScene.scissor_box->lrx) &&
        (y >= RPScene.scissor_box->uly) && (y <= RPScene.scissor_box->lrx)) {
        RPDepthFrameBuffer[y][x] = z;
    }
}



/* load an image into the frame buffer as a background 
 * the user is responsible for making sure the image is the
 * right size.
 *
 * RPSetBackgroundImageFile() is called from the parser, to set the filename
 * for the background image. RPLoadBackgroundImage() is called from the
 * renderer whenever it wants to put the contents of that image file
 * into the frame buffer ("when", depends on the rendering algorithm)
 */
void
RPLoadBackgroundImage(void)
{
    int		i, j, txres, tyres;
    Texture_t	*tex;

    if (!Flagged(RPScene.flags, FLAG_BACKGROUND_IMAGE))
	return;

    /*
     * bit of a hack... read it in as a texture, copy to RPColorFrameBuffer, 
     * then free the temp texture
     */
    i = RPLoadTextureFromFile(MAX_TEXTURES, RPScene.background_file, 0x0, 0.0, 0.0, 0.0, 0.0);
    if (!i) {	/* error loading background image */
        UnFlag(RPScene.flags, FLAG_BACKGROUND_IMAGE);
	return;
    }

    tex = RPScene.texture_list[MAX_TEXTURES];

    txres = Min(tex->xres, RPScene.xres);
    tyres = Min(tex->yres, RPScene.yres);

	/* copy background image starting at upper left hand corner */
    for (i=0; i<tyres; i++) {
        for (j=0; j<txres; j++) {
	    RPColorFrameBuffer[i][j].r = tex->tmem[i][j].r;
	    RPColorFrameBuffer[i][j].g = tex->tmem[i][j].g;
	    RPColorFrameBuffer[i][j].b = tex->tmem[i][j].b;
            RPColorFrameBuffer[i][j].a = MAX_COLOR_VAL;
	}
    }

	/* destroy the temporary texture */
    RPFreeTexture(MAX_TEXTURES);
}

/* called from the input parser */
void
RPSetBackgroundImageFile(char *filename)
{
    if (RPScene.background_file != (char *) NULL)
	free(RPScene.background_file);

    RPScene.background_file = (char *) malloc(strlen(filename)+1);
    strcpy(RPScene.background_file, filename);
    Flag(RPScene.flags, FLAG_BACKGROUND_IMAGE);
}

/* return value from framebuffer, caller should free memory */
rgba_t *
RPGetColorFBPixel(int x, int y)
{
    rgba_t	*sample;

    sample = (rgba_t *) malloc(sizeof(rgba_t));

    if (Flagged(RPScene.flags,FLAG_BACKGROUND_IMAGE)) {
        sample->r = RPColorFrameBuffer[y][x].r;
        sample->g = RPColorFrameBuffer[y][x].g;
        sample->b = RPColorFrameBuffer[y][x].b;
        sample->a = RPColorFrameBuffer[y][x].a;
    } else {
	sample->r = RPScene.background_color.r; 
	sample->g = RPScene.background_color.g; 
	sample->b = RPScene.background_color.b; 
	sample->a = RPScene.background_color.a; 
    }

    return(sample);
}

/* store an rgba value (each component range 0-255) into the frame buffer */
void
RPPutColorFBPixel(int x, int y, int r, int g, int b, int a)
{
    float       f;

    if ((x >= RPScene.scissor_box->ulx) && (x <= RPScene.scissor_box->lrx) && 
	(y >= RPScene.scissor_box->uly) && (y <= RPScene.scissor_box->lrx)) {
        r = Clamp0255(r);
        g = Clamp0255(g);
        b = Clamp0255(b);
        a = Clamp0255(a);

        if (a == MAX_COLOR_VAL) {
            RPColorFrameBuffer[y][x].r = (u8) r;
            RPColorFrameBuffer[y][x].g = (u8) g;
            RPColorFrameBuffer[y][x].b = (u8) b;
            RPColorFrameBuffer[y][x].a = (u8) a;
        } else {    /* do alpha-blending */
            f = (float)a/(float)MAX_COLOR_VAL;
            RPColorFrameBuffer[y][x].r = (u8) Clamp0255(f*r + 
						(1.0-f)*RPColorFrameBuffer[y][x].r);
            RPColorFrameBuffer[y][x].g = (u8) Clamp0255(f*g + 
						(1.0-f)*RPColorFrameBuffer[y][x].g);
            RPColorFrameBuffer[y][x].b = (u8) Clamp0255(f*b + 
						(1.0-f)*RPColorFrameBuffer[y][x].b);
            RPColorFrameBuffer[y][x].a = (u8) Clamp0255(f*a + 
						(1.0-f)*RPColorFrameBuffer[y][x].a);
        }
    }
}

/* write frame buffer to a file */
int
RPWriteColorFB(void)
{
   fprintf(stderr,"%s : creating output file [%s]\n",
	    program_name, RPScene.output_file);

   return (write_bmp(RPScene.output_file, RPScene.xres, RPScene.yres));
}

/* set object render flags (called from parser) */
void
RPSetObjectFlags(u32 flags)
{
    Flag(_RPTempObjRenderFlags, flags);
}

/* clear object render flags (called from parser) */
void
RPClearObjectFlags(u32 flags)
{
    UnFlag(_RPTempObjRenderFlags, flags);
}

/* a helper function that resets the camera and projection parameters...
 * different rendering algorihtms make slighly different use (or ignore) some
 * paramaters, so the input scene description gathers this from a couple different
 * commands.. this function ties it all together. (see documentation for the
 * restrictions and interactions of the input commands)
 */
static void
update_camera_and_projection(void)
{
    float	tmp_mtx[4][4];

    RPScene.camera->pos.x = camera_pos.x; 
    RPScene.camera->pos.y = camera_pos.y; 
    RPScene.camera->pos.z = camera_pos.z; 

    RPScene.camera->coi.x = camera_coi.x; 
    RPScene.camera->coi.y = camera_coi.y; 
    RPScene.camera->coi.z = camera_coi.z; 

    RPScene.camera->up.x = camera_up.x; 
    RPScene.camera->up.y = camera_up.y; 
    RPScene.camera->up.z = camera_up.z; 
    RPScene.camera->fov = fov;
    RPScene.camera->aspect = aspect;

    RPScene.hither = hither;
    RPScene.yon = yon;

	/* fix some internal variables: */
    RPScene.camera->eye.x = 0.0; 
    RPScene.camera->eye.y = 0.0; 
    RPScene.camera->eye.z = 0.0; /* after xform */

    RPScene.camera->dir.x = 0.0; 
    RPScene.camera->dir.y = 0.0; 
    RPScene.camera->dir.z = -1.0; /* look dir */

    RPScene.camera->fovr = DegToRad * fov;

        /* create View Matrix */
    lookat_mtx(tmp_mtx, 
	       RPScene.camera->pos.x, RPScene.camera->pos.y, RPScene.camera->pos.z,
	       RPScene.camera->coi.x, RPScene.camera->coi.y, RPScene.camera->coi.z,
	       RPScene.camera->up.x, RPScene.camera->up.y, RPScene.camera->up.z);
    load_matrix(tmp_mtx, MTX_TYPE_VIEW);

        /* create Proj Matrix */
    perspective_mtx(tmp_mtx, fov, aspect, hither, yon);
    load_matrix(tmp_mtx, MTX_TYPE_PROJ);        /* assumes PROJ */
}

/* set the scale for the depth buffer range. hither and yon in a properly
 * constructed projection matrix will scale the z values for the frame buffer.
 * our framebuffer is floating point, so this isn't too critical... this architecture
 * left over from GPU driver implementation that used fixed-point math and fancy
 * tricks to get the most out of the zbuffer range.
 */ 
void
RPSetDepthRange(float near, float far)
{
    hither = near;
    yon = far;

    update_camera_and_projection();
}


/* set camera (called from parser) */
void
RPSetCamera(xyz_t pos, xyz_t coi, xyz_t up, float fovy, float asp)
{
	/* save parameters so we can reset on related commands: */
    camera_pos.x = pos.x; camera_pos.y = pos.y; camera_pos.z = pos.z;
    camera_coi.x = coi.x; camera_coi.y = coi.y; camera_coi.z = coi.z;
    camera_up.x = up.x; camera_up.y = up.y; camera_up.z = up.z;

    fov = fovy; 

	/* if asp < 0.0, just change hither/yon */
    if (asp > 0.0) {
        aspect = asp; 
    }

    update_camera_and_projection();
}

/* set projection (called from parser) */
void
RPSetProjection(float asp, float near, float far)
{
	/* if aspect < 0.0, just change hither/yon */
    if (asp > 0.0) {
        aspect = asp; 
    }

	/* if near < far, just change aspect */
    if (near < far) {
        hither = near; 
	yon = far;
    }

    update_camera_and_projection();
}

/* set fog (called from parser) */
void
RPSetFog(float start, float end, float r, float g, float b, float a)
{
	/* fog values are positive, "distance from the eye"...
	 * after MV transform to eye space, we are looking down the 
	 * -z axis, so all z values are negative (and getting more negative
	 * farther away. So we must flip the sign to make the math in the 
	 * shader work (rather than explain all of this to the user...)
	 */

    if (start > end) {
	fprintf(stderr,"WARNING : %s : fog start > fog end (%f > %f)\n",
		program_name, start, end);
    }

    RPScene.fog_start = -start;
    RPScene.fog_end = -end;

    RPScene.fog_color.r = r;
    RPScene.fog_color.g = g;
    RPScene.fog_color.b = b;
    RPScene.fog_color.a = a;
}


/* set viewport (called from parser) */
void
RPSetViewport(float sx, float sy, float sz, float tx, float ty, float tz)
{
    RPScene.viewport->sx = sx;
    RPScene.viewport->sy = sy;
    RPScene.viewport->sz = sz;
    RPScene.viewport->tx = tx;
    RPScene.viewport->ty = ty;
    RPScene.viewport->tz = tz;
}

/* set screen scissor box (called from the parser) */
/* if you use the proper fb access functions above, they will test with the scissor
 * box for your desired effect (or to prevent rendering outside the valid framebuffer)
 */
void
RPSetScissor(int tulx, int tuly, int tlrx, int tlry)
{
    RPScene.scissor_box->ulx = tulx;
    RPScene.scissor_box->uly = tuly;
    RPScene.scissor_box->lrx = tlrx;
    RPScene.scissor_box->lry = tlry;
}

static rgba_t	line_color = {255, 0, 0, 255};

/* pixel plotting function for line drawing routine */
static void
plot_pixel(int x, int y, int color)
{
    float	weight;

    weight = (float)color/255.0;

    if (x < 0 || y < 0 || x >= RPScene.xres || y >= RPScene.yres)
        return;

    RPPutColorFBPixel(x, y, (int) ((float)line_color.r * weight), 
                      (int) ((float)line_color.g * weight), 
                      (int) ((float)line_color.b * weight), 
                      (int) ((float)line_color.a * weight));
}

/* draw a line in the frame buffer. useful for debugging.
 * x,y input is screen coordinates, useaa if true, uses the 
 * slightly more expensive anti-aliased line algorithm.
 * color is raw pixel values, 8/8/8/8
 */
void
RPDrawColorFBLine(int x1, int y1, int x2, int y2, rgba_t color, int useaa)
{
    line_color.r = color.r;
    line_color.g = color.g;
    line_color.b = color.b;
    line_color.a = color.a;

    if (useaa)
        abresline(x1, y1, x2, y2, plot_pixel);
    else
        bresline(x1, y1, x2, y2, plot_pixel);
}


