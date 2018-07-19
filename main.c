
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

#include "rp.h"

#ifdef MORAY
#   include "ray.h"
#   define PROGRAM_VERSION	"2.0"
#   define USAGE_STRING "[-D ...] [-I ...] [-b] [-d[d]] [-m samples] [-v] [-y] scenefile"
    static int		use_multisample = 0;
#endif
#ifdef SCAN
#   include "scan.h"
#   define PROGRAM_VERSION	"0.8"
#   define USAGE_STRING "[-D ...] [-I ...] [-b] [-d[d]] [-v] [-y] scenefile"
#endif
#ifdef PAINT
#   include "paint.h"
#   define PROGRAM_VERSION	"1.0"
#   define USAGE_STRING "[-D ...] [-I ...] [-b] [-d[d]] [-v] [-y] scenefile"
#endif


/*
 * main routine.
 *
 */
int
main(int argc, char *argv[])
{
    static rgba_t	sky_blue = {135, 206, 235, MAX_COLOR_VAL};
    char		cppdefs[512], usage_string[256];
    clock_t		begin, end;
    double		elapsed;
    int			use_blue = FALSE, debug = FALSE, parsedebug = FALSE;

    setprogname(argv[0]);
    setlocale(LC_ALL,"");
    strcpy(cppdefs, "");
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

	  case 'v':
	    if (debug == 0)
	        debug++;
	    break;

	  case 'y':
	    parsedebug = TRUE;
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

    RPInitScene();
    RPInitInputVertices();
    RPInitInputPolygons();

	/* let command flags override default scene flags for a few things: */
	/* (could still be changed by the input file) */
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

    if (!RPParseInputFile(argv[1], parsedebug, cppdefs)) {
	fprintf(stderr,"%s : ERROR : could not parse input file [%s], exiting...\n",
			program_name, argv[1]);
	exit(EXIT_FAILURE);
    }

    RPFreeInputVertices();
    RPFreeInputPolygons();

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


