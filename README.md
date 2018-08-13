# Rendering-Plant

**_Rendering Plant_** is a 3D Rendering Software Framework... a bunch of code that does 
all the useful and necessary stuff to write a 3D renderer. It includes:

- scene file input parsing and loading
- scene, objects, lights, and materials loading  and render state management
- geometry input (including Wavefront .obj files)
- geometry processing pipeline (transform, clip, project to screen coordinates)
- texture input and sampling
- final image output to a Microsoft BMP file
- other utility functions

There are 4 renderers included with this package, allowing you to make pictures 
immediately or use them as examples how to use the framework or integrate your
own renderer.

`moray` - a basic Whitted-stye ray tracer

`scan` - a scanline rendering algorithm (much faster than the ray tracer with almost all of it's features)

`paint` - a fast GPU-style "painter's" algorithm, with z-buffer.

`draw` - a hidden-line renderer.

All together there are over 20K lines of C code plus documentation and another
10K+ lines of generated code.

Most of the code is in the framework... the renderers consist of some specific code
that links into their library, with the renderers sharing `main.c` 
(using some `#define` code) to create each renderer executable.

See 
[USERGUIDE.md](http://github.com/oldwatchlover/Rendering-Plant/blob/master/Documentation/USERGUIDE.md) 
for usage and command line options for the renderers that share `main.c`

For **_Rendering Plant_** input format, see 
[RP-INPUT-FORMAT.md](http://github.com/oldwatchlover/Rendering-Plant/blob/master/Documentation/RP-INPUT-FORMAT.md) 
for a detaled description of the input file format.

There are also some sample scenes in `./Scenes` and corresponding images of what those
scenes shoud look like in `./Images`



