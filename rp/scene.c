
/*
 * File:	RPScene.c
 *
 * This file holds the data structures and functions that manipulate RPScenes.
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
#include <errno.h>
#include <sys/stat.h>

#include "rp.h"

    /* only one scene per image, this is it: */

Scene_t		RPScene;

    /* these are normally accessed via calls in state.c, but are global
     * for convenience/to permit optimizations, fancy buffer access, etc.
     */
rgba_t          RPColorFrameBuffer[MAX_YRES][MAX_XRES];
float           RPDepthFrameBuffer[MAX_YRES][MAX_XRES];


static Camera_t        default_camera = {
    {0.0, 0.0, 500.0},          /* pos */
    {0.0, 0.0, 0.0},            /* center of interest */
    {0.0, 1.0, 0.0},            /* up */
    30.0,                       /* fov */
    MAX_XRES/MAX_YRES,		/* aspect */
    {0.0, 0.0, 0.0},            /* eye */
    {0.0, 0.0, -1.0},           /* eye dir */
    30.0 * DegToRad             /* fov in radians */
};

static Viewport_t      default_viewport = {
    MAX_XRES/4.0, MAX_YRES/4.0, 4096.0, MAX_XRES/4.0, MAX_YRES/4.0, 0.0
};

       /* scissor box */
static Scissor_t       default_scissor_box = { 0, 0, (MAX_XRES-1), (MAX_YRES-1) };

/* for scene input file parser: */
int     _RPmylineno;
char    _RPline_buffer[MAX_FILENAME_LENGTH];
char    _RPinput_file[MAX_FILENAME_LENGTH];

/* variables from the parser: */
extern FILE *yyin;
extern int yydebug;
extern int yyparse(void);

/* initialize the RPScene.... should only be called once, at the begining of time,
 * or unpredictable things might occur...
 */ 
void
RPInitScene(void)
{
    xyz_t       pos, coi, up;
    float       tmp_mtx[4][4];


    RPScene.flags = 0x0;
    RPScene.generic_flags = 0x0;
    RPScene.camera = &default_camera;
    RPScene.viewport = &default_viewport; 
    RPScene.scissor_box = &default_scissor_box; 
	/* obj_list is empty */
    RPScene.hither = 100.0;
    RPScene.yon = 10000.0;
    RPScene.obj_count = 0;
	/* light_list is empty */
    RPScene.light_count = 0;
    RPScene.xres = MAX_XRES/2;
    RPScene.yres = MAX_YRES/2;
    RPScene.num_samples = 1;
    RPScene.background_color.r = RPScene.background_color.g = 0;
    RPScene.background_color.b = RPScene.background_color.a = 0;
    RPScene.fog_color.r = 0.0; RPScene.fog_color.g = 0.0;
    RPScene.fog_color.b = 0.0; RPScene.fog_color.a = 0.0;
    RPScene.fog_start = REALLY_BIG_FLOAT;
    RPScene.fog_end = 0.0;
    RPScene.ambient = 0.0;
    RPScene.output_file = (char *) malloc(strlen(DEFAULT_OUTPUT_FILE)+1);
    strcpy(RPScene.output_file, DEFAULT_OUTPUT_FILE);
    RPScene.background_file = (char *) NULL;

	/* stats counters */
    RPScene.input_polys = 0;
    RPScene.clipped_polys = 0;
    RPScene.trivial_rejected_polys = 0;
    RPScene.tiny_rejected_polys = 0;

        /* set matrices */
    ident_mtx(m_mtx);
    ident_mtx(v_mtx);
    ident_mtx(p_mtx);
    ident_mtx(mv_mtx);
    ident_mtx(mvp_mtx);
    ident_mtx(l_mtx);

        /* set camera */
    pos.x = 0.0; pos.y = 0.0; pos.z = 1000.0;
    coi.x = 0.0; coi.y = 0.0; coi.z = 0.0;
    up.x = 0.0; up.y = 1.0; up.z = 0.0;
    RPSetCamera(pos, coi, up, 20.0, MAX_XRES/MAX_YRES);

    perspective_mtx(tmp_mtx, 20.0, MAX_XRES/MAX_YRES, 100.0, 10000.0);
    load_matrix(tmp_mtx, MTX_TYPE_PROJ);        /* assumes PROJ */

        /* set viewport */
    RPSetViewport(RPScene.xres/2.0, RPScene.yres/2.0, 8192.0,
                  RPScene.xres/2.0, RPScene.yres/2.0, 0.0);

    RPSetScissor(0, 0, (RPScene.xres-1), (RPScene.yres-1));

        /* clear frame buffers */
    RPClearColorFB(NULL);
    RPClearDepthFB(NULL);
}

/* set render flags (called from parser) */
void
RPSetSceneFlags(u32 flags)
{
    Flag(RPScene.flags, flags);
}

/* clear render flags (called from parser) */
void
RPClearSceneFlags(u32 flags)
{
    UnFlag(RPScene.flags, flags);
}

/* set generic (renderer-defined) render flags (called from parser) */
void
RPSetGenericSceneFlags(u32 flags)
{
    Flag(RPScene.generic_flags, flags);
}

/* clear render flags (called from parser) */
void
RPClearGenericSceneFlags(u32 flags)
{
    UnFlag(RPScene.generic_flags, flags);
}


int
RPParseInputFile(char *fname, int debugparse, char *cppdefs)
{
    struct stat         statbuffer;
    char                cppcmd[1024];

    strcpy(_RPinput_file, fname);

    if (stat(_RPinput_file, &statbuffer) != 0) {
        fprintf(stderr,"%s : ERROR : can't open input file [%s].\n",
		program_name,_RPinput_file);
        return (FALSE);
    }

    /* some environments prefer this... "/usr/bin/gcc -x c-header -E "... */
    sprintf(cppcmd,"/usr/bin/cpp %s %s ", cppdefs, _RPinput_file);

    if ((yyin = (FILE *) popen(cppcmd, "r")) == NULL) {
        fprintf(stderr,"%s : ERROR : could not process [%s] errno = %d\n",
                program_name,_RPinput_file,errno);
        return (FALSE);
    }

    yydebug = debugparse;

    fprintf(stderr,"%s : parsing input file [%s]...\n", program_name,_RPinput_file);

    if (yyparse())
        return (FALSE);	/* 0 return means end of file (success) */

    fclose(yyin);

    return (TRUE);
}


