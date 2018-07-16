/*
 * File: 	ray.c
 *
 * The top of the "moray" ray trace algorithm
 *
 */

/*
 * 
 * MIT License
 * 
 * Copyright (c) 2018 Steve Anderson
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "rp.h"
#include "ray.h"

/* count up various rays traced for info purposes */
RayStats_t	RayStats;

static void	init_ray_stats(void);

/*
 * raytrace the entire scene.
 *
 * The "root" of the ray graph.
 */
void
raytrace_scene(void)
{
    Colorf_t	thiscolor;
    rgba_t	*color = (rgba_t *) NULL;
    Ray_t	*eyeray;
    float	tanfov, progress = 0.0, wt = 0.5;
    int 	x, y, i, j, sample_count;

    init_ray_stats();
    RPLoadBackgroundImage();

    if (Flagged(RPScene.flags, FLAG_SCENE_MULTISAMPLE)) {
	RPScene.num_samples = (RPScene.num_samples > 5) ? (5) : Max(RPScene.num_samples, 1);
	wt = 1.0/(RPScene.num_samples+1);
    }

    fprintf(stderr,"Raytracing Scene:\n");
    fprintf(stderr,"\tResolution %d x %d\n",RPScene.xres,RPScene.yres);
    if (Flagged(RPScene.flags, FLAG_SCENE_MULTISAMPLE)) 
	fprintf(stderr,"\tMultisampling primary rays: using %d rays per pixel\n",
		Sqr(RPScene.num_samples));
    fprintf(stderr,"\t[%d] objects...\n",RPScene.obj_count);
    fprintf(stderr,"\t[%d] lights...\n",RPScene.light_count);

	/* tranform objects to camera space */
    RPProcessObjects();

	/* transform lights to camera space */
    RPTransformLights();

	/* fov is actually fov/2.0 */
    tanfov = tanf(RPScene.camera->fovr/2.0);

    fprintf(stderr,"Progress:  %5.2f %%",progress*100.0);

	/* re-use this ray for all primary rays, but must reset within inner loop */
    eyeray = NewRay(PRIMARY_RAY, -1);

    	/* send primary ray from eye to the pixel on the sreen (down -z axis) */
    for (y=0; y<RPScene.yres; y++) {
	for (x=0; x<RPScene.xres; x++) {

	    sample_count = 0;
	    thiscolor.r = thiscolor.g = thiscolor.b = thiscolor.a = 0.0;
	    for (i=0; i<RPScene.num_samples; i++) {
	        for (j=0; j<RPScene.num_samples; j++) {

		    /* clear/reset primary ray: */
	            eyeray->depth = 0;
	            eyeray->t = MAX_RAY_T;
	            if (eyeray->surf != (TriShade_t *)NULL)
		        free(eyeray->surf);
	            eyeray->surf = (TriShade_t *) NULL;

                    eyeray->orig.x = RPScene.camera->eye.x;	/* orig is (0,0,0) */
                    eyeray->orig.y = RPScene.camera->eye.y; 
                    eyeray->orig.z = RPScene.camera->eye.z; 

		        /* compute fb(y,x) to u,v params spanning camera plane: */
	            eyeray->dir.x = ((2.0 * (x + (i*wt))) / (float)RPScene.xres - 1.0) * 
			            (RPScene.camera->aspect * tanfov);
	            eyeray->dir.y = (1.0 - 2 * (y + (j*wt)) / (float)RPScene.yres) * tanfov;
	            eyeray->dir.z = RPScene.camera->dir.z; 
	            vector_normalize(&(eyeray->dir));

  	            color = trace_ray(eyeray);

	            if (color != NULL) {
		        if (eyeray->t == MAX_RAY_T && 
			    Flagged(RPScene.flags, FLAG_BACKGROUND_IMAGE)) {
		             /* miss, but background image was loaded */
		             thiscolor.r += (float) RPColorFrameBuffer[y][x].r;
		             thiscolor.g += (float) RPColorFrameBuffer[y][x].g;
		             thiscolor.b += (float) RPColorFrameBuffer[y][x].b;
		             thiscolor.a += (float) RPColorFrameBuffer[y][x].a;
		         } else {
		             thiscolor.r += (float) color->r;
		             thiscolor.g += (float) color->g;
		             thiscolor.b += (float) color->b;
		             thiscolor.a += (float) color->a;
		         }

		         sample_count++;
	                 free(color); /* free color allocated in trace_ray() */
	                 color = (rgba_t *) NULL;
 	             } else {
		         /* ray depth exceeded, no color returned */
	             }
	             RayStats.primary_ray_count++;
	        }
	    }
            RPColorFrameBuffer[y][x].r = (int) (thiscolor.r/sample_count);
            RPColorFrameBuffer[y][x].g = (int) (thiscolor.g/sample_count);
            RPColorFrameBuffer[y][x].b = (int) (thiscolor.b/sample_count);
            RPColorFrameBuffer[y][x].a = (int) (thiscolor.a/sample_count);
	}
	progress = (float)y/(float)RPScene.yres;
        fprintf(stderr,"\b\b\b\b\b\b\b%5.2f %%",progress*100.0);
    }

    FreeRay(eyeray);

    RPCleanupObjects();
    RPCleanupTextures();

    fprintf(stderr,"\b\b\b\b\b\b\b\b100 %% ... done!\n");
    fprintf(stderr,"\n%s : Rendering Summary:\n",program_name);
    fprintf(stderr,"------------------------------------------------------\n");
    fprintf(stderr,"%s : [%'16d]\tinput polygons\n",
            program_name, RPScene.input_polys);
    fprintf(stderr,"%s : [%'16d]\tintersections avoided with culled polygons\n",
            program_name, RayStats.culled_polys);

    fprintf(stderr,"%s : [%'16d]\tprimary rays cast (%'d hits)\n",
            program_name, RayStats.primary_ray_count, RayStats.primary_ray_hit_count);
    fprintf(stderr,"%s : [%'16d]\treflection rays cast (%'d hits)\n",
            program_name, RayStats.reflection_ray_count, RayStats.reflection_ray_hit_count);
    fprintf(stderr,"%s : [%'16d]\trefraction rays cast (%'d hits)\n",
            program_name, RayStats.refraction_ray_count, RayStats.refraction_ray_hit_count);
    fprintf(stderr,"%s : [%'16d]\tshadow rays cast (%'d hits)\n", 
            program_name, RayStats.shadow_ray_count, RayStats.shadow_ray_hit_count);
    fprintf(stderr,"\n");
}

/* 
 * trace a ray 
 *
 * this function is called from the root, generating a pixel value for a screen
 * pixel; it is also called recurisivley from shade_pixel() for reflect and 
 * refract rays.
 *
 * (shadow rays are implemented with a simpler function, see below)
 *
 * This function allocates and returns a rgba_t structure... which the caller
 * is responsible for freeing.
 *
 */
rgba_t *
trace_ray(Ray_t *ray)
{
    Object_t	*op;
    Material_t	*m;
    rgba_t	*color = (rgba_t *) NULL;
    xyz_t    	surf, view, normal;
    float	t = MAX_RAY_T;
    int		i, found = FALSE;
    
    ray->depth++;

    if (ray->depth > MAX_RAY_DEPTH) {
	return(NULL);
    }

    color = (rgba_t *) malloc(sizeof(rgba_t));

	/* handle fog in the background */
    if (Flagged(RPScene.flags, FLAG_FOG)) {
        color->r = (u8) Clamp0255(RPScene.fog_color.r * 255.0);
        color->g = (u8) Clamp0255(RPScene.fog_color.g * 255.0);
        color->b = (u8) Clamp0255(RPScene.fog_color.b * 255.0);
        color->a = (u8) Clamp0255(RPScene.fog_color.a * 255.0);
    } else {
        color->r = RPScene.background_color.r;
        color->g = RPScene.background_color.g;
        color->b = RPScene.background_color.b;
        color->a = RPScene.background_color.a;
    }
    
	/* intersect ray with all of the objects */

    for (i=0; i<RPScene.obj_count; i++) {

	op = RPScene.obj_list[i];
    
	found = object_intersect(ray, op, &t, &surf, &normal);

	/* if hit and it's closest so far, calc shade */
	if (found && t < ray->t) {

	    ray->t = t;

	    m = op->material;

		/* view vector is -ray.dir */
	    vector_scale(&view, &(ray->dir), -1.0f);

	    if (op->type == OBJ_TYPE_SPHERE) {

	        shade_sphere_pixel(color, m, ray, &normal, &surf, &view, op);

            } else if (op->type == OBJ_TYPE_POLY) {

	        shade_tri_pixel(color, m, ray, &normal, &surf, &view, op);

		if (ray->surf != (TriShade_t *) NULL) {
		    free(ray->surf);
		    ray->surf = (TriShade_t *) NULL;
		}

            } else {
		/* can't happen */
		fprintf(stderr,"%s ERROR : unknown object type %d (%s, %d)\n",
			program_name, op->type, __FILE__,__LINE__);
	    }

    	    if (ray->type == PRIMARY_RAY)
	        RayStats.primary_ray_hit_count++;
    	    else if (ray->type == REFLECTION_RAY)
	        RayStats.reflection_ray_hit_count++;
    	    else if (ray->type == REFRACTION_RAY)
	        RayStats.refraction_ray_hit_count++;
	    else 
		fprintf(stderr,"%s ERROR : unknown ray type %d (%s, %d)\n",
			program_name, ray->type, __FILE__,__LINE__);
        }
    }

    return (color); 
}


/*
 * optimized one level ray trace, looks for objects blocking this light
 * simple version doesn't handle partial shadows from transparent objects
 */
int
trace_shadow_ray(int id, xyz_t *origin, Light_t *light)
{
    Object_t    *op;
    Ray_t       *shadow;
    xyz_t       surf, normal;
    float       t;
    int         i, found = FALSE;


    shadow = NewRay(SHADOW_RAY, id);
    RayStats.shadow_ray_count++;

    shadow->orig.x = origin->x;
    shadow->orig.y = origin->y;
    shadow->orig.z = origin->z;

    vector_sub(&(shadow->dir), &(light->pos), origin);
    vector_normalize(&(shadow->dir));

    for (i=0; i<RPScene.obj_count && !found; i++) {
        op = RPScene.obj_list[i];
        found = object_intersect(shadow, op, &t, &surf, &normal);
    }

    FreeRay(shadow);

    if (found)
        RayStats.shadow_ray_hit_count++;

    return (found);
}


static void
init_ray_stats(void)
{
    RayStats.culled_polys             = 0;
    RayStats.primary_ray_count        = 0;
    RayStats.primary_ray_hit_count    = 0;
    RayStats.reflection_ray_count     = 0;
    RayStats.reflection_ray_hit_count = 0;
    RayStats.refraction_ray_count     = 0;
    RayStats.refraction_ray_hit_count = 0;
    RayStats.shadow_ray_count         = 0;
    RayStats.shadow_ray_hit_count     = 0;
}

/* was used for debugging */
void
DumpRay(Ray_t *ray)
{

    if (ray->type == PRIMARY_RAY)
	fprintf(stderr,"Ray : PRIMARY ");
    else if (ray->type == SHADOW_RAY)
	fprintf(stderr,"Ray : SHADOW ");
    else if (ray->type == REFLECTION_RAY)
	fprintf(stderr,"Ray : REFLECTION ");
    else if (ray->type == REFRACTION_RAY)
	fprintf(stderr,"Ray : REFRACTION ");

    fprintf(stderr," orig id = %d depth = %d t = %f\n",
		ray->origid, ray->depth, ray->t);
    fprintf(stderr,"\torig = (%10.3f ,%10.3f, %10.3f)\n",
		ray->orig.x, ray->orig.y, ray->orig.z);
    fprintf(stderr,"\tdir  = (%10.3f ,%10.3f, %10.3f)\n",
		ray->dir.x, ray->dir.y, ray->dir.z);
}


/* allocates a new ray, initializes it a little bit and returns it */
Ray_t *
NewRay(int type, int origid)
{
    Ray_t	*ray;

    ray = (Ray_t *) calloc(1, sizeof(Ray_t));

    ray->type = type;
    ray->origid = origid;
    ray->depth = 0;
    ray->t = MAX_RAY_T;
    ray->surf = (TriShade_t *) NULL;

    return(ray);
}

void
FreeRay(Ray_t *ray)
{
    if (ray == (Ray_t *) NULL) {
	fprintf(stderr,"ERROR : %s : %d : attempt to free empty ray.\n",__FILE__,__LINE__);
        return;
    }

    if (ray->surf != (TriShade_t *) NULL) {
	free (ray->surf);
    }
    free (ray);
}

