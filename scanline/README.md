
A scanline algorithm 3D render.


Actually, it's not a full scanline algorithm (Watkins, Newell-Newell-Sancha, or
REYES A-buffer, ...) but more properly a scanline frontend for a ray tracing
back end.

It sorts the polygons into scanline bucket lists, then creates edge-pairs or spans
of the polygons that touch that scanline.

As it processes across each edge-pair on a given scanline, it casts a camera
ray into the polygons that touch that pixel. This is a very simple and fairly
powerful accleration when compared to a classic ray tracing algorithm - each
pixel is typically covered by only a handful of polygons rather than the thousands
(or more!) that might exist in the scene and would need to be intersected with
a brute force algorithm.

Pixel shading is calculated with the ray tracing model, including secondary rays
for shadows, reflection and refraction.


Limitations:

    - it is a polygonal renderer. Only triangles (or input that can be broken up
      into triangles). It is built with my librp.a framework, which will
      approximate spheres with polygonal data automatically.

    - it has all the limitations of "moray" (libray.a), with respect to shading
      and texturing.


It is a toy... no super fancy acceleration techniques, no super fancy global illumination
models, etc.


