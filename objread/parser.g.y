%{

/*
 * File:        parser.g.y
 *
 * yacc grammar to parse Wavefront .obj files
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

#include "objread.h"

#define YYDEBUG 1

/* for linux version, increase this: */
#undef YYMAXDEPTH
#define YYMAXDEPTH  16384
#undef YYSTACKSIZE
#define YYSTACKSiZE YYMAXDEPTH

/* any static local variables for the parser... */

static int	stringlistcount = 0;
static char	*stringlist[128];

#define	MAX_FACE_INDEX	32
static int	face_index_count = 0;
static int	faceilist[MAX_FACE_INDEX];
static int	facetlist[MAX_FACE_INDEX];
static int	facenlist[MAX_FACE_INDEX];

//static void	yywarn(char *string);
static void	yyerror(char *s);

/* from main.c */
extern int mylineno;
extern char line_buffer[];
extern char input_file[];

int 	yylex(void); /* added this to quiet warnings with bison -y */

%}

%name-prefix="zz"

%union {
	int		integer;
	float		floater;
	char		*string;
	unsigned int	uint;
}

/* syntax pieces: */
%token	<integer>	FIRSTTOKEN	/* must be first */
%token	<integer>	INTEGER
%token	<floater>	FLOATER
%token	<string>	F_ITN_INDEX
%token	<string>	F_IT_INDEX
%token	<string>	F_IN_INDEX
%token	<string>	STRING

/* command tokens */
%token	<integer>	VERTEX
%token	<integer>	VTEXCOORDS
%token	<integer>	VNORMALS
%token	<integer>	VPARAMS
%token	<integer>	FACE
%token	<integer>	OBJECT
%token	<integer>	GROUP
%token	<integer>	SMOOTHGROUP
%token	<integer>	MTLLIB
%token	<integer>	USEMTL
%token	<integer>	MAPLIB
%token	<integer>	USEMAP

/* complex command pieces */
%type   <floater>       fexpression

%token	<integer>	LASTTOKEN	/* must be last */

/* for instruction rules: */
%left   MINUS_OP 
%nonassoc       UMINUS UPLUS


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

/* v  input data: */

          VERTEX fexpression fexpression fexpression
	{
	    xyz_t	pos;
            float	w;

	    pos.x = $2;
	    pos.y = $3;
	    pos.z = $4;
	    w = 1.0;

	    add_vertex(pos.x, pos.y, pos.z, w);
	}
        | VERTEX fexpression fexpression fexpression fexpression
	{
	    xyz_t	pos;
            float	w;

	    pos.x = $2;
	    pos.y = $3;
	    pos.z = $4;
	    w = $5;

	    add_vertex(pos.x, pos.y, pos.z, w);
	}
	/* this isn't in the spec, but I've seen in some data? */
        | VERTEX fexpression fexpression fexpression fexpression fexpression fexpression
	{
	    xyz_t	pos;
            float	w = 1.0, r, g, b;

	    pos.x = $2;
	    pos.y = $3;
	    pos.z = $4;
	    r = $5;
	    g = $6;
	    b = $7;

	    add_vertex(pos.x, pos.y, pos.z, w);
	}

/* vt input data: */

        | VTEXCOORDS fexpression 
	{
	    float 	u;

	    u = $2;

	    add_vtexcoords(1, u, 0.0, 0.0);
	}
        | VTEXCOORDS fexpression fexpression 
	{
	    float 	u, v;

	    u = $2;
	    v = $3;
	    add_vtexcoords(2, u, v, 0.0);
	}
        | VTEXCOORDS fexpression fexpression fexpression 
	{
	    float 	u, v, w;

	    u = $2;
	    v = $3;
	    w = $4;

	    add_vtexcoords(3, u, v, w);
	}

/* vn input data: */

        | VNORMALS fexpression fexpression fexpression 
	{
	    xyz_t	pos;

	    pos.x = $2;
	    pos.y = $3;
	    pos.z = $4;

	    add_vnormal(pos.x, pos.y, pos.z);
	}

/* vp input data: */

        | VPARAMS fexpression 
	{
	    float	u;

	    u = $2;
		/* unused */
	}
        | VPARAMS fexpression fexpression 
	{
	    float	u, v;

	    u = $2;
	    v = $3;
		/* unused */
	}
        | VPARAMS fexpression fexpression fexpression 
	{
	    float	u, v, w;

	    u = $2;
	    v = $3;
	    w = $4;
		/* unused */
	}

/* f input data: */

        | FACE faceindexlist 
	{
	    add_face(face_index_count, faceilist, facetlist, facenlist);
	    face_index_count = 0;
	}

/* o input data: */

        | OBJECT INTEGER 
	{
            int		obj;
	    char	objname[64];

	    obj = $2;

	    sprintf(objname,"%d",obj);

	    add_object(objname);
	}
        | OBJECT STRING 
	{
	    char	*objname;

	    objname = $2;

	    add_object(objname);
	}

/* g input data: */

        | GROUP 
	{
	    char	*def = "default";

	    stringlist[0] = def;

	    add_group(1, stringlist);
	}
        | GROUP stringlist 
	{
	    add_group(stringlistcount, stringlist);
	    stringlistcount = 0;
	}

/* s input data: */

        | SMOOTHGROUP INTEGER 
	{
	    int	smooth;

	    smooth = $2;

	    add_smooth(smooth);
	}
        | SMOOTHGROUP STRING 
	{
	    char	*smooth;
            int		s = 0;

		/* handle it as a text string, could be "off" */

	    smooth = $2;

	    if (strcmp(smooth,"off")==0)
		s = 0;
            else
		sscanf(smooth,"%d",&s);

	    add_smooth(s);
	}

/* mtllib input data: */

        | MTLLIB stringlist 
	{
	    add_mtllib(stringlistcount, stringlist);
	    stringlistcount = 0;
	}

/* usemtl input data: */

        | USEMTL STRING 
	{
 	    char	*mtl;

	    mtl = $2;

	    add_usemtl(mtl);
	}

/* maplib input data: */

        | MAPLIB stringlist
	{
	    add_maplib(stringlistcount, stringlist);
	    stringlistcount = 0;
	}

/* usemap input data: */

        | USEMAP STRING
	{
 	    char	*map;

	    map = $2;

	    add_usemap(map);
	}
	;




FACEINDEX:
	  STRING
	{	/* vertex texture and normal */
	    int		i, k, v0, t0, n0;
  	    char	*fmt, *s;

	    fmt = $1;

	    s = fmt;	/* count / char in token */
	    for (k=0; s[k]; s[k]=='/' ? k++: *s++);

	    if (k == 0) {		/* no / in token, index only */
		sscanf(fmt,"%d",&v0);
		t0 = n0 = 0;
	    } else if (k == 1) {	/* index and tex coord only */
		sscanf(fmt,"%d/%d",&v0,&t0);
		n0 = 0;
	    } else if (k == 2) {	/* index tex coord and normal */

		    /* handle case where no tex, just 2 consec / */
		for (i=0; i<(int)strlen(fmt); i++)
		    if (fmt[i] == '/')
			break;

		if (fmt[i] == '/' && fmt[i+1] == '/') {
		    sscanf(fmt,"%d//%d",&v0,&n0);
		    t0 = 0;
		} else {
		    sscanf(fmt,"%d/%d/%d",&v0,&t0,&n0);
		}
	    } else {			/* error */
		fprintf(stderr,"face: don't recognize [%s]\n",fmt);
		v0 = t0 = n0 = 0;
	    }

	    i = face_index_count;
	    faceilist[i] = v0;
	    facetlist[i] = t0;
	    facenlist[i] = n0;
	    face_index_count++;

	    if (face_index_count > MAX_FACE_INDEX) {
		fprintf(stderr,"face has too many vertices... [%d]\n",face_index_count);
		exit(-1);
	    }
	}
	| INTEGER
	{	/* only vertex */
	    int	i;

	    i = face_index_count;
	    faceilist[i] = $1;
	    facetlist[i] = 0;
	    facenlist[i] = 0;
	    face_index_count++;

	    if (face_index_count > MAX_FACE_INDEX) {
		fprintf(stderr,"face has too many vertices... [%d]\n",face_index_count);
		exit(-1);
	    }
	}
	;

/* list of face indices, up to 8 verts in a face (else punt)  */
faceindexlist:
	  FACEINDEX FACEINDEX FACEINDEX
	{	/* 3 verts */
	}
	| FACEINDEX faceindexlist
	{	/* 4 or more verts */
	}
	;


/* list of strings: */
stringlist:
	STRING
	{
	    stringlist[stringlistcount] = $1;
	    stringlistcount++;
	}
        | STRING stringlist
	{
	}
	;


/* like iexpression, except floating point (no logical) */
fexpression:
	FLOATER
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
        ;
%%

int yywrap(void)
{
    return 1;
}

#if 0 /* removed to quiet bison warnings on Mac */
static void
yywarn(char *string)
{
    fprintf(stderr,"WARNING: %s in file %s at line no. %d: [", 
	    string, input_file, mylineno);
    fprintf(stderr,"%s",line_buffer);
    fprintf(stderr,"]\n");
}
#endif

static void
yyerror(char *s)
{
    fprintf(stderr,"ERROR: %s in file %s at line no. %d: [", 
	    s, input_file, mylineno);
    fprintf(stderr,"%s",line_buffer);
    fprintf(stderr,"]\n");
}

