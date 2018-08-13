
# _paint_ - A Painter's Algorithm

This renderer is a "painter's algorithm", originally develped as a
hardware GPU algorithm simulator<sup>[1](#algsim)</sup>.

### ALGORITHM

Per-poly rasterization with clipping and z-buffer. The "painter's" algorithm is one of the
simplest visual surface algorithms (which is why it is favored for hardware implementations).
The renderer simply loops through every polygon for every object in the scene and draws each to
the frame buffer. A depth buffer (z-buffer) is used to determine which surface is closest to the eye
and should be displayed for each pixel.

This renderer rasterizes in screen space... therefore it forces `RPSetSceneFlags(FLAG_PERSP_TEXTURE)`
for proper calculation of perspectively-corrected texture coordinates.

This renderer implements and uses two of the generic scene state flags:

    RENDER01        disable clipping (useful for debugging or performance optimization)
    RENDER02        outline all triangles with a red border (useful for debugging)

### IMPLEMENTATION LIMITATIONS

    - transparency. You would need to sort all the geometry back-to-front
      and paint in that order for z-buffer transparency to work.

    - fancy shading. reflection, refractions, etc. It supports basic
      "OpenGL style fixed function" shading.

    - non-polygonal geometry (however, if linked to librp.a, spheres can be
      approximated with polygonal data automatically for you).  

### IDEAS FOR FUTURE WORK

Some ideas for improvements:

    - MIP mapped textures can be implemented trivially becuase the hardest part of 
      a MIP map algorithm is choosing the right MIP level... with a screen-space rasterizing
      method, we get this for free: the rasterization deltas give us the ratio of texels to 
      pixels (one reason why MIP mapping can be implemented in hardware GPU's so easily).
      There is some code for this but it is not completed or connected.

    - Environment mapping. Partially implemented, but disabled.

    - Richer shader function.

    - Anti-aliasing.


----
<a name="algsim"><sup>1</sup></a>
What do you mean _"algorithm simulator"_?

For GPU development, usually these steps are followed:

    1) algorithm simulation (this code). High level (C in this case) implementation of
       the graphics pipeline and any "special algorithms" planned for hardware 
       implementation.

    2) chip simulator. Clock-cycle accurate and bus/output bit accurate simulator that 
       behaves exactly as the chip would. This is used for hardware verification and 
       test case development, as well as early driver work against a working hardware 
       model.

    3) GPU ASIC design. Coding the gates and circuits that can then be transferred to
       a semiconductor foundry for making the actual chips.

Why? Mainly because fabricating the first chips at the semiconductor foundry costs 
millions of dollars and takes 4-6 weeks (at a minimum) to get back your first 
working samples. If those samples don't work, you've wasted a lot of money and a 
lot of time. (Per Moore's law, the "lifetime" of a top performance semiconductor is 
about 18 months... so if you screw this up, you've literally burned a third of 
the competitive life cycle of your product (!))

In my career I've worked on GPU ASIC teams designing world-class GPU's, as well as with
other GPU teams doing OpenGL drivers and other low-level system code for the GPU.

The biggest lesson I've learned in all of these projects I've taken with me to all
engineering and software development I've worked on. It's best phrased as a question:

	"If you are developing software, what would you differently if running the
	compiler cost a million dollars and you had to wait 4 weeks for your output?"

You'd simulate. And test. And simulate some more. And verify your designs. You'd do
anything you could so that when you got the results back from your "compiler" you would
have a perfect, working and shippable product.


