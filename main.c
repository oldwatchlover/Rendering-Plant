
/*
 * File:	main.c
 *
 *
 * This is the main() routine for the "Rendering Plant" 3D render platform.
 *
 * This main() function drives whatever renderer is linked via it's own library
 * and the rp platfrom library.
 *
 * There are some #defines in here that are renderer specific, this file gets
 * compiled into multiple renderer executables.
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
#include <locale.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>

#include "rp.h"

#ifdef MORAY
#   include "ray.h"
#   define PROGRAM_VERSION	"2.0"
#   define USAGE_STRING "[-D ...] [-I ...] [-b] [-d[d]] [-m samples] [-p] [-v] [-y] scenefile"
    static int		use_multisample = 0;
#endif
#ifdef SCAN
#   include "scan.h"
#   define PROGRAM_VERSION	"0.6"
#   define USAGE_STRING "[-D ...] [-I ...] [-b] [-d[d]] [-m samples] [-p] [-v] [-y] scenefile"
#endif
#ifdef PAINT
#   include "paint.h"
#   define PROGRAM_VERSION	"1.0"
#   define USAGE_STRING "[-D ...] [-I ...] [-b] [-d[d]] [-p] [-v] [-y] scenefile"
#endif

/* for scene input file parser: */
int	mylineno;
char	line_buffer[MAX_FILENAME_LENGTH];
char	input_file[MAX_FILENAME_LENGTH];

/* variables from the parser: */
extern FILE *yyin;
extern int yydebug;
extern int yyparse(void);

/* local declarations for this file: */
static int	debug = 0;
static int	pause = 0;
static void 	init_input_memory(void);
static void 	free_input_memory(void);


/*
 * main routine.
 *
 */
int
main(int argc, char *argv[])
{
    static rgba_t	sky_blue = {135, 206, 235, MAX_COLOR_VAL};
    struct stat		statbuffer;
    char		cppcmd[1024], cppdefs[512], usage_string[256], 
			use_blue = FALSE;
    clock_t		begin, end;
    double		elapsed;

    setprogname(argv[0]);
    setlocale(LC_ALL,"");
    strcpy(cppdefs, "");
    yydebug = 0;
    sprintf(usage_string,"usage : %s %s", argv[0], USAGE_STRING);

    RPInit(argv[0], 0x0);	/* must call this first */

    while ((argc > 1) && (argv[1][0] == '-')) {
	switch(argv[1][1]) {

          case 'D':
	    sprintf(cppdefs, "%s %s", cppdefs, argv[1]);
	    break;
	    
          case 'I':
	    sprintf(cppdefs, "%s %s", cppdefs, argv[1]);
	    break;
	    
          case 'b':
	    use_blue = TRUE;
	    break;
	    
	  case 'd':
	    debug++;
	    if (argv[1][2] == 'd')
	        debug++;
	    break;
	    
#ifdef  MORAY	    /* only ray tracer does multisampling */
	  case 'm': /* option flag to set multisampling parameter: */
	    use_multisample = Clamp0x(atoi(argv[2]), 5);
	    argc--;
	    argv++;
	    break;
#endif
	  case 'p':
	    pause = TRUE;
	    break;
	    
	  case 'v':
	    if (debug == 0)
	        debug++;
	    break;

	  case 'y':
	    yydebug = TRUE;
	    break;
	    
	  default:
	    fprintf(stderr," %s : ERROR : program option [%s] not recognized.\n", 
			program_name, argv[1]);
	    fprintf(stderr,"%s\n",usage_string);
	    break;
	}
	argc--;
	argv++;
    }

    if (argc < 2) {
	fprintf(stderr," %s : ERROR : no input file specified.\n",program_name);
	fprintf(stderr,"%s\n",usage_string);
	exit(EXIT_FAILURE);
    }

    fprintf(stderr,"%s : Version %s, compiled %s against Rendering Plant version %s\n",
	    program_name, PROGRAM_VERSION, __DATE__, RP_VERSION);

    if (pause) {	/* pause execution to set up profiling */
  	int	c;
	c = fgetc(stdin);
    }

    strcpy(input_file, argv[1]);

    if (stat(input_file, &statbuffer) != 0) {
	fprintf(stderr,"%s : ERROR : can't open input file [%s].\n",program_name,input_file);
	exit(EXIT_FAILURE);
    }

    /* some environments prefer this... "/usr/bin/gcc -x c-header -E "... */
    sprintf(cppcmd,"/usr/bin/cpp %s %s ", cppdefs, input_file);
    
    if ((yyin = (FILE *) popen(cppcmd, "r")) == NULL) {
	fprintf(stderr,"ERROR : %s : could not process [%s] errno = %d\n",
		__FILE__,input_file,errno);
	exit(EXIT_FAILURE);
    }

    init_input_memory();
    RPInitScene();

	/* let command flags override scene flags for a few things: */
    if (debug == 2)
	RPSetSceneFlags(FLAG_VERBOSE2);
    else if (debug == 1)
	RPSetSceneFlags(FLAG_VERBOSE);

    if (use_blue)
        RPSetBackgroundColor(&sky_blue);

#ifdef  MORAY
    if (use_multisample > 0) {
	RPSetSceneFlags(FLAG_SCENE_MULTISAMPLE);
	RPScene.num_samples = use_multisample;
    }
#endif

#if (defined  PAINT || defined SCAN)
	/* if the renderer does not support implicit sphere geometry: */ 
    RPEnableSphereSupport(FALSE);
#endif

    fprintf(stderr,"%s : parsing input file [%s]...\n",
	    program_name,input_file);

    if (yyparse())
	exit(EXIT_FAILURE);
    
    fclose(yyin);

    free_input_memory();

	/* any other "beginning of time" renderer init goes here */

    begin = clock();

	    /* render scene */
#if defined (MORAY)
    raytrace_scene();
#elif defined (PAINT)
    paint_scene();
#elif defined (SCAN)
    scan_scene();
#else
    fprintf(stderr,"%s : ERROR : no scene renderer defined!\n",program_name);
#endif

    end = clock();

    elapsed = (double)(end - begin) / CLOCKS_PER_SEC;
    fprintf(stderr,"%s : rendering took %lf seconds.\n",program_name,elapsed);

    if (!RPWriteColorFB()) {	/* output the final image */
	fprintf(stderr,"ERROR : %s : cannot write image to file.\n", program_name);
    }

    exit(EXIT_SUCCESS);
}

static void
init_input_memory(void)
{
    RPInitInputVertices();
    RPInitInputPolygons();
}

static void
free_input_memory(void)
{
    RPFreeInputVertices();
    RPFreeInputPolygons();
}

