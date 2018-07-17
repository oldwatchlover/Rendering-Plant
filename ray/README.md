
# moray

A recursive, Whitted-style ray tracing algorithm.

It renders spheres and triangle-based data, supports texture mapping
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

Limitations:

    - spheres are implemented as implicit geometry and their texture coordinates are generated with a straightforward spherical mapping.

    - only point light sources are currently supported.

    - only one texture per object.

    - filtered texture sampling is not supported.

    - mip mapping is not supported.

    - no bump mapping reflection mapping.

    - simple Blinn-Phong type lighting model (no global illumination, radiosity, or depth of field, etc.)



