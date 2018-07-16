
/*
 * File:        paint.h
 *
 * This include file holds all of the define/typedef/externs for the 
 * GPU algorithm simuator renderer.
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

#ifndef __PAINT_H__
#define __PAINT_H__

	/* defines: */

#define MAX_ZVAL	REALLY_BIG_FLOAT

	/* data types: */

	/* extern variables and functions: */
extern int	culled_polys, drawn_polys;

/* from paint.c */
extern void	paint_scene(void);

/* from paintgeom.c */
extern void	paint_obj_process(void);

/* from clip.c */
extern u8       gen_clipcodes(xyz_t *v, float w);
extern int      clip_triangle(Object_t *op, Tri_t *tri);

/* from rasterize.c */
extern void     paint_tri(Object_t *op, Tri_t *tri);

/* from paintshade.c */
extern void	shade_pixel(Object_t *op, 
			xyz_t *N, xyz_t *point, xyz_t *view, Colorf_t *shade);


#endif
/* __PAINT_H__ */

