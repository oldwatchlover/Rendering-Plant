
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
extern char line_buffer[];
extern char input_file[];
extern int  mylineno;

%}

%%

%{
/* language keywords: */
%}

"ALLFLAGS"             {
                            strcat(line_buffer, yytext);
			    yylval.integer = ALLFLAGS;
			    RET(ALLFLAGS); 
                       }
"sceneflags"           {
                            strcat(line_buffer, yytext);
			    yylval.integer = SCENEFLAGS;
			    RET(SCENEFLAGS); 
                        }
"VERBOSE"               {
                            strcat(line_buffer, yytext);
			    yylval.integer = VERBOSE;
			    RET(VERBOSE); 
                        }
"VERBOSE2"              {
                            strcat(line_buffer, yytext);
			    yylval.integer = VERBOSE2;
			    RET(VERBOSE2); 
                        }
"NOSHADOW"              {
                            strcat(line_buffer, yytext);
			    yylval.integer = NOSHADOW;
			    RET(NOSHADOW); 
                        }
"FOG"                   {
                            strcat(line_buffer, yytext);
			    yylval.integer = FOG;
			    RET(FOG); 
                        }
"ZBUFFER"               {
                            strcat(line_buffer, yytext);
			    yylval.integer = ZBUFFER;
			    RET(ZBUFFER); 
                        }
"MULTISAMPLE"           {
                            strcat(line_buffer, yytext);
			    yylval.integer = MULTISAMPLE;
			    RET(MULTISAMPLE); 
                        }
"PERSPTEXTURE"          {
                            strcat(line_buffer, yytext);
			    yylval.integer = PERSPTEXTURE;
			    RET(PERSPTEXTURE); 
                        }

"objflags"              {
                            strcat(line_buffer, yytext);
			    yylval.integer = OBJFLAGS;
			    RET(OBJFLAGS); 
                        }
"CULL_BACK"             {
                            strcat(line_buffer, yytext);
			    yylval.integer = CULL_BACK;
			    RET(CULL_BACK); 
                        }
"CULL_FRONT"            {
                            strcat(line_buffer, yytext);
			    yylval.integer = CULL_FRONT;
			    RET(CULL_FRONT); 
                        }
"TEXTURE"               {
                            strcat(line_buffer, yytext);
			    yylval.integer = TEXTURE;
			    RET(TEXTURE); 
                        }
"LIGHTING"              {
                            strcat(line_buffer, yytext);
			    yylval.integer = LIGHTING;
			    RET(LIGHTING); 
                        }
"FLATSHADE"             {
                            strcat(line_buffer, yytext);
			    yylval.integer = FLATSHADE;
			    RET(FLATSHADE); 
                        }
"SMOOTHSHADE"           {
                            strcat(line_buffer, yytext);
			    yylval.integer = SMOOTHSHADE;
			    RET(SMOOTHSHADE); 
                        }
"RANDSHADE"             {
                            strcat(line_buffer, yytext);
			    yylval.integer = RANDSHADE;
			    RET(RANDSHADE); 
                        }
"POLYSHADE"             {
                            strcat(line_buffer, yytext);
			    yylval.integer = POLYSHADE;
			    RET(POLYSHADE); 
                        }
"VERTSHADE"             {
                            strcat(line_buffer, yytext);
			    yylval.integer = VERTSHADE;
			    RET(VERTSHADE); 
                        }
"VERTEXNORMALS"         {
                            strcat(line_buffer, yytext);
			    yylval.integer = VERTNORM;
			    RET(VERTNORM); 
                        }
"VERTNORM"              {
                            strcat(line_buffer, yytext);
			    yylval.integer = VERTNORM;
			    RET(VERTNORM); 
                        }
"REFLECT"               {
                            strcat(line_buffer, yytext);
			    yylval.integer = REFLECT;
			    RET(REFLECT); 
                        }
"BUMP"                  {
                            strcat(line_buffer, yytext);
			    yylval.integer = BUMP;
			    RET(BUMP); 
                        }
"TEXGEN_SPHERE"         {
                            strcat(line_buffer, yytext);
			    yylval.integer = TEXGENSPHERE;
			    RET(TEXGENSPHERE); 
                        }
"TEXGEN_CYLINDER"       {
                            strcat(line_buffer, yytext);
			    yylval.integer = TEXGENCYLINDER;
			    RET(TEXGENCYLINDER); 
                        }
"genericflags"          {
                            strcat(line_buffer, yytext);
			    yylval.integer = GENERICFLAGS;
			    RET(GENERICFLAGS); 
                        }
"RENDER01"              {
                            strcat(line_buffer, yytext);
			    yylval.integer = RENDER01;
			    RET(RENDER01); 
                        }
"RENDER02"              {
                            strcat(line_buffer, yytext);
			    yylval.integer = RENDER02;
			    RET(RENDER02); 
                        }
"RENDER03"              {
                            strcat(line_buffer, yytext);
			    yylval.integer = RENDER03;
			    RET(RENDER03); 
                        }
"RENDER04"              {
                            strcat(line_buffer, yytext);
			    yylval.integer = RENDER04;
			    RET(RENDER04); 
                        }
"RENDER05"              {
                            strcat(line_buffer, yytext);
			    yylval.integer = RENDER05;
			    RET(RENDER05); 
                        }
"RENDER06"              {
                            strcat(line_buffer, yytext);
			    yylval.integer = RENDER06;
			    RET(RENDER06); 
                        }
"RENDER07"              {
                            strcat(line_buffer, yytext);
			    yylval.integer = RENDER07;
			    RET(RENDER07); 
                        }
"RENDER08"              {
                            strcat(line_buffer, yytext);
			    yylval.integer = RENDER08;
			    RET(RENDER08); 
                        }
"matrix"                {
                            strcat(line_buffer, yytext);
			    yylval.integer = MATRIX;
			    RET(MATRIX); 
                        }
"mtx"                   {
                            strcat(line_buffer, yytext);
			    yylval.integer = MATRIX;
			    RET(MATRIX); 
                        }
"MTX_PUSH"              {
                            strcat(line_buffer, yytext);
			    yylval.integer = MTX_PUSH;
			    RET(MTX_PUSH); 
                        }
"MTX_PROJECTION"        {
                            strcat(line_buffer, yytext);
			    yylval.integer = MTX_PROJECTION;
			    RET(MTX_PROJECTION); 
                        }
"MTX_VIEW"              {
                            strcat(line_buffer, yytext);
			    yylval.integer = MTX_VIEW;
			    RET(MTX_VIEW); 
                        }
"MTX_MODEL"             {
                            strcat(line_buffer, yytext);
			    yylval.integer = MTX_MODEL;
			    RET(MTX_MODEL); 
                        }
"MTX_LOAD"              {
                            strcat(line_buffer, yytext);
			    yylval.integer = MTX_LOAD;
			    RET(MTX_LOAD); 
                        }
"MTX_MULT"              {
                            strcat(line_buffer, yytext);
			    yylval.integer = MTX_MULT;
			    RET(MTX_MULT);
                        }
"light"                 {
                            strcat(line_buffer, yytext);
			    yylval.integer = LIGHT;
			    RET(LIGHT); 
                        }
"spotlight"             {
                            strcat(line_buffer, yytext);
			    yylval.integer = SPOTLIGHT;
			    RET(SPOTLIGHT); 
                        }
"material"              {
                            strcat(line_buffer, yytext);
			    yylval.integer = MATERIAL;
			    RET(MATERIAL); 
                        }
"color"                 {
                            strcat(line_buffer, yytext);
			    yylval.integer = COLOR;
			    RET(COLOR); 
                        }
"ambient"               {
                            strcat(line_buffer, yytext);
			    yylval.integer = AMBIENT;
			    RET(AMBIENT); 
                        }
"diffuse"               {
                            strcat(line_buffer, yytext);
			    yylval.integer = DIFFUSE;
			    RET(DIFFUSE); 
                        }
"specular"              {
                            strcat(line_buffer, yytext);
			    yylval.integer = SPECULAR;
			    RET(SPECULAR); 
                        }
"highlight"             {
                            strcat(line_buffer, yytext);
			    yylval.integer = HIGHLIGHT;
			    RET(HIGHLIGHT); 
                        }
"hilite"                {
                            strcat(line_buffer, yytext);
			    yylval.integer = HIGHLIGHT;
			    RET(HIGHLIGHT); 
                        }
"shiny"                 {
                            strcat(line_buffer, yytext);
			    yylval.integer = SHINY;
			    RET(SHINY); 
                        }
"reflection"            {
                            strcat(line_buffer, yytext);
			    yylval.integer = REFLECTION;
			    RET(REFLECTION); 
                        }
"refraction"            {
                            strcat(line_buffer, yytext);
			    yylval.integer = REFRACTION;
			    RET(REFRACTION); 
                        }
"texname"               {
                            strcat(line_buffer, yytext);
			    yylval.integer = TEXNAME;
			    RET(TEXNAME); 
                        }
"fog"                   {
                            strcat(line_buffer, yytext);
			    yylval.integer = FOG_CMD;
			    RET(FOG_CMD);
                        }
"output"                {
                            strcat(line_buffer, yytext);
			    yylval.integer = OUTPUT;
			    RET(OUTPUT); 
                        }
"clear"                 {
                            strcat(line_buffer, yytext);
			    yylval.integer = CLEAR_BACK;
			    RET(CLEAR_BACK); 
                        }
"background"            {
                            strcat(line_buffer, yytext);
			    yylval.integer = BACKGROUND;
			    RET(BACKGROUND); 
                        }
"scissor"               {
                            strcat(line_buffer, yytext);
                            yylval.integer = SCISSOR;
                            RET(SCISSOR);
                        }
"viewport"              {
                            strcat(line_buffer, yytext);
                            yylval.integer = VIEWPORT;
                            RET(VIEWPORT);
                        }
"vp"                    {
                            strcat(line_buffer, yytext);
                            yylval.integer = VIEWPORT;
                            RET(VIEWPORT);
                        }
"vertex"                {
                            strcat(line_buffer, yytext);
			    yylval.integer = VERTEX;
			    RET(VERTEX); 
                        }
"vtx"                   {
                            strcat(line_buffer, yytext);
			    yylval.integer = VERTEX;
			    RET(VERTEX); 
                        }
"texture"              {
                            strcat(line_buffer, yytext);
			    yylval.integer = TEXTURE_FILE;
			    RET(TEXTURE_FILE);
                        }
"CLAMP"                 {
                            strcat(line_buffer, yytext);
			    yylval.integer = TXT_CLAMP;
			    RET(TXT_CLAMP);
                        }
"WRAP"                  {
                            strcat(line_buffer, yytext);
			    yylval.integer = TXT_WRAP;
			    RET(TXT_WRAP);
                        }
"MIRROR"                {
                            strcat(line_buffer, yytext);
			    yylval.integer = TXT_MIRROR;
			    RET(TXT_MIRROR);
                        }
"FILT"                  {
                            strcat(line_buffer, yytext);
			    yylval.integer = TXT_FILT;
			    RET(TXT_FILT);
                        }
"MODULATE"              {
                            strcat(line_buffer, yytext);
			    yylval.integer = TXT_MODULATE;
			    RET(TXT_MODULATE);
                        }
"tex"                   {
                            strcat(line_buffer, yytext);
			    yylval.integer = TEXTURE;
			    RET(TEXTURE); 
                        }
"sphere"                {
                            strcat(line_buffer, yytext);
			    yylval.integer = SPHERE;
			    RET(SPHERE); 
                        }
"sp"                    {
                            strcat(line_buffer, yytext);
			    yylval.integer = SPHERE;
			    RET(SPHERE); 
                        }
"trilist"               {
                            strcat(line_buffer, yytext);
			    yylval.integer = TRILIST;
			    RET(TRILIST); 
                        }
"loadobj"               {
                            strcat(line_buffer, yytext);
			    yylval.integer = LOADOBJ;
			    RET(LOADOBJ); 
                        }
%{
/* high-level matrix commands: */
%}
"identity"              {
                            strcat(line_buffer, yytext);
			    yylval.integer = IDENTITY;
			    RET(IDENTITY); 
                        }
"perspective"           {
                            strcat(line_buffer, yytext);
                            yylval.integer = PERSPECTIVE;
                            RET(PERSPECTIVE);
                        }
"orthographic"          {
                            strcat(line_buffer, yytext);
                            yylval.integer = ORTHOGRAPHIC;
                            RET(ORTHOGRAPHIC);
                        }
"depthrange"            {
                            strcat(line_buffer, yytext);
                            yylval.integer = DEPTHRANGE;
                            RET(DEPTHRANGE);
                        }
"depth"                 {
                            strcat(line_buffer, yytext);
                            yylval.integer = DEPTHRANGE;
                            RET(DEPTHRANGE);
                        }
"camera"                {
                            strcat(line_buffer, yytext);
			    yylval.integer = CAMERA;
			    RET(CAMERA); 
                        }
"pop"                   {
                            strcat(line_buffer, yytext);
			    yylval.integer = POP;
			    RET(POP); 
                        }
"scale"                 {
                            strcat(line_buffer, yytext);
			    yylval.integer = SCALE;
			    RET(SCALE); 
                        }
"rotate"                {
                            strcat(line_buffer, yytext);
			    yylval.integer = ROTATE;
			    RET(ROTATE); 
                        }
"translate"             {
                            strcat(line_buffer, yytext);
			    yylval.integer = TRANSLATE;
			    RET(TRANSLATE); 
                        }

%{
/* expressions and other syntax tokens: */
%}

\n			{
                            line_buffer[0] = '\0';
                            mylineno++;
			}
"/*"			{
			    skipcomments();
			}			
"#"			{
			    /* skip comments */
                            skippound();
			}
"~"	 		{
                            strcat(line_buffer, yytext);
			    yylval.integer = NOT_OP; 
			    RET(NOT_OP); 
			}			
"!"	 		{
                            strcat(line_buffer, yytext);
			    yylval.integer = NOT_OP; 
			    RET(NOT_OP); 
			}			
"&"	 		{
                            strcat(line_buffer, yytext);
			    yylval.integer = AND_OP; 
			    RET(AND_OP); 
			}			
"|"	 		{
                            strcat(line_buffer, yytext);
			    yylval.integer = OR_OP; 
			    RET(OR_OP); 
			}			
"^"	 		{
                            strcat(line_buffer, yytext);
			    yylval.integer = XOR_OP; 
			    RET(XOR_OP); 
			}			
">>"	 		{
                            strcat(line_buffer, yytext);
			    yylval.integer = RSHFT_OP; 
			    RET(RSHFT_OP); 
			}			
"<<"	 		{
                            strcat(line_buffer, yytext);
			    yylval.integer = LSHFT_OP; 
			    RET(LSHFT_OP); 
			}			
"*"	 		{
                            strcat(line_buffer, yytext);
			    yylval.integer = MULT_OP; 
			    RET(MULT_OP); 
			}			
"/"	 		{
                            strcat(line_buffer, yytext);
			    yylval.integer = DIV_OP; 
			    RET(DIV_OP); 
			}			
"%"	 		{
                            strcat(line_buffer, yytext);
			    yylval.integer = MOD_OP; 
			    RET(MOD_OP); 
			}			
"+"	 		{
                            strcat(line_buffer, yytext);
			    yylval.integer = PLUS_OP; 
			    RET(PLUS_OP); 
			}			
"-"	 		{
                            strcat(line_buffer, yytext);
			    yylval.integer = MINUS_OP; 
			    RET(MINUS_OP); 
			}			
"("	 		{
                            strcat(line_buffer, yytext);
			    yylval.integer = OP_PAREN; 
			    RET(OP_PAREN); 
			}			
")"			{
                            strcat(line_buffer, yytext);
			    yylval.integer = CL_PAREN; 
			    RET(CL_PAREN); 
			}
"["	 		{
                            strcat(line_buffer, yytext);
			    yylval.integer = OP_BRACKET; 
			    RET(OP_BRACKET); 
			}			
"]"			{
                            strcat(line_buffer, yytext);
			    yylval.integer = CL_BRACKET; 
			    RET(CL_BRACKET); 
			}
"{"	 		{
                            strcat(line_buffer, yytext);
			    yylval.integer = OP_CURLY;
			    RET(OP_CURLY); 
			}			
"}"			{
                            strcat(line_buffer, yytext);
			    yylval.integer = CL_CURLY;
			    RET(CL_CURLY); 
			}
","			{
                            strcat(line_buffer, yytext);
			    yylval.integer = COMMA; 
			    RET(COMMA); 
			}
";"			{
                            strcat(line_buffer, yytext);
			    yylval.integer = SEMICOLON;
			    RET(SEMICOLON); 
			}
0x[0-9a-fA-F]+		{   /* hex integer */
                            strcat(line_buffer, yytext);
			    sscanf(yytext, "%x", &yylval.integer);
			    RET(INTEGER);
			}
0X[0-9a-fA-F]+		{   /* hex integer */
                            strcat(line_buffer, yytext);
			    sscanf(yytext, "%x", &yylval.integer);
			    RET(INTEGER);
			}
[1-9][0-9]*		{   /* decimal integer */
                            strcat(line_buffer, yytext);
			    yylval.integer = atoi(yytext);
			    RET(INTEGER); 
			}			
0[0-7]*		        {   /* octal integer */
                            strcat(line_buffer, yytext);
			    yylval.integer = strtol(yytext, NULL, 8);
			    RET(INTEGER); 
			}			
[0-9]*\.[0-9]*		{   /* floating point */
                            strcat(line_buffer, yytext);
			    yylval.floater = atof(yytext);
			    RET(FLOATER); 
			}			
\"[^\"]*[\"]            {   /* quoted string */
                            strcat(line_buffer, yytext);
                            yylval.string = (char *) malloc(strlen(yytext)+1);
                            strcpy(yylval.string, yytext+1);
                            yylval.string[yyleng-2] = (char) NULL;
                            RET(QSTRING);
                        }
[ \t]*			{
                            strcat(line_buffer, yytext);
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
        mylineno = lineno-1;
        strcpy(input_file, fname);
    }
    mylineno++;
}
