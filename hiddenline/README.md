
# _draw_ - A Simple Hidden Line Drawing Renderer

This render generates line drawings, black ink on a white background.

There is an additional command line option `-t` which has the same effect
as including `genericflags(RENDER03);` in your input file. 

It links with `libpaint.a` and uses some of that library's core featurs.

### ALGORITHM

First it paints the scene into the z-buffer using the painters algorithm (does
not modify color buffer)

Then it decoposes the polygonal geometry into "edges"...

Evaluating the edges, it identifies:

    - silhouette edges : an edge that belongs to only one (not culled) polygon, or
                         an edge shared between two polygons, where one poly is
                         culled and the other is not

    - culled edges     : an edge shared by two culled polygons is also culled

    - creases          : edges where the unculled polygons that share the edge have 
                         an angle that exceeds a threshold. (actually we calculate
                         the cos() of that angle using dot product of normals)

    - material         : edges where the shared polygons have different materials,
                         resulting in an expected visual disconinuity.

Then we draw only the edges that are creases, silhouettes, or material changes, 
checking the z-buffer as we do. When we draw the edges, we bias the z value just a 
little so we can use the buffer to hide hidden edges but safely draw the silhouettes 
and creases. This depends on a "bias factor" (always a bad idea in programming!) but is a common
technique in these types of algorithms, and this is intended to be a "quick and dirty"
implementation...

("culled" above usually means "back-facing" but **_Rendering Plant_** handles
`CULL_BACK` or `CULL_FRONT`, similar to OpenGL allowing the user to select which
faces get culled, so it could mean "font-facing" if the scene was specified as such)

**_draw_** accepts in the input stream these `genericflags()` scene flags:

    RENDER01 : disable clipping.

    RENDER03 : render the scene in color before rendering the edges - if you
               set your material properties correctly this can create a very
               effective "toon shading" effect.

### IMPLEMENTATION LIMITATIONS

Edge detection and memory usage is very straightforward and not optimized.

Progress feedback from the program is not very helpful.

### BUGS:

    - if multiple polygons get clipped, generating the same new clipped point,
      those polygons will not share a reference to that point, leading to
      incorrect silhouette edges being identified (the ground plane on 
      simple.in shows this issue).

      This is technically a bug (or side effect) of the clip code, but as that
      clip code was originaly written for a hardware graphics pipeline where 
      duplication of vertices is a good thing (performance wise) rather than
      searching the entire vertex array for potential points to re-use from 
      previously clipped triangles, I'm not inclined to fix that.

### IDEAS FOR FUTURE WORK

    - handle textures better. Texel changes could generate visual disconinuities
      creating an edge, similar to material changes.

    - silhouette dection could be better In some cases the silhouette edge is rendered
      as a broken line (due to bias/depth buffer tests). While this may be ok for crease
      edges, it is not ok for object silhouettes. 


