
/*
 * File:        scan.h
 *
 * This include file holds all of the scanline renderer specific stuff
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

#ifndef __SCAN_H__
#define __SCAN_H__

	/* defines: */

	/* data types: */

typedef struct { 
    int		x, y;
} xyi_t;

typedef struct ep {	/* edge-pair structure */

    int		id;
    xyi_t	min, max;
    Object_t	*op;
    int		polyid;
    struct ep  	*next;

} ep_t;


	/* extern variables/functions: */

/* from scan.c */
extern int	input_polys;
extern int	primary_ray_count;
extern int	ray_hit_count;

extern void     scan_scene(void);

/* from raycast.c */
extern void	raytracer_init(void);
extern void	cast_primary_ray(int x, int y, ep_t *eplist);

/* from edge.c */

extern ep_t	*new_ep(void);
extern void	free_eplist(ep_t **eplist);
extern void	insert_edgepair(ep_t **eplist, ep_t *ep);
extern void	process_edgepairs(int y, ep_t **eplist);
extern void	print_edgepairs(ep_t *eplist);

/* from geom.c */
extern void	scan_obj_process(void);

#endif
/* __SCAN_H__ */

