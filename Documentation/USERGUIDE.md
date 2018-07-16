# Rendering Plant User Guide

### SYNOPSIS

        <renderer> [options] input_file

### DESCRIPTION

**_Rendering Plant_** is a framework to build 3D rendering software.

The framework consists of two libraries: `librp.a` and `libobj.a`.

The sample renderers are also compiled into libraries and there is
`main.c` which links to the libraries to create the renderer executable.

This guide describes the usage of the sample renderers included with
the **_Rendering Plant_** distribution:

        moray           a simple ray tracer.
        paint           a GPU style painter's algorithm.
        scan            a scanline renderer.

The `input_file` is a text based scene description to render (the full input 
language is described briefly below and ad nauseum in another document.

Behavior and limitations of each sample renderer is documented in their respective
directories.

### OPTIONS
The following options are available:

    -D          Similar to -D for a compiler, this adds an implicit #define 
                into the preprocessor buffer which is read before the scene
                file is preprocessed (before being interpreted by the ray
                tracer, the scene file is passed through /usr/bin/cpp so
                that the scene file can use #include, #define, etc., as well
                as C language style comments.


    -I <dir>    Add the specified directory to the search path for include
                files used for the scene input file.


    -b          Use a pleasing sky blue background color rather than the default
                black.


    -d[d]       Verbose [-d] and even more verbose [-dd] diagnostic information
                is printed while running. Useful for debugging.


    -m <samp>   Number of samples per image pixel. Only the primary rays
                (from the camera through the frame buffer) support multsampling;
                <samp> * <samp> samples are cast for each primary ray and averaged
                to determine the final frame buffer pixel value.


    -y          Enable debug statements from yacc (bison).



### SCENE INPUT FILE FORMAT

See 
[RP-INPUT-FORMAT.md](http://github.com/oldwatchlover/Rendering-Plant/blob/master/Documentation/RP-INPUT-FORMAT.md) 
for a more detaled description of the input file format.

Briefly, the input file format is a text file, with statements that look
similar to C style function calls or data structures.  C style comments, and 
white space  are permitted. The input file is run through the C pre-processor 
before parsing, providing support for macro expansion, include files, etc.
in the input file.

The language does not support some concepts that would be found in a true
production level 3D renderer (animation frame support, complex object 
formats, etc.).

The naming and concepts (such as matrix naming/processing, coordinate space,
and render state) are aligned pretty well with OpenGL.

A (heavily commented) sample input file that uses many of the common features 
looks like this:

```


        #define XRES    1920
        #define YRES    1080

        /*
         * scene flags affect the entire scene... you cannot turn flags on or off
         * between objects
         */
        sceneflags(VERBOSE ZBUFFER);

        ## uncomment these to enable fog and test fog
        ##sceneflags(FOG);
        ##fog(1000.0, 6000.0, 1.0, 1.0, 1.0, 1.0);

        /*
         * output image file and resolution, with a pleasing blue background
         */
        output("simple.bmp", XRES, YRES);
        clear(0.5294, 0.8078, 0.9216, 1.0); # r,g,b,a (asm lang style comments ok)

        /*
         * camera to render the scene. Paramters are:
         *          position         looking at      up      fov    aspect ratio
         *      x     y     z       x    y    z    x  y  z  degrees   x/y
         */
        camera(0.0, 200.0, 2500.0, 0.0, 10.0, 0.0, 0, 1, 0,  20.0, XRES/YRES);
        depthrange(100.0, 10000.0);

        /*
         * light is position (x,y,z) and color (r,g,b)
         */
        light(-2000.0, 1000.0, 3000.0, 1.0, 1.0, 1.0);
        light(0.0, 1000.0, 0.0, 1.0, 1.0, 1.0);

        /* clear all object flags: */
        objflags(~ALLFLAGS);
        objflags(TEXTURE VERTSHADE LIGHTING CULL_BACK);

        /* load texture and set material parameters */
        texture("Texture/checker.bmp", WRAP MODULATE, 4.0, 4.0, 0.0, 0.0);
        material(color,    1.0, 1.0, 1.0, 1.0);
        material(ambient,  0.2, 0.2, 0.2, 1.0);
        material(diffuse,  0.6, 0.6, 0.6, 1.0);
        material(specular, 0.1, 0.1, 0.1, 1.0);
        material(shiny,    1.0);
        material(texname,  "Texture/checker.bmp");

        /* see OpenGl docs for details about the importance of order when
         * combining transformations to create the model matrix.
         *
         * you should scale, then rotate, then translate
         *
         * since the matrix stack multiplies matrices in reverse order, you
         * should always specify them in this order in the scene file:
         */
        identity(MTX_MODEL);            # start with identity modeling matrix
        translate(0.0, 0.0, 0.0);       # translate x,y,z
        rotate(0.0, 0.0, 1.0, 0.0);     # rotate r degrees around vector x,y,z
        scale(4.0, 1.0, 2.0);           # scale x,y,z

        /*
         * a direct way to describe geometry is to provide the vertex list
         * and triangle list.
         */
        vertex[4] {
        ## |--model space coord--||----- color ------||- tex coord -|
        ## | x        y       z  ||  r    g    b   a ||  s     t    |
          {-800.0, -200.0, -800.0,   0, 1.0,   0, 1.0, 0.000, 0.000},
          { 800.0, -200.0,  500.0,   0,   0, 1.0, 1.0, 1.000, 1.000},
          { 800.0, -200.0, -800.0,   0,   0,   0, 1.0, 1.000, 0.000},
          {-800.0, -200.0,  500.0, 1.0,   0,   0, 1.0, 0.000, 1.000}
        };

        trilist[2] {
        ## index into vertex array above describes the triangles
            { 0, 1, 2 },
            { 0, 3, 1 },
        };

        objflags(~ALLFLAGS);
        objflags(LIGHTING);

        /* transparent ball at center of screen: */
        identity(MTX_MODEL);
        material(color, 1.0, 1.0, 1.0, 1.0);
        material(ambient, 0.2, 0.2, 0.2, 0.1);
        material(diffuse, 0.6, 0.6, 0.6, 0.1);
        material(specular, 1.0, 1.0, 1.0, 0.1);
        material(shiny, 180.0);
        material(reflection, 0.8);
        material(refraction, 1.4);
        sphere(0.0, -100.0, 0.0, 100.0);

        /* blue ball on right: */
        identity(MTX_MODEL);
        material(color, 0.0, 0.0, 1.0, 1.0);
        material(ambient, 0.0, 0.0, 0.2, 1.0);
        material(diffuse, 0.0, 0.0, 0.6, 1.0);
        material(specular, 1.0, 1.0, 1.0, 1.0);
        material(shiny, 180.0);
        material(reflection, 0.0);
        material(refraction, 0.0);
        sphere(400.0, 100.0, -200.0, 200.0);

        /* reflective ball on left: */
        identity(MTX_MODEL);
        material(color, 1.0, 1.0, 1.0, 1.0);
        material(ambient, 0.2, 0.2, 0.2, 1.0);
        material(diffuse, 0.6, 0.6, 0.6, 1.0);
        material(specular, 1.0, 1.0, 1.0, 1.0);
        material(shiny, 200.0);
        material(reflection, 0.8);
        material(refraction, 0.0);
        sphere(-450.0, 200.0, -450.0, 250.0);

        /* red ball in the distance: */
        identity(MTX_MODEL);
        material(color, 1.0, 0.0, 0.0, 1.0);
        material(ambient, 0.2, 0.0, 0.0, 1.0);
        material(diffuse, 0.6, 0.0, 0.0, 1.0);
        material(specular, 1.0, 1.0, 1.0, 1.0);
        material(shiny, 120.0);
        material(reflection, 0.0);
        material(refraction, 0.0);
        sphere(200.0, 100.0, -2500.0, 200.0);

        objflags(~ALLFLAGS);
        objflags(FLATSHADE LIGHTING CULL_BACK);

        /* a tall yellow box */
        identity(MTX_MODEL);
        translate(-170.0, -200.0, -200.0);
        rotate(45.0, 0.0, 1.0, 0.0);
        scale(100.0, 300.0, 100.0);

        material(color, 1.0, 1.0, 0.0, 1.0);
        material(ambient, 0.2, 0.2, 0.0, 1.0);
        material(diffuse, 0.7, 0.7, 0.0, 1.0);
        material(specular, 0.3, 0.3, 0.3, 1.0);
        material(shiny, 20.0);
        material(reflection, 0.0);
        material(refraction, 0.0);

        /*
         * geometry can also be loaded from a Wavefront .obj file:
         */
        loadobj("obj/cube.obj");

```

The scene rendered by the sample input file above looks like this:

<img src="../Images/simple.bmp"
     alt="Simple moray Scene Rendering"/>
     

### BUILDING A RENDERER WITH **_RENDERING PLANT_**

Required steps to build a renderer with **_Rendering Plant_**:

- include `rp.h` in your renderer source code. In turn, that file will include
`rp_defines.h`, `rp_types.h` and `rp_externs.h`. Become familiar with those
files as they define the interface to the **_Rendering Plant_** framework.

- link with `librp.a` and `libobj.a` libraries. Both are required; even if
you are not using Wavefront `.obj` files, the scene parser needs that library
to link successfully.

- add a few lines of code in `main.c` or if you create your own `main()` function,
follow these steps:

    - set up yacc/lexx (bison/flex) properly, see `main.c` (certain variables are expected; the input should be piped through the C preprocessor, etc.).

    - call `RPInit()` before doing anything else.

    - call `RPInitInputVertices()`, `RPInitInputPolygons()` and `RPInitScene()` before parsing the input file.

    - after parsing the input file, call your main scene render entry point.

    - in your main scene render function, you should:

        - clear the screen and zbuffer if desired.
        - set any desired state.
        - call `RPProcessObjects()`. After this, all objects will be in camera space, transformed with the camera at the origin looking down the -Z axis. If you pass `TRUE` to that function, the geometry will also be projected, generating clip codes and screen coordinates.
        - you should then be able to use the `RPScene` and other data structures to create your rendered image.
        - you can call `RPWriteColorFB()` to output the final image.

See the sample code and `Makefile`'s for details.

### LIMITATIONS


### BUGS

