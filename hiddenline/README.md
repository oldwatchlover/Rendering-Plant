

This renderer is a simple line drawing renderer. 

First it paints the scene into the z-buffer using the painters algorithm (does
not modify color buffer)

Then it decoposes the polygonal geometry into "edges"...

Evaluating the edges, it identifies:

    - silhouette edges : an edge that belongs to only one (not culled) polygon, or
                         an edge shared between two polygons, where one poly is
                         culled and the other is not

    - culled edges     : all polygons that share the edge are culled

    - creases          : edges where the unculled polygons that share the edge have 
                         an angle that exceeds a threshold. (actually we calculate
                         the cos() of that angle using dot product of normals)

Then we draw only the edges that are creases or silhouettes, checking the z-buffer
as we do. When we draw the edges, we bias the z value just a little so we can use
the buffer to hide hidden edges but safely draw the silhouettes and creases. This
depends on a "bias factor" (always a bad idea in programming!) but is a common
technique in these types of algorithms, and this is intended to be a "quick and dirty"
implementation...

We should also identify any texture or material changes... however the renderer
does not currently do that.

("culled" above usually means "back-facing" but **_Rendering Plant_** handles
`CULL_BACK` or `CULL_FRONT`, similar to OpenGL allowing the user to select which
faces get culled, so it could mean "font-facing" if the scene was specified as such)

BUGS:

    - if multiple polygons get clipped, generating the same new clipped point,
      those polygons will not share a reference to that point, leading to
      incorrect silhouette edges being identified (the ground plane on 
      simple.in shows this issue).

      This is technically a bug (or side effect) of the clip code, but as that
      clip code was originaly written for a hardware graphics pipeline where 
      duplication of vertices is a good thing (performance wise) rather than
      searching the entire vertex array for potential points to re-use from 
      previously clipped triangles, I'm not inclined to fix that.


