
/*
 * File:	rp_externs.h
 *
 * This file is a bit of old-school object oriented programming practice..
 * it holds all of the extern variables and functions exported from each
 * module to share with other modules.
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

#ifndef __RP_EXTERNS_H__
#define __RP_EXTERNS_H__

/* set by RenderPlantInit() in init.c. used to provide consistent error messages */
extern char		*program_name;

/* these scene data structures are available to the renderer.
 * (color_fb/zbuffer have access functions that are the preferred
 * interface, but the buffer is available for more complex operations)
 */
extern Scene_t		Scene;
extern rgba_t   	color_fb[MAX_YRES][MAX_XRES];
extern float	   	zbuffer[MAX_YRES][MAX_XRES];

/*
 * these transformation matrices are available to the renderer:
 * see matrix.c for more information.
 */
extern float    	m_mtx[4][4];	/* top of MODEL matrix stack */
extern float    	v_mtx[4][4];	/* top of VIEW matrix stack */
extern float    	p_mtx[4][4];	/* top of PROJECTION matrix stack */
extern float    	mv_mtx[4][4];	/* current M*V */
extern float    	mvp_mtx[4][4];	/* current M*V*P */
extern float    	l_mtx[4][4];	/* light transform */
                                        /* transpose of inverse of MV */


/**************** exported function calls: ****************************/

/* from init.c */
extern void		RenderPlantInit(char *progname, u32 flags);

/* from rand.c */
extern float    	my_random(void);

/* from state.c */
extern void     	output_set(char *fname, int txres, int tyres);
extern void     	set_background_color(rgba_t *color);
extern void     	clear_cfb(rgba_t *color);
extern rgba_t   	*get_cfb_pixel(int x, int y);
extern void     	put_cfb_pixel(int x, int y, int r, int g, int b, int a);
extern void		put_cfb_line(int x1, int y1, int x2, int y2, 
					rgba_t cololr, int useaa);
extern void     	load_background_image(void);
extern void     	set_background_image(char *filename);
extern int      	write_cfb(void);
extern void		clear_zb(float *zval);
extern int		zbuffer_test(int x, int y, float z);
extern void		put_z_pixel(int x, int y, float z);
extern void     	set_object_flags(u32 flags);
extern void     	clear_object_flags(u32 flags);
extern void     	set_camera(xyz_t pos, xyz_t coi, xyz_t up, 
					float fov, float aspect);
extern void		set_depth_range(float near, float far);
extern void		set_projection(float aspect, float near, float far);
extern void     	set_fog(float start, float end, 
					float r, float g, float b, float a);
extern void     	set_scissor(int tulx, int tuly, int tlrx, int tlry);
extern void     	set_viewport(float sx, float sy, float sz, 
					float tx, float ty, float tz);

/* from scene.c */
extern void		scene_init(void);
extern void     	set_scene_flags(u32 flags);
extern void     	clear_scene_flags(u32 flags);
extern void     	set_scene_generic_flags(u32 flags);
extern void     	clear_scene_generic_flags(u32 flags);

/* from material.c */
extern void		free_current_material(void);
extern Material_t     	*get_current_material(void);
extern void		set_material_color(Colorf_t color);
extern void		set_material_amb(Colorf_t color);
extern void		set_material_diff(Colorf_t color);
extern void		set_material_spec(Colorf_t color);
extern void		set_material_highlight(Colorf_t color);
extern void		set_material_shiny(float value);
extern void		set_material_refl(float value);
extern void		set_material_refr(float value);
extern void		set_material_texture(char *name);
extern void		DumpMaterial(Material_t *m);


/* from bmp_util.c */
extern int      	read_bmp(const char *filename, Texture_t *tex);
extern int      	write_bmp(const char *filename, int width, int height);


/* from matrix.c */
extern void     	cat_matrix(float mf[4][4], float nf[4][4], float res[4][4]);
extern void     	transform_xyz(float m[4][4], xyz_t *in, xyz_t *out, float *w);
extern void     	load_matrix(float mtx[4][4], u32 flags);
extern void     	pop_matrix(u32 flags);
extern void     	ident_mtx(float mf[4][4]);
extern void		perspective_mtx(float mf[4][4], float fovy, 
				float aspect, float near,float far);
extern void 		ortho_mtx(float mf[4][4], float l, float r, 
				float b, float t, float n, float f);
extern void     	lookat_mtx(float mf[4][4], 
				float xEye, float yEye, float zEye,
                           	float xAt,  float yAt,  float zAt,
                           	float xUp,  float yUp,  float zUp);
extern void     	scale_mtx(float mf[4][4], float x, float y, float z);
extern void     	rotate_mtx(float mf[4][4], float a, float x, float y, float z);
extern void     	translate_mtx(float mf[4][4], float x, float y, float z);
extern void     	transpose_mtx(float om[4][4], float tm[4][4]);
extern void     	invert_mtx(float om[4][4], float im[4][4]);
extern void     	vector_add(xyz_t *result, xyz_t *v1, xyz_t *v2);
extern void     	vector_sub(xyz_t *result, xyz_t *v1, xyz_t *v2);
extern void     	vector_scale(xyz_t *result, xyz_t *v, float s);
extern void     	vector_cross(xyz_t *result, xyz_t *v1, xyz_t *v2);
extern float    	vector_dot(xyz_t v1, xyz_t v2);
extern void     	make_normal(xyz_t *n, xyz_t *p1, xyz_t *p2, xyz_t *p3);
extern void     	vector_normalize(xyz_t *n);


/* from light.c */
extern void     	set_light(float lx, float ly, float lz, 
					float r, float g, float b);
extern void     	light_transform(void);


/* from objects.c */
extern Object_t 	*object_add(int type);
extern void		object_read(char *fname);
extern void		object_free(Object_t *op);
extern void		object_cleanup(void);
extern void     	object_process(void);
extern void		DumpObj(Object_t *op);


/* from sphere.c */
extern void		sphere_support(int val);
extern void     	sphere_add(xyz_t center, float radius);
extern void		sphere_process(Object_t *op, Sphere_t *sp);


/* from polygons.c */
extern void		init_input_polys(void);
extern void		free_input_polys(void);
extern void     	tri_add(Tri_t *tri);
extern void     	tri_close(int tcount);
extern void     	tri_process_one(Object_t *op, Tri_t *tri);
extern void     	tri_process(Object_t *op, int count, Tri_t *tris);


/* from vertex.c */
extern void		init_input_vertex(void);
extern void		free_input_vertex(void);
extern void     	vertex_add(Vtx_t *vtx, int i);
extern void     	vertex_close(int vcount);
extern void     	vertex_transform(Object_t *op, int count, Vtx_t *verts);
extern void     	vertex_project(int count, Vtx_t *verts);

/* from clip.c */
extern u8		gen_clipcodes(xyz_t *v, float w);
extern int		clip_triangle(Object_t *op, Tri_t *tri);


/* from texture.c */
extern int       	texture_load(int texnum, char *filename, u32 flags,
                             float sscale, float tscale,
                             float soff, float toff);
extern void	  	texture_free(int texnum);
extern void	  	texture_cleanup(void);
extern u32        	texture_flags(int texnum);
extern int	  	texture_find(char *name);
extern rgba_t     	texture_point_sample(int texnum, 
				float s, float t, float inv_w);
extern rgba_t     	texture_filt_sample(int texnum, 
				float s, float t, float inv_w,
				float DxDs, float DyDs, float DxDt, float DyDt,
				float DxDw, float DyDw);
extern rgba_t     	reflection_sample(xyz_t *r);
extern rgba_t	  	bump_sample(int texnum, float nx, float ny, float nz, 
				float s, float t, float inv_w);
extern void		generate_spherical_texcoords(Object_t *op);


#endif
/* __RP_EXTERNS_H__ */


