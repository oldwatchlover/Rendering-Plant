
/*
 * File:        hidden.h
 *
 * This include file holds all of the define/typedef/externs for the 
 * hidden line renderer.
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

#ifndef __HIDDEN_H__
#define __HIDDEN_H__

	/* defines: */

#define FLAG_EDGE_CULL_BACK	FLAG_CULL_BACK		/* 0x001 */
#define FLAG_EDGE_CULL_FRONT	FLAG_CULL_FRONT		/* 0x002 */
#define FLAG_EDGE_SILHOUETTE	0x0004
#define FLAG_EDGE_CREASE	0x0008

#define MAX_ZVAL	REALLY_BIG_FLOAT

	/* data types: */

	/* an edge structure */
typedef struct {

    u32		flags;
    int		tri0, tri1;
    int		v0, v1;  

} Edge_t;


	/* holds list of edges for an object */
typedef struct {

    Edge_t	*edges;
    int		num_edges;

} ObjEdges_t;


	/* extern variables and functions: */
extern int	total_edges, drawn_edges;

/* from hidden.c */
extern void	draw_scene(void);

/* from edges.c */
extern void		create_obj_edges(Object_t *op);
extern void		process_obj_edges(Object_t *op);
extern void		draw_edges(Object_t *op);

#endif
/* __HIDDEN_H__ */

