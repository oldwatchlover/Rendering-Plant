
/*
 * File:	parser.lx.l
 *
 * The lex grammar for Rendering Plant scene input
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

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "rp.h"
#include "y.tab.h"

#undef YYLMAX
#define YYLMAX 4096

#ifdef LEXDEBUG
#	define RET(x) {fprintf(stderr,"lex %d [%s]\n", x, yytext); \
				return(x);}
#else
#	define RET(x) return(x);
#endif

static void skipcomments(void);
static void skippound(void);
extern char _RPline_buffer[];
extern char _RPinput_file[];
extern int  _RPmylineno;

%}

%%

%{
/* language keywords: */
%}

"ALLFLAGS"             {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = ALLFLAGS;
			    RET(ALLFLAGS); 
                       }
"sceneflags"           {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = SCENEFLAGS;
			    RET(SCENEFLAGS); 
                        }
"VERBOSE"               {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = VERBOSE;
			    RET(VERBOSE); 
                        }
"VERBOSE2"              {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = VERBOSE2;
			    RET(VERBOSE2); 
                        }
"NOSHADOW"              {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = NOSHADOW;
			    RET(NOSHADOW); 
                        }
"FOG"                   {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = FOG;
			    RET(FOG); 
                        }
"ZBUFFER"               {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = ZBUFFER;
			    RET(ZBUFFER); 
                        }
"MULTISAMPLE"           {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = MULTISAMPLE;
			    RET(MULTISAMPLE); 
                        }
"PERSPTEXTURE"          {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = PERSPTEXTURE;
			    RET(PERSPTEXTURE); 
                        }

"objflags"              {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = OBJFLAGS;
			    RET(OBJFLAGS); 
                        }
"CULL_BACK"             {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = CULL_BACK;
			    RET(CULL_BACK); 
                        }
"CULL_FRONT"            {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = CULL_FRONT;
			    RET(CULL_FRONT); 
                        }
"TEXTURE"               {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = TEXTURE;
			    RET(TEXTURE); 
                        }
"LIGHTING"              {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = LIGHTING;
			    RET(LIGHTING); 
                        }
"FLATSHADE"             {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = FLATSHADE;
			    RET(FLATSHADE); 
                        }
"SMOOTHSHADE"           {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = SMOOTHSHADE;
			    RET(SMOOTHSHADE); 
                        }
"RANDSHADE"             {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = RANDSHADE;
			    RET(RANDSHADE); 
                        }
"POLYSHADE"             {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = POLYSHADE;
			    RET(POLYSHADE); 
                        }
"VERTSHADE"             {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = VERTSHADE;
			    RET(VERTSHADE); 
                        }
"VERTEXNORMALS"         {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = VERTNORM;
			    RET(VERTNORM); 
                        }
"VERTNORM"              {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = VERTNORM;
			    RET(VERTNORM); 
                        }
"REFLECT"               {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = REFLECT;
			    RET(REFLECT); 
                        }
"BUMP"                  {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = BUMP;
			    RET(BUMP); 
                        }
"TEXGEN_SPHERE"         {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = TEXGENSPHERE;
			    RET(TEXGENSPHERE); 
                        }
"TEXGEN_CYLINDER"       {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = TEXGENCYLINDER;
			    RET(TEXGENCYLINDER); 
                        }
"genericflags"          {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = GENERICFLAGS;
			    RET(GENERICFLAGS); 
                        }
"RENDER01"              {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = RENDER01;
			    RET(RENDER01); 
                        }
"RENDER02"              {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = RENDER02;
			    RET(RENDER02); 
                        }
"RENDER03"              {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = RENDER03;
			    RET(RENDER03); 
                        }
"RENDER04"              {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = RENDER04;
			    RET(RENDER04); 
                        }
"RENDER05"              {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = RENDER05;
			    RET(RENDER05); 
                        }
"RENDER06"              {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = RENDER06;
			    RET(RENDER06); 
                        }
"RENDER07"              {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = RENDER07;
			    RET(RENDER07); 
                        }
"RENDER08"              {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = RENDER08;
			    RET(RENDER08); 
                        }
"matrix"                {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = MATRIX;
			    RET(MATRIX); 
                        }
"mtx"                   {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = MATRIX;
			    RET(MATRIX); 
                        }
"MTX_PUSH"              {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = MTX_PUSH;
			    RET(MTX_PUSH); 
                        }
"MTX_PROJECTION"        {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = MTX_PROJECTION;
			    RET(MTX_PROJECTION); 
                        }
"MTX_VIEW"              {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = MTX_VIEW;
			    RET(MTX_VIEW); 
                        }
"MTX_MODEL"             {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = MTX_MODEL;
			    RET(MTX_MODEL); 
                        }
"MTX_LOAD"              {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = MTX_LOAD;
			    RET(MTX_LOAD); 
                        }
"MTX_MULT"              {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = MTX_MULT;
			    RET(MTX_MULT);
                        }
"light"                 {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = LIGHT;
			    RET(LIGHT); 
                        }
"spotlight"             {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = SPOTLIGHT;
			    RET(SPOTLIGHT); 
                        }
"material"              {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = MATERIAL;
			    RET(MATERIAL); 
                        }
"color"                 {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = COLOR;
			    RET(COLOR); 
                        }
"ambient"               {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = AMBIENT;
			    RET(AMBIENT); 
                        }
"diffuse"               {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = DIFFUSE;
			    RET(DIFFUSE); 
                        }
"specular"              {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = SPECULAR;
			    RET(SPECULAR); 
                        }
"highlight"             {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = HIGHLIGHT;
			    RET(HIGHLIGHT); 
                        }
"hilite"                {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = HIGHLIGHT;
			    RET(HIGHLIGHT); 
                        }
"shiny"                 {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = SHINY;
			    RET(SHINY); 
                        }
"reflection"            {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = REFLECTION;
			    RET(REFLECTION); 
                        }
"refraction"            {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = REFRACTION;
			    RET(REFRACTION); 
                        }
"texname"               {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = TEXNAME;
			    RET(TEXNAME); 
                        }
"fog"                   {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = FOG_CMD;
			    RET(FOG_CMD);
                        }
"output"                {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = OUTPUT;
			    RET(OUTPUT); 
                        }
"clear"                 {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = CLEAR_BACK;
			    RET(CLEAR_BACK); 
                        }
"background"            {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = BACKGROUND;
			    RET(BACKGROUND); 
                        }
"scissor"               {
                            strcat(_RPline_buffer, yytext);
                            yylval.integer = SCISSOR;
                            RET(SCISSOR);
                        }
"viewport"              {
                            strcat(_RPline_buffer, yytext);
                            yylval.integer = VIEWPORT;
                            RET(VIEWPORT);
                        }
"vp"                    {
                            strcat(_RPline_buffer, yytext);
                            yylval.integer = VIEWPORT;
                            RET(VIEWPORT);
                        }
"vertex"                {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = VERTEX;
			    RET(VERTEX); 
                        }
"vtx"                   {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = VERTEX;
			    RET(VERTEX); 
                        }
"texture"              {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = TEXTURE_FILE;
			    RET(TEXTURE_FILE);
                        }
"CLAMP"                 {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = TXT_CLAMP;
			    RET(TXT_CLAMP);
                        }
"WRAP"                  {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = TXT_WRAP;
			    RET(TXT_WRAP);
                        }
"MIRROR"                {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = TXT_MIRROR;
			    RET(TXT_MIRROR);
                        }
"FILT"                  {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = TXT_FILT;
			    RET(TXT_FILT);
                        }
"MODULATE"              {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = TXT_MODULATE;
			    RET(TXT_MODULATE);
                        }
"tex"                   {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = TEXTURE;
			    RET(TEXTURE); 
                        }
"sphere"                {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = SPHERE;
			    RET(SPHERE); 
                        }
"sp"                    {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = SPHERE;
			    RET(SPHERE); 
                        }
"trilist"               {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = TRILIST;
			    RET(TRILIST); 
                        }
"loadobj"               {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = LOADOBJ;
			    RET(LOADOBJ); 
                        }
%{
/* high-level matrix commands: */
%}
"identity"              {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = IDENTITY;
			    RET(IDENTITY); 
                        }
"perspective"           {
                            strcat(_RPline_buffer, yytext);
                            yylval.integer = PERSPECTIVE;
                            RET(PERSPECTIVE);
                        }
"orthographic"          {
                            strcat(_RPline_buffer, yytext);
                            yylval.integer = ORTHOGRAPHIC;
                            RET(ORTHOGRAPHIC);
                        }
"depthrange"            {
                            strcat(_RPline_buffer, yytext);
                            yylval.integer = DEPTHRANGE;
                            RET(DEPTHRANGE);
                        }
"depth"                 {
                            strcat(_RPline_buffer, yytext);
                            yylval.integer = DEPTHRANGE;
                            RET(DEPTHRANGE);
                        }
"camera"                {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = CAMERA;
			    RET(CAMERA); 
                        }
"pop"                   {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = POP;
			    RET(POP); 
                        }
"scale"                 {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = SCALE;
			    RET(SCALE); 
                        }
"rotate"                {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = ROTATE;
			    RET(ROTATE); 
                        }
"translate"             {
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = TRANSLATE;
			    RET(TRANSLATE); 
                        }

%{
/* expressions and other syntax tokens: */
%}

\n			{
                            _RPline_buffer[0] = '\0';
                            _RPmylineno++;
			}
"/*"			{
			    skipcomments();
			}			
"#"			{
			    /* skip comments */
                            skippound();
			}
"~"	 		{
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = NOT_OP; 
			    RET(NOT_OP); 
			}			
"!"	 		{
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = NOT_OP; 
			    RET(NOT_OP); 
			}			
"&"	 		{
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = AND_OP; 
			    RET(AND_OP); 
			}			
"|"	 		{
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = OR_OP; 
			    RET(OR_OP); 
			}			
"^"	 		{
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = XOR_OP; 
			    RET(XOR_OP); 
			}			
">>"	 		{
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = RSHFT_OP; 
			    RET(RSHFT_OP); 
			}			
"<<"	 		{
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = LSHFT_OP; 
			    RET(LSHFT_OP); 
			}			
"*"	 		{
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = MULT_OP; 
			    RET(MULT_OP); 
			}			
"/"	 		{
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = DIV_OP; 
			    RET(DIV_OP); 
			}			
"%"	 		{
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = MOD_OP; 
			    RET(MOD_OP); 
			}			
"+"	 		{
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = PLUS_OP; 
			    RET(PLUS_OP); 
			}			
"-"	 		{
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = MINUS_OP; 
			    RET(MINUS_OP); 
			}			
"("	 		{
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = OP_PAREN; 
			    RET(OP_PAREN); 
			}			
")"			{
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = CL_PAREN; 
			    RET(CL_PAREN); 
			}
"["	 		{
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = OP_BRACKET; 
			    RET(OP_BRACKET); 
			}			
"]"			{
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = CL_BRACKET; 
			    RET(CL_BRACKET); 
			}
"{"	 		{
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = OP_CURLY;
			    RET(OP_CURLY); 
			}			
"}"			{
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = CL_CURLY;
			    RET(CL_CURLY); 
			}
","			{
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = COMMA; 
			    RET(COMMA); 
			}
";"			{
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = SEMICOLON;
			    RET(SEMICOLON); 
			}
0x[0-9a-fA-F]+		{   /* hex integer */
                            strcat(_RPline_buffer, yytext);
			    sscanf(yytext, "%x", &yylval.integer);
			    RET(INTEGER);
			}
0X[0-9a-fA-F]+		{   /* hex integer */
                            strcat(_RPline_buffer, yytext);
			    sscanf(yytext, "%x", &yylval.integer);
			    RET(INTEGER);
			}
[1-9][0-9]*		{   /* decimal integer */
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = atoi(yytext);
			    RET(INTEGER); 
			}			
0[0-7]*		        {   /* octal integer */
                            strcat(_RPline_buffer, yytext);
			    yylval.integer = strtol(yytext, NULL, 8);
			    RET(INTEGER); 
			}			
[0-9]*\.[0-9]*		{   /* floating point */
                            strcat(_RPline_buffer, yytext);
			    yylval.floater = atof(yytext);
			    RET(FLOATER); 
			}			
\"[^\"]*[\"]            {   /* quoted string */
                            strcat(_RPline_buffer, yytext);
                            yylval.string = (char *) malloc(strlen(yytext)+1);
                            strcpy(yylval.string, yytext+1);
                            yylval.string[yyleng-2] = (char) NULL;
                            RET(QSTRING);
                        }
[ \t]*			{
                            strcat(_RPline_buffer, yytext);
			    /* skip white space including commas */
			}			
.			{
			    fprintf(stderr,"scanner error\n");
			    fprintf(stderr,"unrecognized char %s\n", yytext);
			}
%%
static void
skipcomments(void)
{
    for (;;) {
	while (input() != '*');
	if (input() != '/') {
	    unput(yytext[yyleng-1]);
	} else {
	    return;
	}
    }
}

static void
skippound(void)
{
    int     i = 0, lineno;
    char    c, poundbuf[256], fname[80];

    while ((c = input()) != '\n')
        poundbuf[i++] = c;

    poundbuf[i] = '\0';
    if (sscanf(poundbuf, " %d \"%s\"",&lineno, fname) == 2) {
        fname[strlen(fname)-1] = '\0';
        _RPmylineno = lineno-1;
        strcpy(_RPinput_file, fname);
    }
    _RPmylineno++;
}
