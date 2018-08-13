
# _moray_ - A Recursive, Whitted-Style Ray Tracing Algorithm

### ALGORITHM

This is a simple, elegant implementation of a recursive ray tracing algorithm
as presented in Turner Whitted's classic paper 
_An Improved Illumination Model for Shaded Display._<sup>[1](#whittedref)</sup> 

It traces rays forwards, into the scene, from the camera (eye) through the image plane (per pixel)
and into world space (a "primary ray"), intersecting with all objects in the scene.

At the point of intersection for a primary ray eminating from the camera, "secondary" rays
are cast into the scene, tracing shadows (from the intersection point to each light source, looking
for objects that may block that light), reflections (in the reflected direction), and refraction
(for transparent objects, using Fresnel's equations to "bend" the refreacted ray direction).

The closest object in the world that intersects the primary ray (and any contribution from it's
secondary rays) is used to determine the color of the screen pixel.

It renders implicit spheres and triangle-based data, supports texture mapping
and has a decent "ray tracing" shading model that includes shadows, reflection, refraction,
and an OpenGL-like Blinn-Phong shading model.

It is designed for simplicity and flexibility, not speed. It does not incorporate any
of the acceleration techniques of serious ray tracers, nor does it create super
realistic global illumination models or depth of field, etc. 

This program adds an extra command line argument, `-m <numsamples>` permitting
multiple samples per primary ray. At each screen pixel, `<numsamples> * <numsamples>` are
cast into the scene and averaged to determine that pixel value. The maximum value
for `<numsamples>` is `5` (25 rays per pixel).  Secondary rays do not support
multisampling.

<a name="whittedref"><sup>1</sup></a>Turner Whitted, _An Improved Illumination Model for 
Shaded Display,_ Comunications of the ACM, Vol. 23 Issue 6, June 1980.

### IMPLEMENTATION LIMITATIONS

    - spheres are implemented as implicit geometry and their texture coordinates are generated with a straightforward spherical mapping.

    - only point light sources are currently supported.

    - only one texture per object.

    - filtered texture sampling is not supported.

    - mip mapping is not supported.

    - no bump mapping or reflection mapping.

    - simple Blinn-Phong type lighting model (no global illumination, radiosity, or depth of field, etc.)


### IDEAS FOR FUTURE WORK

Some ideas for improvements:

    - (remove some of the implementation limitations above).

    - Add acceleration techniques (space partitioning, etc.)

    - Add multi-sampling to secondary rays (cone tracing, etc.)

    - Handle self-intersections better.

    - 

