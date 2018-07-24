# Rendering-Plant
3D Rendering software framework

**_Rendering Plant_** is a 3D Rendering Software Framework... a bunch of code that does 
all the useful and necessary stuff to write a 3D renderer. It includes:

- scene file input parsing and loading
- scene, object, material and render state management
- geometry input (including Wavefront .obj files)
- geometry processing pipeline (transform, clip, project to screen coordinates)
- texture input and sampling
- utility functions

There are 4 renderers included with this package, allowing you to make pictures 
immediately or use them as examples to integrate your code into the framework.

- `moray` : a basic Whitted-stye ray tracer
- `scan` : a scanline rendering algorithm (much faster than the ray tracer with almost all of it's features)
- `paint` : a fast GPU-style "painter's" algorithm, with z-buffer.
- `draw` : a hidden-line renderer.


All together there are over 20K lines of C code plus documentation and another
10K+ lines of generated code.

