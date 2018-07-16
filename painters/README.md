

This renderer is a hardware GPU algorithm simulator(1)...

Per-poly rasterization with clipping and z-buffer.

Things it does NOT do:

    - transparency. You would need to sort all the geometry back-to-front
      and paint in that order for z-buffer transparency to work.

    - fancy shading. reflection, refractions, etc. It supports basic
      "OpenGL style fixed function" shading.

    - non-polygonal geometry (however, if linked to librp.a, spheres can be
      approximated with polygonal data automatically for you).


----
(1) What do you mean "algorithm simulator"?

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

Why? Mainly becuase fabricating the first chips at the semiconductor foundry costs 
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


