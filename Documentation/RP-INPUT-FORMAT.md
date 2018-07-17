
# Rendering Plant Input Format

This document describes the input file format for the **_Rendering Plant_** framework. As a
fairly full-featured 3D rendering platform, the input format is quite rich.

The integrated sample renderers require an input file as an argument on the 
command line and this file is read in to describe the scene to render.

I imagine anyone that is interested in this fun piece of code will dive into the
acutal code, so I'm not going to write a professional quality user manual. However,
there is a lot of code and features to digest, so at least a brief outline of
the input language seems appropriate. By looking at a few sample files and using
this document for reference you should be able to create your own scenes
and follow along in the source code to add your own renderer or features.

#### C Preprocessor

Before the renderer parses the input file, the input file is passed through the 
C preprocessor. This allows the input file to use C preprocessor directives, such
as:

`#include`, allowing you to subdivide the input data (such as large data files) 
into separate files, which are then included in a top level scene file.

`#define`, constants and macros. You can define constants and macros to make the 
input file more readable, flexible or easier for debugging.

_comments_, two types of comments are supported to make your input file more readable.

and any other useful features of the C preprocessor that you wish to take advantage of.

#### Comments

Since the C preprocessor strips whitespace and C style comments, you may use C style
comments in your file:

```
    /* this is a comment that is legal moray input */

    /*
     * so is this
     *
     */
```

Additionally, the  input parser recognizes `#` as assembly language style comments.
So this is very useful to annotate your file without using extreme vertical spacing:

```
    clear(0.5294, 0.8078, 0.9216, 1.0);      # this is a pleasing sky blue color
```

However, you must be careful as the `#` in the first column will be interpreted
by the C preprocessor... if you use `##` in the first column, that will
create a comment with no error. (A full explanation of why this works is beyond
the scope of this document, but if you are curious you can run the input file 
through the C preprocessor manually and look at the file generated... the 
preprocessor generates diagnostic and information commented with the `#` character, 
so (like other compilers that use the C preprocessor) **_Rendering Plant_** is made to digest 
these comments.

So this is a legal comment (`##` beginning in column 1):

```
    ##sceneflags(NOSHADOW);	# turn off shadows
```

But this will generate a C preprocessor error (`#` beginning in column 1):

```
    #sceneflags(NOSHADOW);	# turn off shadows
```
like this:

```
    % moray Scene/simple.in
    moray : Version 0.6, dated Jun 17 2018
    moray : parsing input file [Scene/simple.in]...
    Scene/simple.in:61:2: error: invalid preprocessing directive
    #sceneflags(NOSHADOW);

```

#### Data Values

**_Rendering Plant_** input is fairly flexible about data types. While some commands do require
an integer or floating point value - how those are specified can take many forms.


        hexadecimal       beginning with 0x or 0X, followed by [0-F] or [0-f]
        octal:            beginning with 0, followed by [0-7]
        decimal integer   base 10, no decimal
        floating point    base 10, with a decimal. This data type supercedes integer,
                          so a floating point value such as 10 or 10.0 are equivalent.
        text strings      text appearing in double quotes...`"like this"` 

An integer is legal anywhere a float is required, however the opposite is not true.

#### Expressions

Numeric and logical expressions are permitted in the appropriate context. The
following are legal input tokens:

```
        ~, &, |, ^, >>, <<, *, /, %, +, -, (, ), 
```

Expression evaluation precedence occurs as you might expect:

We have 3 precedence levels:

        most binding, highest precedence:       unary  -,~
                                                binary *,/,%,<<,>>,^,&,|
        least binding, lowest precedence:       binary +,-

#### Filenames

Several commands require a file name as a quoted text string. 

File path searching for scene files can be confusing because the files
specified in scene commands are processed differently than files proessed by
the C preprocessor.

So

```
        output("simple.bmp", 1920, 1080);
```

and

```
        output("./simple.bmp", 1920, 1080);
```

are equivalent, creating the output image in the directory where the program was invoked..

However, for `#include` files... that path is determined by the C preprocessor 
(and potentially any search directories passed in with the `-I` command line argument).
 
For example, if you invoke *moray* like this:

```
        % moray ./Scene/simple.in
```

and an include file with some scene data is contained in `simple.in`:

```
        #include "Data/unitbox.h"
```
then the file `unitbox.h` will be expected to be found in `./Scene/Data` while
the file:

```
        texture("Texture/checker.bmp", WRAP, 1.0, 1.0, 0.0, 0.0);
```

would be expected to be found in the directory `./Texture` relative to where
*moray* was invoked:

```
        % ls
          Documentation/	LICENSE		README.md	Texture/	simple.bmp
          Images/		Makefile	Scene/		moray*		src/
        % ls Scene
          Data/                 simple.in
        % ls Scene/Data
          unitbox.h
        % ls Texture
          checker.bmp
```

I keep my directory structure as above or use Unix symlinks to access the data from the
directory where I am working.


## Scene State Commands

A scene input file contains required information about the global scene (cameras,
lights, ...), meta-data about the scene (output file, debugging flags, ...) and
objects in the scene (geometry, textures, materials) to be rendered.

Remember - **_Rendering Plant_** is the framework not the actual renderer... it handles
I/O and provides utility functions to a renderer. Throughout this document we
are describing the scene input language *not* the behavior of any particular
renderer. There are several sample rendering algorithms provided with the
**_Rendering Plant_** framework distribution... you should consult documentation on
those programs to see if a particular feature is supported or how it is implemented.

### List of commands:

Global Scene Commands
  * [background](#background)
  * [camera](#camera)
  * [clear](#clear)
  * [depthrange](#depthrange)
  * [fog](#fog)
  * [genericflags](#genericflags)
  * [light](#light)
  * [output](#output)
  * [sceneflags](#sceneflags)

Material Commands
  * [material](#material)
    * [ambient](#material)
    * [diffuse](#material)
    * [specular](#material)
    * [highlight](#material)
    * [reflection](#material)
    * [refraction](#material)
    * [shiny](#material)
    * [texname](#material)
  * [texture](#texture)

Matrix Commands
  * [identity](#identity)
  * [matrix](#matrix)
  * [orthographic](#orthographic)
  * [perspective](#perspective)
  * [pop](#pop)
  * [rotate](#rotate)
  * [scale](#scale)
  * [translate](#translate)

Object Commands
  * [loadobj](#loadobj)
  * [objflags](#objflags)
  * [sphere](#sphere)
  * [trilist](#trilist)
  * [vertex](#vertex)

___

### background

Load an image into the frame buffer as a background

#### Specification

        background(filename);

#### Parameters

        filename        name of the image file to load

#### Description

Loads a 24 bit Windows BMP image format file into the frame buffer, beginning
at x,y location (0,0).

#### Notes

The image should be the same size as the frame buffer for full coverage, however
this is not enforced (it's possible you might want to load a sky image as wide
as the frame buffer, but only in the upper half of the frame buffer).

It might be a good idea to add offset parameters...

___


### camera

Set the camera location and parameters in world space

#### Specification

        camera(x, y, z, coix, coiy, coiz, upx, upy, upz, aspect, fov);

#### Parameters

        x, y, z                 the position of the camera in world space
        coix, coiy, coiz        the point in world space the camera is looking at
        upx, upy, upz           the up vector of the camera
        aspect                  the x/y aspect ratio of the image
        fov                     the field of view in angles

#### Description 

Creates a camera object in worlds space, effectively setting the `MTX_VIEW` matrix
for the scene.

#### Notes

Loads the `MTX_VIEW` matrix. You cannot provide a different matrix type.

The field of view is actually fov/2  (should fix this for a clearer interface)


___


### clear

Clear the frame buffer to a specified color

#### Specification

        clear(red, green, blue, alpha);

#### Parameters

        red, green, blue, alpha	- floating point color values in the range of 0.0-1.0

#### Description

Clears the frame buffer to the color specified.

#### Notes

The default frame buffer color is black:

```
	clear(0,0,0,0);
``` 

___


### depthrange

Set the hither and yon clipping planes for the camera.

#### Specification

        depthrange(near, far);

#### Parameters

        near     the near (or hither) clipping plane (floating point, distance
                 from the camera.

        far      the far (or yon) clipping plane (floating point, distance
                 from the camera.

#### Description

The near and far clipping planes (sometimes called "hither" and "yon") are used
for algorithms with polygon clipping as the near and far clipping planes. 

Near and far are also used by algorithms which use a depth buffer (Z buffer) to scale
the screen-space Z which is stored in the depth buffer. **_Rendering Plant_** provides
only a floating point depth buffer, so scaling the depth range is not as critical
as it might be on GPU hardware with fixed-point mathematics.

#### Notes

See documentation for each renderer for information how this command is applied to
the rendering pipeline.


___

### fog

Specify linear fog parameters for the scene

#### Specification

        fog(start, end, r, g, b, a);

#### Parameters

        start           positive floating point, distance from the eye
        end             positive floating point, distance from the eye
                        (should be larger than start)
        r, g, b	    floating point color (R,G,B), range 0.0-1.0

#### Description

Specifies a linear fog ramp, from a near point to a far point. Pixels with Z values
Beyond the end of the fog will resolve to the full fog color.

#### Notes

This specifies the fog parameters but it must be enabled with a call to

```
	sceneflags(FOG);
```


___

### genericflags

Set or clear generic scene state flags
                                
#### Specfication
	
        genericflags( flaglist );

#### Parameters

A flaglist is one or more render state flags.

        Supported flags are:

            RENDER01       first generic flag
            RENDER02       second generic flag
            RENDER03       third generic flag
            RENDER04       fourth generic flag
            RENDER05       fifth generic flag
            RENDER06       sixth generic flag
            RENDER07       seventh generic flag
            RENDER08       eighth generic flag

#### Description

Enable/disable generic render state flags for the scene.

A flag may be negated with the `~` or `!` operators. This is useful
to "clear" a flag in the render state.

The default state for all flags is off.

These flags are passed through the input stream and the **_Rendering Plant_** 
system, their status maintained in `RPScene.generic_flags`. Their purpose
is to provide some renderer-defined behaviors via the input stream.

If a renderer defines any of these flags, they should use the corresponding
generic flag definition in `rp_defines.h` to test `RPScene.generic_flags` during
execution.

For example:

`RENDER01` is interpreted by both `scan` and `paint` as "No Clipping". Setting
this flag disables clipping in those renderers. This was added as a debugging
feature, but could also be useful for acceleration (for example, in video games
it is common to use a higher-order object visibiity method so that clipping
is not necessary).

`RENDER02` is interpreted by both `scan` and `paint` as "Outline Triangles".
Setting this flag causes all triangles drawn to be outlined with a red line.
This was added as a debugging feature.

#### Notes

These are "scene-wide" flags, affecting all objects in the scene. It might be
useful to have a similar feature for object flags (not implemented).

See the source code and the example scene `clip.in` for further information.

___

### light

Specify a local light in world coordinates

#### Specification

        light(lx, ly, lz, r, g, b);

#### Parameters

        lx, ly, lz      position of this light in world space. Floating point values.
        r, g, b	    color of this light. Floating point values, range 0-1.0.

#### Description

Creates a local light and adds it to the scene.

#### Notes

This API currently only supports a simple local light (lights in all direction, no
attenuation, etc.). Spot lights and other light sources should be added TBD.

___

### output

Set the output filename and image resolution

#### Specification

        output(filename, xres, yres);

#### Parameters

        filename        name of the BMP image file to be created
        xres            horizontal resolution of the frame buffer
        yres            vertical resolution of the frame buffer

#### Description

Sets the resolution and filename for the output image.

The output file will be a 24 bit BMP format image file.

#### Notes

The maximum horizontal resolution allowed is 3840.

The maximum vertical resolution allowed is 2160.


___

### sceneflags

Set or clear scene state flags
                                
#### Specfication
	
        sceneflags( flaglist );

#### Parameters

A flaglist is one or more render state flags.

        Supported flags are:

            ALLFLAGS       sets (or clears) all flags
            FOG            enable fog during shading
            MULTISAMPLE    enable pixel multisampling during render
            NOSHADOW       disable shadow generation
            PERSPTEXTURE   use perspective corrected (screen space) texture coordinates
            VERBOSE        print diagnostic information while running
            VERBOSE2       print even more diagnostic information while running
            ZBUFFER        enable depth buffer 

#### Description

Enable/disable render state flags for the scene.

A flag may be negated with the `~` or `!` operators. This is useful
to "clear" a flag in the render state.

The default state for all flags is off.

#### Notes

Some flags (such as `FOG`) require additional calls for their complete
effect to be realized.

`NOSHADOW` is provided for debugging and faster rendering.

API Note: a flaglist is just a list of flags... there are no logical operators
permitted between them; only unary `~` to negate a flag. Order does not matter.


___

### material

Set a material parameter

#### Specification

        material(color, r, g, b, a);
        material(ambient, r, g, b, a);
        material(diffuse, r, g, b, a);
        material(specular, r, g, b, a);
        material(shiny, float);
        material(reflection, float);
        material(refraction, float);
        material(texname, string);

#### Parameters

        color           set the material color to r,g,b,a (floating point, 0.0-1.0)
        ambient         set the ambient term to r,g,b,a (floating point, 0-1.0)
        diffuse         set the diffuse term to r,g,b,a (floating point, 0-1.0)
        specular        set the specular term to r,g,b,a (floating point, 0-1.0)
        highlight       set the highlight color to r,g,b,a (floating point, 0-1.0)
        shiny           set Blinn-Phong exponent to a floating point value
        reflection      set the reflection index to a floating point value
        refraction      set the refraction index to a floating point value
        texname         use the texture "string" (filename of a loaded texture)

#### Description

This command sets the parameters of the current material. The first argument is
the paramater to set, followed by the appropriate values.

Color is floating point, range 0.0-1.0

Ambient, diffuse, and specular are floating point, range 0 - 1.0 (usually)

Highlight is the color of the specular highlight.

Shiny is the Blinn-Phong exponent term for specular reflection.

Reflection value is a floating point, 0.0 - 1.0, ranging from "not reflective" to
"100% reflective".

Refraction is the refractive index of the material. See
[Wikipedia list of refractive indices](https://en.wikipedia.org/wiki/List_of_refractive_indices)

Texname is a name that references a loaded texture. This is the same as the filename
used to load that texture.

#### Notes



___

### texture

Load a texture image from a file

#### Specification

        texture( [texnum] filename, [textureflags], sx, sy, soff, toff);

#### Parameters

        texnum                  optional, slot to load this texture
        filename                filename of the texture image file
        textureflags            optional texture flags
        sx, sy                  horizontal and vertical scale factors for the texture
        soff, toff              horizontal and vertical offsets for the texture

The optional textureflags can be one or more of:

        CLAMP                   repeat the last pixel if u/v goes over 1.0
        FILT                    filter sample the texture
        WRAP                    "wrap" the texture if u/v goes over 1.0
        MIRROR                  "mirror" the texture if u/v goes over 1.0
        MODULATE                multiply the texture color by the shaded pixel color

#### Description

This command loads an image file to use as a texture.

An optional `texnum` is the slot to load the texture (0-31). If no `texnum` is provided,
the next available texture slot is used.

The texture image file should be a 24 bit BMP format image.

The behavior of the `textureflags`, scaling, and offset is similar to OpenGL.

#### Notes

This command may be abbreviated:

```
	tex("checker.bmp", WRAP, 1.0, 1.0, 0.0, 0.0);
```
The program has a maximum of 32 textures.

Only one texture per object.


___

### identity

Create an identity matrix

#### Specification

	identity(mtxtype [mtxflags]);

#### Parameters

Matrix type can be one of:

        MTX_PROJECTION
        MTX_VIEW
        MTX_MODEL

Optional matrix flaglist can be one or more of:

        MTX_LOAD        load this matrix (replacing top of stack)
        MTX_PUSH        push current top of stack before loading
        MTX_MULT        multiply this matrix by the top of stack

#### Description

Creates an identity matrix on the specified matrix stack.

#### Notes

If no matrix flags are supplied, the default behavior is `MTX_LOAD`


___

### matrix

Load a 4x4 matrix

#### Specfication
	
        matrix( type, [flaglist,]
                0.0, 0.0, 0.0, 0.0,
                0.0, 0.0, 0.0, 0.0,
                0.0, 0.0, 0.0, 0.0,
                0.0, 0.0, 0.0, 0.0);

#### Parameters

Matrix type can be one of:

        MTX_PROJECTION
        MTX_VIEW
        MTX_MODEL

Optional matrix flaglist can be one or more of:

        MTX_LOAD        load this matrix (replacing top of stack)
        MTX_PUSH        push current top of stack before loading
        MTX_MULT        multiply this matrix by the top of stack

Followed by 16 floating point values in row major order that describe the matrix:

		m[0][0], m[0][1], m[0][2], m[0][3],
		m[1][0], m[1][1], m[j][2], m[1][3],
		m[2][0], m[2][1], m[2][2], m[2][3],
		m[3][0], m[3][1], m[3][2], m[3][3]

#### Description

Loads a 4x4 (row major) matrix onto the respective matrix stack.

This is most useful if you are creating matrices outside of this program (such
as driving the renderer from an animation or scene assembly program).

#### Notes

___

### orthographic

Create an orthorgraphic projection matrix.

#### Specification

        orthographic( left, right, bottom, top, near, far );

#### Parameters

        left                    left edge of projection volume
        right                   right edge of projection volume
        bottom                  bottom edge of projection volume
        top                     top edge of projection volume
        near                    near edge of projection volume
        far                     far edge of projection volume

#### Description

Creates an OpenGL style projection matrix and loads it as `MTX_PROJECTION`. Input 
parameters are in screen space, with 0,0 centered in the middle of the image.

#### Notes


___

### perspective

Create a perspective projection matrix.

#### Specification

        perspective( fovy, aspect, near, far );

#### Parameters

        fovy                    field of view in y direction
        aspect                  aspect ratio (x/y) of image plane
        near                    near edge of projection volume
        far                     far edge of projection volume

#### Description

Creates an OpenGL style projection matrix and loads it as `MTX_PROJECTION`. 

#### Notes

This command is not required. The parameters are also set by `camera()` and `depthrange()`
If this command is prefered, it must come after `camera()`.


___

### pop

Pop a matrix off of a matrix stack

#### Specification

        pop(type);

#### Parameters

Matrix type can be one of:

        MTX_PROJECTION
        MTX_VIEW
        MTX_MODEL

#### Description
	
Pop the specified matrix stack.

#### Notes


___

### rotate

Create a rotation matrix

#### Specification

        rotate([mtxtype mtxflags,] a, x, y, z);

#### Parameters

Optional matrix type can be one of:

        MTX_PROJECTION
        MTX_VIEW
        MTX_MODEL

Optional matrix flaglist can be one or more of:

		MTX_LOAD	- load this matrix (replacing top of stack)
		MTX_PUSH	- push current top of stack before loading
		MTX_MULT	- multiply this matrix by the top of stack
		a		- angle to rotate, in degrees
		x, y, z		- floating point, vector x,y,z about which to rotate

#### Description

Creates an OpenGL style rotation matrix of angle a, about vector x,y,z:

x,y,z can be an arbitrary axis vector. Construction of an arbitrary rotation
matrix is beyond the scope of this document (see the code) but for our orthogonal
axis vectors:

Rotation around X-axis (1, 0, 0):

        m[0] = [        1        0        0       0  ]
        m[1] = [        0   cos(a)  -sin(a)       0  ]
        m[2] = [        0   sin(a)   cos(a)       0  ]
        m[3] = [        0        0        0       1  ]

Rotation around Y-axis (0, 1, 0):

        m[0] = [   cos(a)        0   sin(a)       0  ]
        m[1] = [        0        1        0       0  ]
        m[2] = [  -sin(a)        0   cos(a)       0  ]
        m[3] = [        0        0        0       1  ]

Rotation around Z-axis (0, 0, 1):

        m[0] = [   cos(a)  -sin(a)        0       0  ]
        m[1] = [   sin(a)   cos(a)        0       0  ]
        m[2] = [        0        0        1       0  ]
        m[3] = [        0        0        0       1  ]

If no mtxtype is specified, it assumes `MTX_MODEL`

If no mtxflags are specified, it assumes `MTX_MULT`
 
#### Notes

3D rotation is highly order dependent. If you find yourself doing anything other than
`identity()` followed by one `rotate()` command, you should really understand how
this works. Google Euler's rotaton theorem.


___

### scale

Create a scaling matrix

#### Specification

        scale([mtxtype mtxflags,] sx, sy, sz);

#### Parameters

Optional matrix type can be one of:

        MTX_PROJECTION
        MTX_VIEW
        MTX_MODEL

Optional matrix flaglist can be one or more of:

        MTX_LOAD        load this matrix (replacing top of stack)
        MTX_PUSH        push current top of stack before loading
        MTX_MULT        multiply this matrix by the top of stack
        sx, sy, sz      floating point, scale values for x,y,z

#### Description

Creates an OpenGL style scaling matrix:

        m[0] = [       sx        0        0       0  ]
        m[1] = [        0       sy        0       0  ]
        m[2] = [        0        0       sz       0  ]
        m[3] = [        0        0        0       1  ]

If no mtxtype is specified, it assumes `MTX_MODEL`

If no mtxflags are specified, it assumes `MTX_MULT`
 

___

### translate

Create a translation matrix

#### Specification

        translate([mtxtype mtxflags,] tx, ty, tz);

#### Parameters

Optional matrix type can be one of:

        MTX_PROJECTION
        MTX_VIEW
        MTX_MODEL

Optional matrix flaglist can be one or more of:

        MTX_LOAD        load this matrix (replacing top of stack)
        MTX_PUSH        push current top of stack before loading
        MTX_MULT        multiply this matrix by the top of stack

Translate values:

        tx, ty, tz      floating point translation values for x,y,z

#### Description

Creates an OpenGL style translation matrix:

        m[0] = [        1        0        0       0  ]
        m[1] = [        0        1        0       0  ]
        m[2] = [        0        0        1       0  ]
        m[3] = [       tx       ty       tz       1  ]

If no mtxtype is specified, it assumes `MTX_MODEL`

If no mtxflags are specified, it assumes `MTX_MULT`

#### Notes

___

### loadobj

Load a Wavefront .obj geometry from a file.

#### Specfication

        loadobj( filename );

#### Parameters

        filename        a quoted text string, the file to load

#### Description

Loads a Wavefront `.obj` file as polygonal geometry. If the polygons have
more than 3 vertices, it will convert to triangles.

#### Notes

Only a subset of the full OBJ spec is supported. Material-related commands are
consumed but ignored.

There is no support for paramentric curves or surfaces other than polygonal geometry.

___

### objflags

Set or clear object state flags

#### Specfication

        objflags( flaglist );

#### Parameters

A flaglist is one or more render state flags.

        Supported flags are:

            ALLFLAGS       sets (or clears) all flags
            BUMP           use bump mapping 
            CULL_BACK      do not render back-facing polygons
            CULL_FRON      do not render front-facing polygons
            FLATSHADE      use one normal per polygon
            LIGHTING       enable lighting calculations
            POLYSHADE      use the polygon during shading
            RANDSHADE      randomly color each vertex of a polygon
            REFLECT        use reflection mapping 
            SMOOTHSHADE    interpolate the normal across a polygon during shading
            TEXGENSPHERE   enable texture mapping during shading.
            TEXTURE        enable texture mapping during shading.
            VERTNORM       generate vertex normals
            VERTSHADE      use the polygon's vertex colors during shading
            VERTEXNORMALS  interpret the vertex colors as normals on input

#### Description

Enable/disable render state flags for the object.

A flag may be negated with the `~` operator. This is useful
to "clear" a flag in the render state.

Render state is persistent until changed. When an object is "bound"
a snaphot of the current render state is copied to that object and
used for it's rendering. While this can be useful to explot this
behavior, it is recommended that each object fully specify it's desired
render state to avoid unintended consequences.

___

### sphere

Define a sphere geometry object

#### Specification

        sphere(cx, cy, cz, radius);

#### Parameters

        cx, cy, cz        floating point, the x,y,z center in MODEL space
        radius            floating point, the radius of the sphere

#### Description

Creates a sphere and adds it to the scene. The center x,y,z and radius are in MODEL 
space.

Upon processing this command, the current MODEL and VIEW matrices, as well as
the current material and render state are bound to this object.
 
If a renderer does not support a implicit definition of a sphere, a polygonal
sphere is automatically generated to satisfy this command.

A renderer tells **_Rendering Plant_** that native spheres are not supported by
making a call to 'RPEnableSphereSupport(FALSE);' in the renderer source code
before the input scene is parsed. 

#### Notes

This command may be abbreviated:

```
        sp(0.0, 0.0, 0.0, 100.0);
```


___

### trilist

Define a triangle list for a polygonal object

#### Specification

        trilist [numtri] {
            {v0, v1, v2},
                ...
            {v0, v1, v2},
        };

#### Parameters

        numtri        integer, the number of triangles in the list
        v0            integer, the index of the 1st vertex of this
                      triangle into the current vertex list
        v1            integer, the index of the 2nd vertex of this
                      triangle into the current vertex list
        v2            integer, the index of the 3rd vertex of this
                      triangle into the current vertex list

#### Description


Creates a polygonal object and adds it to the scene. The object is defined
by a list of triangles (which index into the current vertex list). The first
index of the vertex list is 0.

Upon processing this command, the current MODEL and VIEW matrices, as well as
the current material and render state are bound to this object.
 
#### Notes

The comma at the end of the last triangle in the list is optional (similar to C structure
initialization).


___

### vertex

Define a vertex list for a polygonal object

#### Specification

        vertex [numvert] {
            {x, y, z, r, g, b, a, s, t},
                ...
            {x, y, z, r, g, b, a, s, t},
        };

#### Parameters

        numvert	      integer, the number of vertices in the list
        x, y, z	      floating point, the x,y,z in MODEL space of the vertex
        r, g, b, a    floating point, the r,g,b,a color of the vertex (0.0-1.0)
        s, t          floating point, the s,t texture coordinates of the vertex

#### Description

This command specifies a vertex list (which will be later indexed by a triangle).

If the `VERTEXNORMALS` render flag is set, the r,g,b is interpreted as the x,y,z of
the normal for that vertex.

#### Notes

This command may be abbreviated:

```
	vtx[3] {
            { 1.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0 },
            { 0.0, 1.0, 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0 },
            { 0.0, 0.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0 },
        };
```

The comma at the end of the last vertex in the list is optional (similar to C structure
initialization).

A homogenous w coordinate of 1.0 is assumed and set for you prior to transformations.


