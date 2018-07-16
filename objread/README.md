
A library that reads in Wavefront .obj files.

This library is required, librb.a depends on it. It is a separate library becuase
(1) it started out that way, and (2) it's cleaner than trying to manage a single
program with multiple instances of yacc/lex parser.

It imports the polygonal geometry.

Doesn't really support Materials or Textures (yet), just the geometry. It will
read and consume the commands, but not do anything with them.

Doesn't support groups well or handle smoothing groups. It will read and
consume the commands, but not do anything with them.

Doesn't handle any data other than polygonal faces (and it outputs triangles only)
It will probably barf on files with curved survaces, etc.

It has been tested with dozens of "free" .obj files downloaded off the internet and
object files exported from various big-boy modeling packages (3DS Max, Maya, 
Blender, ...)

Not production level by any means, but good enough to play and do some rendering.

