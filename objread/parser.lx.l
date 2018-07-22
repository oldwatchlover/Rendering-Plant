
%option nounput
%option noyywrap
%option prefix="zz"
%{

/*
 * File:        parser.lx.l
 *
 * lex lexical analyser
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
#include "y.tab.h"


#undef zzLMAX
#define zzLMAX 4096

#ifdef LEXDEBUG
#	define RET(x) {fprintf(stderr,"lex %d [%s]\n", x, zztext); \
				return(x);}
#else
#	define RET(x) return(x);
#endif

static void skippound(void);
extern char _RPObjline_buffer[];
extern char _RPObjinput_file[];
extern int  obj_mylineno;

%}

%%

%{
/* language keywords: */
%}

"v"                     {
                            strcat(_RPObjline_buffer, zztext);
			    zzlval.integer = VERTEX;
			    RET(VERTEX); 
                        }
"vt"                    {
                            strcat(_RPObjline_buffer, zztext);
			    zzlval.integer = VTEXCOORDS;
			    RET(VTEXCOORDS); 
                        }
"vn"                    {
                            strcat(_RPObjline_buffer, zztext);
			    zzlval.integer = VNORMALS;
			    RET(VNORMALS); 
                        }
"vp"                    {
                            strcat(_RPObjline_buffer, zztext);
			    zzlval.integer = VPARAMS;
			    RET(VPARAMS); 
                        }
"f"                     {
                            strcat(_RPObjline_buffer, zztext);
			    zzlval.integer = FACE;
			    RET(FACE); 
                        }
"o"                     {
                            strcat(_RPObjline_buffer, zztext);
			    zzlval.integer = OBJECT;
			    RET(OBJECT); 
                        }
"g"                     {
                            strcat(_RPObjline_buffer, zztext);
			    zzlval.integer = GROUP;
			    RET(GROUP); 
                        }
"s"                     {
                            strcat(_RPObjline_buffer, zztext);
			    zzlval.integer = SMOOTHGROUP;
			    RET(SMOOTHGROUP); 
                        }
"mtllib"                {
                            strcat(_RPObjline_buffer, zztext);
			    zzlval.integer = MTLLIB;
			    RET(MTLLIB); 
                        }
"usemtl"                {
                            strcat(_RPObjline_buffer, zztext);
			    zzlval.integer = USEMTL;
			    RET(USEMTL); 
                        }
"maplib"                {
                            strcat(_RPObjline_buffer, zztext);
			    zzlval.integer = MAPLIB;
			    RET(MAPLIB); 
                        }
"usemap"                {
                            strcat(_RPObjline_buffer, zztext);
			    zzlval.integer = USEMAP;
			    RET(USEMAP); 
                        }
%{
/* expressions and other syntax tokens: */
%}

\n			{
                            _RPObjline_buffer[0] = '\0';
                            obj_mylineno++;
			}
\r			{
                            _RPObjline_buffer[0] = '\0';
			}
"#"			{
			    /* skip comments */
                            skippound();
			}
0x[0-9a-fA-F]+		{   /* hex integer */
                            strcat(_RPObjline_buffer, zztext);
			    sscanf(zztext, "%x", &zzlval.integer);
			    RET(INTEGER);
			}
0X[0-9a-fA-F]+		{   /* hex integer */
                            strcat(_RPObjline_buffer, zztext);
			    sscanf(zztext, "%x", &zzlval.integer);
			    RET(INTEGER);
			}
0[0-7]*		        {   /* octal integer */
                            strcat(_RPObjline_buffer, zztext);
			    zzlval.integer = strtol(zztext, NULL, 8);
			    RET(INTEGER); 
			}			
[+-]?[1-9][0-9]*	{   /* decimal integer */
                            strcat(_RPObjline_buffer, zztext);
			    zzlval.integer = atoi(zztext);
			    RET(INTEGER); 
			}			
[-+]?[0-9]*\.[0-9]*     {   /* floating point */
                            strcat(_RPObjline_buffer, zztext);
			    zzlval.floater = atof(zztext);
			    RET(FLOATER); 
			}			
[-+]?[0-9]*\.[0-9]*[eE][-+][0-9]*     {   /* floating point e-notation */
                            strcat(_RPObjline_buffer, zztext);
			    zzlval.floater = atof(zztext);
			    RET(FLOATER); 
			}			
[a-zA-Z0-9._/-]*         {   /* string */
                            strcat(_RPObjline_buffer, zztext);
                            zzlval.string = (char *) malloc(strlen(zztext));
                            strcpy(zzlval.string, zztext);
                            RET(STRING);
                        }
[ \t]*			{
                            strcat(_RPObjline_buffer, zztext);
			    /* skip white space */
			}			
.			{
			    fprintf(stderr,"scanner error\n");
			    fprintf(stderr,"unrecognized char [%s]\n", zztext);
			}
%%

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
        obj_mylineno = lineno-1;
        strcpy(_RPObjinput_file, fname);
    }
    obj_mylineno++;
}


