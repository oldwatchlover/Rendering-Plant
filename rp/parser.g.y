%{

/*
 * File:	parser.g.y
 *
 * The yacc grammar parser for Rendering Plant scene input.
 *
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

/* for linux version, increase this: */
#undef YYMAXDEPTH
#define YYMAXDEPTH  16384
#undef YYSTACKSIZE
#define YYSTACKSiZE YYMAXDEPTH

static char     error_buffer[MAX_FILENAME_LENGTH];
static float	temp_mtx[4][4];
static Vtx_t	temp_vtx;
static int 	tvcnt = 0;
static Tri_t	temp_tri;
static int 	ttcnt = 0;

/* flags are held here temporarily during parsing, these should be 
 * cleared when the appropriate command is finished
 */
static int	mtx_flags = 0;
static u32	temp_txt_flags = 0;

static void	expandstring(char *s);
//static void	yywarn(char *string);
static void	yyerror(char *s);

/* from main.c */
extern int _RPmylineno;
extern char _RPline_buffer[];
extern char _RPinput_file[];

int 	yylex(void); /* added this to quiet warnings with bison -y */

%}

%union {
	int		integer;
	float		floater;
	char		*string;
	unsigned int	uint;
}

/* syntax pieces: */
%token	<integer>	FIRSTTOKEN	/* must be first */
%token	<integer>	OP_PAREN
%token	<integer>	CL_PAREN
%token	<integer>	OP_BRACKET
%token	<integer>	CL_BRACKET
%token	<integer>	OP_CURLY
%token	<integer>	CL_CURLY
%token	<integer>	COMMA
%token	<integer>	SEMICOLON
%token	<integer>	INTEGER
%token	<floater>	FLOATER
%token	<string>	QSTRING

/* iexpression/fexpression pieces */
%token	<integer>	NOT_OP
%token	<integer>	AND_OP
%token	<integer>	OR_OP
%token	<integer>	XOR_OP
%token	<integer>	RSHFT_OP
%token	<integer>	LSHFT_OP
%token	<integer>	MULT_OP
%token	<integer>	DIV_OP
%token	<integer>	MOD_OP
%token	<integer>	PLUS_OP
%token	<integer>	MINUS_OP

/* command tokens */
%token	<integer>	SCENEFLAGS
%token	<integer>	OBJFLAGS
%token	<integer>	GENERICFLAGS
%token	<integer>	MATRIX
%token	<integer>	LIGHT
%token	<integer>	SPOTLIGHT
%token	<integer>	MATERIAL
%token	<integer>	COLOR
%token	<integer>	AMBIENT
%token	<integer>	DIFFUSE
%token	<integer>	SPECULAR
%token	<integer>	HIGHLIGHT
%token	<integer>	SHINY
%token	<integer>	REFLECTION
%token	<integer>	REFRACTION
%token	<integer>	TEXNAME
%token	<integer>	FOG_CMD
%token	<integer>	CLEAR_BACK
%token	<integer>	BACKGROUND
%token  <integer>       VIEWPORT
%token  <integer>       SCISSOR
%token	<integer>	OUTPUT
%token	<integer>	VERTEX
%token	<integer>	TEXTURE_FILE
%token	<integer>	SPHERE
%token	<integer>	TRILIST
%token	<integer>	LOADOBJ

/* for any flag field: */
%token	<integer>	ALLFLAGS

/* global scene flag tokens */
%token	<integer>	VERBOSE
%token	<integer>	VERBOSE2
%token	<integer>	NOSHADOW
%token	<integer>	FOG
%token	<integer>	ZBUFFER
%token	<integer>	MULTISAMPLE
%token	<integer>	PERSPTEXTURE

/* object flag tokens */
%token	<integer>	CULL_BACK
%token	<integer>	CULL_FRONT
%token	<integer>	TEXTURE
%token	<integer>	LIGHTING
%token	<integer>	FLATSHADE
%token	<integer>	SMOOTHSHADE
%token	<integer>	RANDSHADE
%token	<integer>	POLYSHADE
%token	<integer>	VERTSHADE
%token	<integer>	VERTNORM
%token	<integer>	REFLECT
%token	<integer>	BUMP
%token	<integer>	TEXGENSPHERE
%token	<integer>	TEXGENCYLINDER

/* generic render flag tokens */
%token	<integer>	RENDER01
%token	<integer>	RENDER02
%token	<integer>	RENDER03
%token	<integer>	RENDER04
%token	<integer>	RENDER05
%token	<integer>	RENDER06
%token	<integer>	RENDER07
%token	<integer>	RENDER08

/* matrix flag tokens: */
%token	<integer>	MTX_PUSH
%token	<integer>	MTX_PROJECTION
%token	<integer>	MTX_MODEL
%token	<integer>	MTX_VIEW
%token	<integer>	MTX_LOAD
%token	<integer>	MTX_MULT

/* texture flag tokens */
%token	<integer>	TXT_CLAMP
%token	<integer>	TXT_WRAP
%token	<integer>	TXT_MIRROR
%token	<integer>	TXT_FILT
%token	<integer>	TXT_MODULATE

/* high-level matrix commands: */
%token	<integer>	IDENTITY
%token  <integer>       PERSPECTIVE
%token  <integer>       ORTHOGRAPHIC
%token  <integer>       DEPTHRANGE
%token	<integer>	CAMERA
%token	<integer>	POP
%token	<integer>	SCALE
%token	<integer>	ROTATE
%token	<integer>	TRANSLATE

/* complex command pieces */
%type   <integer>       iexpression
%type   <floater>       fexpression
%type   <integer>       mtxtype
%type   <integer>       pointlist
%type   <integer>       point
%type   <integer>       trilist
%type   <integer>       tri
%type   <integer>       sceneflaglist
%type   <integer>       sceneflagval
%type   <integer>       objflaglist
%type   <integer>       objflagval
%type   <integer>       mtxflaglist
%type   <integer>       mtxflagval
%type   <integer>       txtflaglist
%type   <integer>       txtflagval

%token	<integer>	LASTTOKEN	/* must be last */

/* for instruction rules: */

/* Expression Precedence:
 *
 * we have 3 precedence levels:
 *
 *	least binding, lowest precedence:	binary +,-
 *						binary *,/,%,<<,>>,^,&,|
 *	most binding, highest precedence:	unary  -,~
 *
 */
%left	MINUS_OP PLUS_OP
%left	MULT_OP DIV_OP MOD_OP LSHFT_OP RSHFT_OP XOR_OP AND_OP OR_OP
%left	NOT_OP
%nonassoc	UMINUS UPLUS

%start scene

%%

scene:
	    commandStream
	{
	}
	;
	
commandStream:
	    command
	{
	}
	|   scene command
	{
	}
	;
	
command:
          SCENEFLAGS OP_PAREN sceneflaglist CL_PAREN SEMICOLON
	{
		/* do nothing, processed by sceneflaglist rules */
	}
        |  OBJFLAGS OP_PAREN objflaglist CL_PAREN SEMICOLON
	{
		/* do nothing, processed by objflaglist rules */
	}
        |  GENERICFLAGS OP_PAREN genericflaglist CL_PAREN SEMICOLON
	{
		/* do nothing, processed by genericflaglist rules */
	}
        | MATRIX mtxtype mtxflaglist OP_CURLY fexpression COMMA fexpression COMMA fexpression COMMA fexpression COMMA fexpression COMMA fexpression COMMA fexpression COMMA fexpression COMMA fexpression COMMA fexpression COMMA fexpression COMMA fexpression COMMA fexpression COMMA fexpression COMMA fexpression COMMA fexpression CL_CURLY SEMICOLON
	{
	    temp_mtx[0][0] = $5;
	    temp_mtx[0][1] = $7;
	    temp_mtx[0][2] = $9;
	    temp_mtx[0][3] = $11;

	    temp_mtx[1][0] = $13;
	    temp_mtx[1][1] = $15;
	    temp_mtx[1][2] = $17;
	    temp_mtx[1][3] = $19;

	    temp_mtx[2][0] = $21;
	    temp_mtx[2][1] = $23;
	    temp_mtx[2][2] = $25;
	    temp_mtx[2][3] = $27;

	    temp_mtx[3][0] = $29;
	    temp_mtx[3][1] = $31;
	    temp_mtx[3][2] = $33;
	    temp_mtx[3][3] = $35;

	    load_matrix(temp_mtx, mtx_flags);
	    mtx_flags = 0;
	}
        | POP OP_PAREN mtxtype CL_PAREN SEMICOLON
	{
	    pop_matrix(mtx_flags);
	    mtx_flags = 0;
	}
        | TEXTURE_FILE OP_PAREN QSTRING COMMA txtflaglist COMMA fexpression COMMA fexpression COMMA fexpression COMMA fexpression CL_PAREN SEMICOLON
	{
	/* texture() command with optional flags */
            char        *fmt;
	    float	sscale, tscale, soff, toff;

	    fmt = $3;
	    sscale = $7;
	    tscale = $9;
	    soff = $11;
	    toff = $13;

	    expandstring(fmt);
	    RPLoadTextureFromFile(-1, fmt, temp_txt_flags, sscale, tscale, soff, toff);
	    temp_txt_flags = 0;
	}
        | TEXTURE_FILE OP_PAREN QSTRING COMMA fexpression COMMA fexpression COMMA fexpression COMMA fexpression CL_PAREN SEMICOLON
	{
	/* texture() command with no flags */
            char        *fmt;
	    float	sscale, tscale, soff, toff;

	    fmt = $3;
	    sscale = $5;
	    tscale = $7;
	    soff = $9;
	    toff = $11;

	    expandstring(fmt);
	    RPLoadTextureFromFile(-1, fmt, 0x0, sscale, tscale, soff, toff);
	    temp_txt_flags = 0x0;
	}
        | TEXTURE_FILE OP_PAREN iexpression COMMA QSTRING COMMA txtflaglist COMMA fexpression COMMA fexpression COMMA fexpression COMMA fexpression CL_PAREN SEMICOLON
	{
	/* texture command with texture slot id and optional flags */
            char        *fmt;
	    float	sscale, tscale, soff, toff;
	    int		texnum;

	    texnum = $3;
	    fmt = $5;
	    sscale = $9;
	    tscale = $11;
	    soff = $13;
	    toff = $15;

	    expandstring(fmt);
	    RPLoadTextureFromFile(texnum, fmt, temp_txt_flags, 
			 sscale, tscale, soff, toff);
	    temp_txt_flags = 0;
	}
        | TEXTURE_FILE OP_PAREN iexpression COMMA QSTRING COMMA fexpression COMMA fexpression COMMA fexpression COMMA fexpression CL_PAREN SEMICOLON
	{
	/* texture command with texture slot id and no flags */
            char        *fmt;
	    float	sscale, tscale, soff, toff;
	    int		texnum;

	    texnum = $3;
	    fmt = $5;
	    sscale = $7;
	    tscale = $9;
	    soff = $11;
	    toff = $13;

	    expandstring(fmt);
	    RPLoadTextureFromFile(texnum, fmt, 0x0, sscale, tscale, soff, toff);
	    temp_txt_flags = 0x0;
	}
        |   SPHERE OP_PAREN fexpression COMMA fexpression COMMA fexpression COMMA fexpression CL_PAREN SEMICOLON
	{
	    xyz_t	center;
	    float	radius;

	    center.x = $3;
	    center.y = $5;
	    center.z = $7;
	    radius = $9;

	    RPAddSphere(center, radius);
	}
        |   LOADOBJ OP_PAREN QSTRING CL_PAREN SEMICOLON
        {
	/* load a Wavefront .obj file for polygonal geometry */
            char        *fmt;

	    fmt = $3;

	    expandstring(fmt);
	    RPReadObjectFromFile(fmt);
	}
        | TRILIST OP_BRACKET iexpression CL_BRACKET OP_CURLY trilist CL_CURLY SEMICOLON
	{
	    int 	tcnt;

	    tcnt = $3;
	    if (tcnt > MAX_TRIS) {
		sprintf(error_buffer,"%s : too many triangles in trilist (%d).",
                        "ERROR", tcnt);
		yyerror(error_buffer);
	    } else {
		RPCloseTriangleList(tcnt);
            }
	    ttcnt = 0;	/* reset temp tri buffer */
	}
        |   VERTEX OP_BRACKET iexpression CL_BRACKET OP_CURLY pointlist CL_CURLY SEMICOLON
	{
	    int 	vcnt;

	    vcnt = $3;
	    if (vcnt > MAX_VERTS) {
		sprintf(error_buffer,"%s : too many vertices (%d).",
                        "ERROR", vcnt);
		yyerror(error_buffer);
	    } else {
		RPCloseVertexList(vcnt);
            }
	    tvcnt = 0;	/* reset temp points buffer */
	}
        |   VIEWPORT OP_PAREN fexpression COMMA fexpression COMMA fexpression COMMA fexpression COMMA fexpression COMMA fexpression CL_PAREN SEMICOLON
        {
            float       sx, sy, sz, tx, ty, tz;
/*
 * This command is no longer necessary... the viewport is set using the 
 * xres/yres of the output command (or the default) and the zrange is set
 * by the depthrange command (hither/yon for the perspective projection)
 *
 * If you desire custom viewport settings, make sure this command is in the
 * input stream AFTER any output(), camera(), perspective() and depthrange()
 * commands.
 */
            sx = $3;
            sy = $5;
            sz = $7;
            tx = $9;
            ty = $11;
            tz = $13;

            RPSetViewport(sx, sy, sz, tx, ty, tz);
        }
        |   SCISSOR OP_PAREN iexpression COMMA iexpression COMMA iexpression COMMA iexpression CL_PAREN SEMICOLON
        {
/*
 * This command is no longer necessary... the scissor box is set using the 
 * xres/yres of the output command (or the default).
 *
 * If you desire custom scissor settings, make sure this command is in the
 * input stream AFTER any output() command.
 */
            int         tulx, tuly, tlrx, tlry;

            tulx = $3;
            tuly = $5;
            tlrx = $7;
            tlry = $9;

            RPSetScissor(tulx, tuly, tlrx, tlry);
        }
        |   LIGHT OP_PAREN fexpression COMMA fexpression COMMA fexpression COMMA fexpression COMMA fexpression COMMA fexpression CL_PAREN SEMICOLON
	{
            xyz_t	pos;
	    Colorf_t	col;

	    pos.x = $3;
	    pos.y = $5;
	    pos.z = $7;
	    col.r = $9;
	    col.g = $11;
	    col.b = $13;

	    RPSetLight(pos, col);
	}
        |   SPOTLIGHT OP_PAREN fexpression COMMA fexpression COMMA fexpression COMMA fexpression COMMA fexpression COMMA fexpression COMMA fexpression COMMA fexpression COMMA fexpression COMMA fexpression COMMA fexpression COMMA fexpression CL_PAREN SEMICOLON
	{
            xyz_t	pos, coi;
	    Colorf_t	col;
            float	fov, focus, range;

	    pos.x = $3;
	    pos.y = $5;
	    pos.z = $7;

	    coi.x = $9;
	    coi.y = $11;
	    coi.z = $13;

	    fov = $15;
	    focus = $17;
 	    range = $19;

	    col.r = $21;
	    col.g = $23;
	    col.b = $25;

	    RPSetSpotLight(pos, coi, fov, focus, range, 1.0, col);
	}
	/* material() commands have a keyword, with the attribute they set */
	| MATERIAL OP_PAREN COLOR COMMA fexpression COMMA fexpression COMMA fexpression COMMA fexpression CL_PAREN SEMICOLON
        {
	    Colorf_t	color;

	    color.r = $5;
	    color.g = $7;
	    color.b = $9;
	    color.a = $11;

	    RPSetMaterialColor(color);
	}
	| MATERIAL OP_PAREN AMBIENT COMMA fexpression COMMA fexpression COMMA fexpression COMMA fexpression CL_PAREN SEMICOLON
        {
	    Colorf_t	color;

	    color.r = $5;
	    color.g = $7;
	    color.b = $9;
	    color.a = $11;

	    RPSetMaterialAmbient(color);
	}
	| MATERIAL OP_PAREN DIFFUSE COMMA fexpression COMMA fexpression COMMA fexpression COMMA fexpression CL_PAREN SEMICOLON
        {
	    Colorf_t	color;

	    color.r = $5;
	    color.g = $7;
	    color.b = $9;
	    color.a = $11;

	    RPSetMaterialDiffuse(color);
	}
	| MATERIAL OP_PAREN SPECULAR COMMA fexpression COMMA fexpression COMMA fexpression COMMA fexpression CL_PAREN SEMICOLON
        {
	    Colorf_t	color;

	    color.r = $5;
	    color.g = $7;
	    color.b = $9;
	    color.a = $11;

	    RPSetMaterialSpecular(color);
	}
	| MATERIAL OP_PAREN HIGHLIGHT COMMA fexpression COMMA fexpression COMMA fexpression COMMA fexpression CL_PAREN SEMICOLON
        {
	    Colorf_t	color;

	    color.r = $5;
	    color.g = $7;
	    color.b = $9;
	    color.a = $11;

	    RPSetMaterialHighlight(color);
	}
	| MATERIAL OP_PAREN SHINY COMMA fexpression CL_PAREN SEMICOLON
        {
	    float	value;

	    value = $5;

	    RPSetMaterialShiny(value);
	}
	| MATERIAL OP_PAREN REFLECTION COMMA fexpression CL_PAREN SEMICOLON
        {
	    float	value;

	    value = $5;

	    RPSetMaterialReflection(value);
	}
	| MATERIAL OP_PAREN REFRACTION COMMA fexpression CL_PAREN SEMICOLON
        {
	    float	value;

	    value = $5;

	    RPSetMaterialRefraction(value);
	}
	| MATERIAL OP_PAREN TEXNAME COMMA QSTRING CL_PAREN SEMICOLON
        {
	    char	*texname;

	    texname = $5;

	    expandstring(texname);
	    RPSetMaterialTexture(texname);
	}
        |   FOG_CMD OP_PAREN fexpression COMMA fexpression COMMA fexpression COMMA fexpression COMMA fexpression COMMA fexpression CL_PAREN SEMICOLON
	{
	    float	fstart, fend;
	    float	r, g, b, a;

	    fstart = $3;
	    fend = $5;
	    r = $7;
	    g = $9;
	    b = $11;
	    a = $13;

	    RPSetFog(fstart, fend, r, g, b, a);
	}
        |   FOG_CMD OP_PAREN fexpression COMMA fexpression COMMA fexpression COMMA fexpression COMMA fexpression CL_PAREN SEMICOLON
	{	/* handle a fog command with no alpha (assume 1.0) */
	    float	fstart, fend;
	    float	r, g, b, a;

	    fstart = $3;
	    fend = $5;
	    r = $7;
	    g = $9;
	    b = $11;
	    a = 1.0;

	    RPSetFog(fstart, fend, r, g, b, a);
	}
        |   CLEAR_BACK OP_PAREN fexpression COMMA fexpression COMMA fexpression COMMA fexpression CL_PAREN SEMICOLON
        {
 	    Colorf_t	tcolor;
	    rgba_t	clear_color;

	    tcolor.r = $3;
	    tcolor.g = $5;
	    tcolor.b = $7;
	    tcolor.a = $9;

	    clear_color.r = (int) Clamp0255(tcolor.r*255.0);
	    clear_color.g = (int) Clamp0255(tcolor.g*255.0);
	    clear_color.b = (int) Clamp0255(tcolor.b*255.0);
	    clear_color.a = (int) Clamp0255(tcolor.a*255.0);

	    RPSetBackgroundColor(&clear_color);
	}
        |   CLEAR_BACK OP_PAREN fexpression COMMA fexpression COMMA fexpression CL_PAREN SEMICOLON
	{	/* handle a clear command with no alpha (assume 0.0) */
 	    Colorf_t	tcolor;
	    rgba_t	clear_color;

	    tcolor.r = $3;
	    tcolor.g = $5;
	    tcolor.b = $7;
	    tcolor.a = 0.0;

	    clear_color.r = (int) Clamp0255(tcolor.r*255.0);
	    clear_color.g = (int) Clamp0255(tcolor.g*255.0);
	    clear_color.b = (int) Clamp0255(tcolor.b*255.0);
	    clear_color.a = (int) Clamp0255(tcolor.a*255.0);

	    RPSetBackgroundColor(&clear_color);
	}
        |   BACKGROUND OP_PAREN QSTRING CL_PAREN SEMICOLON
        {
            char        *fmt;

	    fmt = $3;

	    expandstring(fmt);
	    RPSetBackgroundImageFile(fmt);
	}
        |   OUTPUT OP_PAREN QSTRING COMMA iexpression COMMA iexpression CL_PAREN SEMICOLON
        {
            char        *fmt;
	    int		xres, yres;

	    fmt = $3;
	    xres = $5;
	    yres = $7;

	    expandstring(fmt);
	    RPSetOutput(fmt, xres, yres);
	}
        /* high level matrix functions: */
        |   IDENTITY OP_PAREN mtxtype mtxflaglist CL_PAREN SEMICOLON
        {
	    float	tmp_mtx[4][4];

	    ident_mtx(tmp_mtx);
	    load_matrix(tmp_mtx, mtx_flags);
	    mtx_flags = 0;
	}
        |   PERSPECTIVE OP_PAREN fexpression COMMA fexpression COMMA fexpression COMMA fexpression CL_PAREN SEMICOLON
        {
            float       tmp_mtx[4][4];
            float       fovy, aspect, near, far;

            fovy = $3;
            aspect = $5;
            near = $7;
            far = $9;

            perspective_mtx(tmp_mtx, fovy, aspect, near, far);
            load_matrix(tmp_mtx, MTX_TYPE_PROJ);        /* assumes PROJ */
            mtx_flags = 0;
        }
        |   ORTHOGRAPHIC OP_PAREN fexpression COMMA fexpression COMMA fexpression COMMA fexpression COMMA fexpression COMMA fexpression CL_PAREN SEMICOLON
        {
            float       tmp_mtx[4][4];
            float       l, r, b, t, n, f;

            l = $3;
            r = $5;
            b = $7;
            t = $9;
            n = $11;
            f = $13;

            ortho_mtx(tmp_mtx, l, r, b, t, n, f);
            load_matrix(tmp_mtx, MTX_TYPE_PROJ);        /* assumes PROJ */
            mtx_flags = 0;
        }
	| DEPTHRANGE OP_PAREN fexpression COMMA fexpression CL_PAREN SEMICOLON
        {
	     float	near, far;

	     near = $3;
	     far = $5;

	     RPSetDepthRange(near, far);
        }
        |   CAMERA OP_PAREN fexpression COMMA fexpression COMMA fexpression COMMA fexpression COMMA fexpression COMMA fexpression COMMA fexpression COMMA fexpression COMMA fexpression COMMA fexpression COMMA fexpression CL_PAREN SEMICOLON
        {
            xyz_t	pos, coi, up;
            float       fovy, aspect;

	    pos.x = $3;
	    pos.y = $5;
	    pos.z = $7;
	    coi.x = $9;
	    coi.y = $11;
	    coi.z = $13;
	    up.x = $15;
	    up.y = $17;
	    up.z = $19;
            fovy = $21;
            aspect = $23;
	    RPSetCamera(pos, coi, up, fovy, aspect);
	    RPSetProjection(aspect, 1.0, 0.0);
	}
        |   SCALE OP_PAREN fexpression COMMA fexpression COMMA fexpression CL_PAREN SEMICOLON
        {
	    float	tmp_mtx[4][4];
	    float	x, y, z;

	    x = $3;
	    y = $5;
	    z = $7;

	    scale_mtx(tmp_mtx, x, y, z);
	    load_matrix(tmp_mtx, MTX_TYPE_MODEL | MTX_FLAG_MULT); /* no flags, assumes MODEL matrix */
	    mtx_flags = 0;
	}
        |   SCALE OP_PAREN mtxtype mtxflaglist COMMA fexpression COMMA fexpression COMMA fexpression CL_PAREN SEMICOLON
        {
	    float	tmp_mtx[4][4];
	    float	x, y, z;

	    x = $6;
	    y = $8;
	    z = $10;

	    scale_mtx(tmp_mtx, x, y, z);
	    load_matrix(tmp_mtx, mtx_flags);
	    mtx_flags = 0;
	}
        |   ROTATE OP_PAREN fexpression COMMA fexpression COMMA fexpression COMMA fexpression CL_PAREN SEMICOLON
        {
	    float	tmp_mtx[4][4];
	    float	a, x, y, z;

	    a = $3;
	    x = $5;
	    y = $7;
	    z = $9;

	    rotate_mtx(tmp_mtx, a, x, y, z);
	    load_matrix(tmp_mtx, MTX_TYPE_MODEL | MTX_FLAG_MULT); /* no flags, assumes MODEL matrix */
	    mtx_flags = 0;
	}
        |   ROTATE OP_PAREN mtxtype mtxflaglist COMMA fexpression COMMA fexpression COMMA fexpression COMMA fexpression CL_PAREN SEMICOLON
        {
	    float	tmp_mtx[4][4];
	    float	a, x, y, z;

	    a = $6;
	    x = $8;
	    y = $10;
	    z = $12;

	    rotate_mtx(tmp_mtx, a, x, y, z);
	    load_matrix(tmp_mtx, MTX_TYPE_MODEL | mtx_flags);
	    mtx_flags = 0;
	}
        |   TRANSLATE OP_PAREN fexpression COMMA fexpression COMMA fexpression CL_PAREN SEMICOLON
        {
	    float	tmp_mtx[4][4];
	    float	x, y, z;

	    x = $3;
	    y = $5;
	    z = $7;

	    translate_mtx(tmp_mtx, x, y, z);
	    load_matrix(tmp_mtx, MTX_TYPE_MODEL | MTX_FLAG_MULT); /* no flags, assumes MODEL matrix */
	    mtx_flags = 0;
	}
        |   TRANSLATE OP_PAREN mtxtype mtxflaglist COMMA fexpression COMMA fexpression COMMA fexpression CL_PAREN SEMICOLON
        {
	    float	tmp_mtx[4][4];
	    float	x, y, z;

	    x = $6;
	    y = $8;
	    z = $10;

	    translate_mtx(tmp_mtx, x, y, z);
	    load_matrix(tmp_mtx, mtx_flags);
	    mtx_flags = 0;
	}
	;


/* some rules for scene render flags: */
sceneflaglist:
            sceneflagval
	{
	}
        |   sceneflagval sceneflaglist
	{
	}
	;
sceneflagval:
            VERBOSE
	{
	    RPSetSceneFlags(FLAG_VERBOSE);
	}
        |   NOT_OP VERBOSE
	{
	    RPClearSceneFlags(FLAG_VERBOSE);
	}
        |   VERBOSE2
	{
	    RPSetSceneFlags(FLAG_VERBOSE2);
	}
        |   NOT_OP VERBOSE2
	{
	    RPClearSceneFlags(FLAG_VERBOSE2);
	}
        |   NOSHADOW
	{
	    RPSetSceneFlags(FLAG_NOSHADOW);
	}
        |   NOT_OP NOSHADOW
	{
	    RPClearSceneFlags(FLAG_NOSHADOW);
	}
        |   FOG
	{
	    RPSetSceneFlags(FLAG_FOG);
	}
        |   NOT_OP FOG
	{
	    RPClearSceneFlags(FLAG_FOG);
	}
        |   ZBUFFER
	{
	    RPSetSceneFlags(FLAG_ZBUFFER);
	}
        |   NOT_OP ZBUFFER
	{
	    RPClearSceneFlags(FLAG_ZBUFFER);
	}
        |   MULTISAMPLE
	{
	    RPSetSceneFlags(FLAG_SCENE_MULTISAMPLE);
	}
        |   NOT_OP MULTISAMPLE
	{
	    RPClearSceneFlags(FLAG_SCENE_MULTISAMPLE);
	}
        |   PERSPTEXTURE
	{
	    RPSetSceneFlags(FLAG_PERSP_TEXTURE);
	}
        |   NOT_OP PERSPTEXTURE
	{
	    RPClearSceneFlags(FLAG_PERSP_TEXTURE);
	}
        |   ALLFLAGS
	{
	    RPSetSceneFlags(FLAG_ALL);
	}
        |   NOT_OP ALLFLAGS
	{
	    RPClearSceneFlags(FLAG_ALL);
	}
	;

/* some rules for object render flags: */
objflaglist:
            objflagval
	{
	}
        |   objflagval objflaglist
	{
	}
	;
objflagval:
            CULL_BACK
	{
	    RPSetObjectFlags(FLAG_CULL_BACK);
	}
        |   NOT_OP CULL_BACK
	{
	    RPClearObjectFlags(FLAG_CULL_BACK);
	}
        |   CULL_FRONT
	{
	    RPSetObjectFlags(FLAG_CULL_FRONT);
	}
        |   NOT_OP CULL_FRONT
	{
	    RPClearObjectFlags(FLAG_CULL_FRONT);
	}
        |   TEXTURE
	{
	    RPSetObjectFlags(FLAG_TEXTURE);
	}
        |   NOT_OP TEXTURE
	{
	    RPClearObjectFlags(FLAG_TEXTURE);
	}
        |   LIGHTING
	{
	    RPSetObjectFlags(FLAG_LIGHTING);
	}
        |   NOT_OP LIGHTING
	{
	    RPClearObjectFlags(FLAG_LIGHTING);
	}
        |   SMOOTHSHADE
	{
	    RPClearObjectFlags(FLAG_FLATSHADE);
	}
        |   NOT_OP SMOOTHSHADE
	{
	    RPSetObjectFlags(FLAG_FLATSHADE);
	}
        |   FLATSHADE
	{
	    RPSetObjectFlags(FLAG_FLATSHADE);
	}
        |   NOT_OP FLATSHADE
	{
	    RPClearObjectFlags(FLAG_FLATSHADE);
	}
/* RANDSHADE, VERTSHADE, POLYSHADE should be mutually exclusive... */
        |   RANDSHADE
	{
	    RPSetObjectFlags(FLAG_RANDSHADE);
	}
        |   NOT_OP RANDSHADE
	{
	    RPClearObjectFlags(FLAG_RANDSHADE);
	}
        |   POLYSHADE
	{
	    RPSetObjectFlags(FLAG_POLYSHADE);
	}
        |   NOT_OP POLYSHADE
	{
	    RPClearObjectFlags(FLAG_POLYSHADE);
	}
        |   VERTSHADE
	{
	    RPSetObjectFlags(FLAG_VERTSHADE);
	}
        |   NOT_OP VERTSHADE
	{
	    RPClearObjectFlags(FLAG_VERTSHADE);
	}
        |   VERTNORM
	{
	    RPSetObjectFlags(FLAG_VERTNORM);
	}
        |   NOT_OP VERTNORM
	{
	    RPClearObjectFlags(FLAG_VERTNORM);
	}
        |   REFLECT
	{
	    RPSetObjectFlags(FLAG_REFLECT);
	}
        |   NOT_OP REFLECT
	{
	    RPClearObjectFlags(FLAG_REFLECT);
	}
        |   BUMP
	{
	    RPSetObjectFlags(FLAG_BUMP);
	}
        |   NOT_OP BUMP
	{
	    RPClearObjectFlags(FLAG_BUMP);
	}
        |   TEXGENSPHERE
	{
	    RPSetObjectFlags(FLAG_TEXGEN_SPHERE);
	}
        |   NOT_OP TEXGENSPHERE
	{
	    RPClearObjectFlags(FLAG_TEXGEN_SPHERE);
	}
        |   TEXGENCYLINDER
	{
	    RPSetObjectFlags(FLAG_TEXGEN_CYLINDER);
	}
        |   NOT_OP TEXGENCYLINDER
	{
	    RPClearObjectFlags(FLAG_TEXGEN_CYLINDER);
	}
        |   ALLFLAGS
	{
	    RPSetObjectFlags(FLAG_ALL);
	}
        |   NOT_OP ALLFLAGS
	{
	    RPClearObjectFlags(FLAG_ALL);
	}
	;

genericflaglist:
            genericflagval
	{
	}
        |   genericflagval genericflaglist
	{
	}
	;
genericflagval:
            RENDER01
	{
	    RPSetGenericSceneFlags(FLAG_RENDER_01);
	}
        |   NOT_OP RENDER01
	{
	    RPClearGenericSceneFlags(FLAG_RENDER_01);
	}
        |   RENDER02
	{
	    RPSetGenericSceneFlags(FLAG_RENDER_02);
	}
        |   NOT_OP RENDER02
	{
	    RPClearGenericSceneFlags(FLAG_RENDER_02);
	}
        |   RENDER03
	{
	    RPSetGenericSceneFlags(FLAG_RENDER_03);
	}
        |   NOT_OP RENDER03
	{
	    RPClearGenericSceneFlags(FLAG_RENDER_03);
	}
        |   RENDER04
	{
	    RPSetGenericSceneFlags(FLAG_RENDER_04);
	}
        |   NOT_OP RENDER04
	{
	    RPClearGenericSceneFlags(FLAG_RENDER_04);
	}
        |   RENDER05
	{
	    RPSetGenericSceneFlags(FLAG_RENDER_05);
	}
        |   NOT_OP RENDER05
	{
	    RPClearGenericSceneFlags(FLAG_RENDER_05);
	}
        |   RENDER06
	{
	    RPSetGenericSceneFlags(FLAG_RENDER_06);
	}
        |   NOT_OP RENDER06
	{
	    RPClearGenericSceneFlags(FLAG_RENDER_06);
	}
        |   RENDER07
	{
	    RPSetGenericSceneFlags(FLAG_RENDER_07);
	}
        |   NOT_OP RENDER07
	{
	    RPClearGenericSceneFlags(FLAG_RENDER_07);
	}
        |   RENDER08
	{
	    RPSetGenericSceneFlags(FLAG_RENDER_08);
	}
        |   NOT_OP RENDER08
	{
	    RPClearGenericSceneFlags(FLAG_RENDER_08);
	}
        |   ALLFLAGS
	{
	    RPSetGenericSceneFlags(FLAG_ALL);
	}
        |   NOT_OP ALLFLAGS
	{
	    RPClearGenericSceneFlags(FLAG_ALL);
	}
	;
 
/* some rules for matrices: */
mtxtype:
            MTX_MODEL
	{
	    UnFlag(mtx_flags, MTX_TYPE_VIEW);
	    UnFlag(mtx_flags, MTX_TYPE_PROJ);
	    Flag(mtx_flags, MTX_TYPE_MODEL);
	}
        |   MTX_VIEW
	{
	    UnFlag(mtx_flags, MTX_TYPE_MODEL);
	    UnFlag(mtx_flags, MTX_TYPE_PROJ);
	    Flag(mtx_flags, MTX_TYPE_VIEW);
	}
        |   MTX_PROJECTION
	{
	    UnFlag(mtx_flags, MTX_TYPE_MODEL);
	    UnFlag(mtx_flags, MTX_TYPE_VIEW);
	    Flag(mtx_flags, MTX_TYPE_MODEL);
	}
	;
mtxflaglist:
            mtxflagval
	{
	}
        |   mtxflagval mtxflaglist
	{
	}
        |	/* empty flag list */
	{
	}
	;
/* there is a bison reduce/reduce conflict here... but it doesn not seem fatal */
mtxflagval:
            MTX_PUSH
	{
	    mtx_flags |= MTX_FLAG_PUSH;		/* set PUSH bit */
	}
        |   MTX_LOAD
	{
	    mtx_flags &= ~MTX_FLAG_MULT;	/* clear MULT bit */
	}
        |   MTX_MULT
	{
	    mtx_flags |= MTX_FLAG_MULT;		/* set MULT bit */
	}
	;

/* texture flags: */
txtflaglist:
            txtflagval
	{
	}
        |   txtflagval txtflaglist
	{
	}
	;
txtflagval:
            TXT_CLAMP
	{
	    temp_txt_flags |= FLAG_TXT_CLAMP;
	}
        |   NOT_OP TXT_CLAMP
	{
	    temp_txt_flags &= ~FLAG_TXT_CLAMP;
	}
        |   TXT_WRAP
	{
	    temp_txt_flags |= FLAG_TXT_WRAP;
	}
        |   NOT_OP TXT_WRAP
	{
	    temp_txt_flags &= ~FLAG_TXT_WRAP;
	}
        |   TXT_MIRROR
	{
	    temp_txt_flags |= FLAG_TXT_MIRROR;
	}
        |   NOT_OP TXT_MIRROR
	{
	    temp_txt_flags &= ~FLAG_TXT_MIRROR;
	}
        |   TXT_FILT
	{
	    temp_txt_flags |= FLAG_TXT_FILT;
	}
        |   NOT_OP TXT_FILT
	{
	    temp_txt_flags &= ~FLAG_TXT_FILT;
	}
        |   TXT_MODULATE
	{
	    temp_txt_flags |= FLAG_TXT_MODULATE;
	}
        |   NOT_OP TXT_MODULATE
	{
	    temp_txt_flags &= ~FLAG_TXT_MODULATE;
	}
	;

/* some rules for vertex lists: */
pointlist:
            point
	{
	}
        |   point COMMA pointlist
	{
	}
        |   point COMMA
	{
		/* handle extraneous comma at the end of a list of points */
	}
	;
point:
            OP_CURLY fexpression COMMA fexpression COMMA fexpression COMMA fexpression COMMA fexpression COMMA fexpression COMMA fexpression COMMA fexpression COMMA fexpression CL_CURLY
	{
	    if (tvcnt < MAX_VERTS) {
		temp_vtx.pos.x = $2;
		temp_vtx.pos.y = $4;
		temp_vtx.pos.z = $6;
		temp_vtx.r = $8;
		temp_vtx.g = $10;
		temp_vtx.b = $12;
		temp_vtx.a = $14;
		temp_vtx.s = $16;
		temp_vtx.t = $18;
		temp_vtx.n.x = $8;	/* color may be normal */
		temp_vtx.n.y = $10;
		temp_vtx.n.z = $12;
		RPAddVertex(&temp_vtx, tvcnt);
		tvcnt++;
	    } else {
		sprintf(error_buffer,"%s : vertex buffer overflow.","ERROR");
		yyerror(error_buffer);
	    }
	}
        ;

/* some rules for triangle lists: */
trilist:
            tri
	{
	}
        |   tri COMMA trilist
	{
	}
        |   tri COMMA
	{
		/* handle extraneous comma at the end of a list of triangles */
	}
	;
tri:
            OP_CURLY iexpression COMMA iexpression COMMA iexpression CL_CURLY
	{
	    if (ttcnt < MAX_VERTS) {
		temp_tri.v0 = $2;
		temp_tri.v1 = $4;
		temp_tri.v2 = $6;
		RPAddTriangle(&temp_tri);
		ttcnt++;
	    } else {
		sprintf(error_buffer,"%s : triangle buffer overflow.","ERROR");
		yyerror(error_buffer);
	    }
	}
        ;

/* integer expression: a very basic rule: */
iexpression:
	  OP_PAREN iexpression CL_PAREN
	{
	    int val;

	    val = $2;
	    $$ = val;
	}
	| INTEGER
	{
	    int val;

	    val = $1;
	    $$ = val;
	}
	| NOT_OP iexpression	%prec UMINUS
	{
	    int val1;

	    val1 = $2;
	    $$ = ~val1;
	}
	| iexpression AND_OP iexpression
	{
	    int val1, val2;

	    val1 = $1;
	    val2 = $3;
	    $$ = val1 & val2;
	}
	| iexpression OR_OP iexpression
	{
	    int val1, val2;

	    val1 = $1;
	    val2 = $3;
	    $$ = val1 | val2;
	}
	| iexpression XOR_OP iexpression
	{
	    int val1, val2;

	    val1 = $1;
	    val2 = $3;
	    $$ = val1 ^ val2;
	}
	| iexpression LSHFT_OP iexpression
	{
	    int val1, val2;

	    val1 = $1;
	    val2 = $3;
	    $$ = val1 << val2;
	}
	| iexpression RSHFT_OP iexpression
	{
	    unsigned int val1, val2;

	    val1 = $1;
	    val2 = $3;
	    $$ = val1 >> val2;
	}
	| iexpression MULT_OP iexpression
	{
	    int val1, val2;

	    val1 = $1;
	    val2 = $3;
	    $$ = val1 * val2;
	}
	| iexpression DIV_OP iexpression
	{
	    int val1, val2;

	    val1 = $1;
	    val2 = $3;
	    if (val2 == 0) {
		sprintf(error_buffer, 
			"expression divides by 0");
		yyerror(error_buffer);
	    }
	    $$ = val1 / val2;
	}
	| iexpression MOD_OP iexpression
	{
	    int val1, val2;

	    val1 = $1;
	    val2 = $3;
	    if (val2 == 0) {
		sprintf(error_buffer, 
			"expression divides by 0");
		yyerror(error_buffer);
	    }
	    $$ = val1 % val2;
	}
	| iexpression PLUS_OP iexpression
	{
	    int val1, val2;

	    val1 = $1;
	    val2 = $3;
	    $$ = val1 + val2;
	}
	| iexpression MINUS_OP iexpression
	{
	    int val1, val2;

	    val1 = $1;
	    val2 = $3;
	    $$ = val1 - val2;
	}
	| MINUS_OP iexpression    %prec UMINUS
	{
	    int val1;

	    val1 = $2;
	    $$ = -val1;
	}
	| PLUS_OP iexpression    %prec UPLUS
	{
	    int val1;

	    val1 = $2;
	    $$ = val1;
	}
        ;

/* like iexpression, except floating point (no logical) */
fexpression:
	  OP_PAREN fexpression CL_PAREN
	{
	    float val;

	    val = $2;
	    $$ = val;
	}
	| FLOATER
	{
	    float val;

	    val = $1;
	    $$ = val;
	}
	| INTEGER
	{
	    float val;

	    val = (float) $1;
	    $$ = val;
	}
	| fexpression MULT_OP fexpression
	{
	    float val1, val2;

	    val1 = $1;
	    val2 = $3;
	    $$ = val1 * val2;
	}
	| fexpression DIV_OP fexpression
	{
	    float val1, val2;

	    val1 = $1;
	    val2 = $3;
	    if (val2 == 0) {
		sprintf(error_buffer, 
			"expression divides by 0");
		yyerror(error_buffer);
	    }
	    $$ = val1 / val2;
	}
	| fexpression PLUS_OP fexpression
	{
	    float val1, val2;

	    val1 = $1;
	    val2 = $3;
	    $$ = val1 + val2;
	}
	| fexpression MINUS_OP fexpression
	{
	    float val1, val2;

	    val1 = $1;
	    val2 = $3;
	    $$ = val1 - val2;
	}
	| MINUS_OP fexpression    %prec UMINUS
	{
	    float val1;

	    val1 = $2;
	    $$ = -val1;
	}
	| PLUS_OP fexpression    %prec UPLUS
	{
	    float val1;

	    val1 = $2;
	    $$ = val1;
	}
        ;
%%

#if 0 /* removed to quiet bison warnings on Mac */
static void
yywarn(char *string)
{
    fprintf(stderr,"WARNING: %s in file %s at line no. %d: [", 
	    string, _RPinput_file, _RPmylineno);
    fprintf(stderr,"%s",_RPline_buffer);
    fprintf(stderr,"]\n");
}
#endif

static void
yyerror(char *s)
{
    fprintf(stderr,"ERROR: %s in file %s at line no. %d: [", 
	    s, _RPinput_file, _RPmylineno);
    fprintf(stderr,"%s",_RPline_buffer);
    fprintf(stderr,"]\n");
}

static void
expandstring(char *s)
{
    char	*c, *tc;

    c = s;
    while (*c != '\0') {
	if (*c == '\\' && *(c+1) == 'n') {
	    *c = '\n';
	    for (tc = c+1; *tc != '\0'; tc++) {
		*tc = *(tc+1);
	    }
	}
	if (*c == '\\' && *(c+1) == 't') {
	    *c = '\t';
	    for (tc = c+1; *tc != '\0'; tc++) {
		*tc = *(tc+1);
	    }
	}
	c++;
    }
}


