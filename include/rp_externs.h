
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

/* set by RPInit() in init.c. used to provide consistent error messages */
extern char		*program_name;

/* these scene data structures are available to the renderer.
 * (color_fb/zbuffer have access functions that are the preferred
 * interface, but the buffer is available for more complex operations)
 */
extern Scene_t		RPScene;
extern rgba_t   	RPColorFrameBuffer[MAX_YRES][MAX_XRES];
extern float	   	RPDepthFrameBuffer[MAX_YRES][MAX_XRES];

/*
 * these transformation matrices are available to the renderer:
 * see matrix.c for more information.
 */
extern float            m_mtx[4][4];    /* top of MODEL matrix stack      */
extern float            v_mtx[4][4];    /* top of VIEW matrix stack       */
extern float            p_mtx[4][4];    /* top of PROJECTION matrix stack */
extern float            mv_mtx[4][4];   /* current M*V matrix, cached     */
extern float            mvp_mtx[4][4];  /* current M*V*P matrix, cached   */
extern float            l_mtx[4][4];    /* light transform cached         */
                                        /* (transpose of inverse of MV)   */

/**************** exported function calls: ****************************/

/* from init.c */
extern void		RPInit(char *progname, u32 flags);

/* from rand.c */
extern float    	RPRandom(void);

/* from state.c */
extern void     	RPSetOutput(char *fname, int txres, int tyres);
extern void     	RPSetBackgroundColor(rgba_t *color);
extern void     	RPClearColorFB(rgba_t *color);
extern rgba_t   	*RPGetColorFBPixel(int x, int y);
extern void     	RPPutColorFBPixel(int x, int y, int r, int g, int b, int a);
extern void		RPDrawColorFBLine(int x1, int y1, int x2, int y2, 
					rgba_t color, int useaa);
extern void     	RPLoadBackgroundImage(void);
extern void     	RPSetBackgroundImageFile(char *filename);
extern int      	RPWriteColorFB(void);
extern void		RPClearDepthFB(float *zval);
extern int		RPTestDepthFB(int x, int y, float z);
extern void		RPPutDepthFBPixel(int x, int y, float z);
extern void     	RPSetObjectFlags(u32 flags);
extern void     	RPClearObjectFlags(u32 flags);
extern void     	RPSetCamera(xyz_t pos, xyz_t coi, xyz_t up, float fov, float aspect);
extern void		RPSetDepthRange(float near, float far);
extern void		RPSetProjection(float aspect, float near, float far);
extern void     	RPSetFog(float start, float end, float r, float g, float b, float a);
extern void     	RPSetScissor(int tulx, int tuly, int tlrx, int tlry);
extern void     	RPSetViewport(float sx, float sy, float sz, 
					float tx, float ty, float tz);

/* from scene.c */
extern int		RPParseInputFile(char *fname, int debugparse, char *cppdefs);
extern void		RPInitScene(void);
extern void     	RPSetSceneFlags(u32 flags);
extern void     	RPClearSceneFlags(u32 flags);
extern void     	RPSetGenericSceneFlags(u32 flags);
extern void     	RPClearGenericSceneFlags(u32 flags);

/* from material.c */
extern void		RPFreeCurrentMaterial(void);
extern Material_t     	*RPGetCurrentMaterial(void);
extern void		RPSetMaterialName(char *name);
extern void		RPSetMaterialColor(Colorf_t color);
extern void		RPSetMaterialAmbient(Colorf_t color);
extern void		RPSetMaterialDiffuse(Colorf_t color);
extern void		RPSetMaterialSpecular(Colorf_t color);
extern void		RPSetMaterialHighlight(Colorf_t color);
extern void		RPSetMaterialShiny(float value);
extern void		RPSetMaterialReflection(float value);
extern void		RPSetMaterialRefraction(float value);
extern void		RPSetMaterialTexture(char *name, int channel);

/* from light.c */
extern void     	RPSetAmbient(Colorf_t color);
extern void     	RPSetLight(xyz_t pos, Colorf_t color);
extern void     	RPSetSpotLight(xyz_t pos, xyz_t coi, float fov, float focus, 
				float range, float value, Colorf_t color);
extern void     	RPTransformLights(void);

/* from objects.c */
extern Object_t 	*RPAddObject(int type);
extern void		RPReadObjectFromFile(char *fname);
extern void		RPFreeObject(Object_t *op);
extern void		RPCleanupObjects(void);
extern void     	RPProcessObjects(int doProject);
extern void		RPDumpObject(Object_t *op);

/* from sphere.c */
extern void		RPEnableSphereSupport(int val);
extern void     	RPAddSphere(xyz_t center, float radius);
extern void		RPProcessSphere(Object_t *op, Sphere_t *sp);

/* from polygons.c */
extern void		RPInitInputPolygons(void);
extern void		RPFreeInputPolygons(void);
extern void     	RPAddTriangle(Tri_t *tri);
extern void     	RPCloseTriangleList(int tcount);
extern void     	RPProcessOneTriangle(Object_t *op, Tri_t *tri);
extern void     	RPProcessAllTriangles(Object_t *op, int count, Tri_t *tris, int doProject);

/* from vertex.c */
extern void		RPInitInputVertices(void);
extern void		RPFreeInputVertices(void);
extern void     	RPAddVertex(Vtx_t *vtx, int i);
extern void     	RPCloseVertexList(int vcount);
extern void     	RPTransformAllVertices(Object_t *op, int count, Vtx_t *verts);
extern void     	RPProjectAllVertices(int count, Vtx_t *verts);

/* from clip.c */
extern u8		RPGenerateVertexClipcodes(xyz_t *v, float w);
extern int		RPClipTriangle(Object_t *op, Tri_t *tri);

/* from texture.c */
extern int       	RPLoadTextureFromFile(int texnum, char *filename, u32 flags,
                             float sscale, float tscale,
                             float soff, float toff);
extern void	  	RPFreeTexture(int texnum);
extern void	  	RPCleanupTextures(void);
extern u32        	RPGetTextureFlags(int texnum);
extern int	  	RPFindTexture(char *name);
extern rgba_t     	RPPointSampleTexture(Texture_t *tex, 
				float s, float t, float inv_w);
extern rgba_t     	RPFilterSampleTexture(Texture_t *tex, 
				float s, float t, float inv_w,
				float DxDs, float DyDs, float DxDt, float DyDt,
				float DxDw, float DyDw);
extern void		RPGenerateSphericalTexcoords(Object_t *op);
extern void		RPGenerateCylindricalTexcoords(Object_t *op);



/***** the following files were imported from other libraries I wrote; I haven't 
 ***** changed the API function calls to match the coding style
 ****/

/* from matrix.c */
extern void             cat_matrix(float mf[4][4], float nf[4][4], float res[4][4]);
extern void             transform_xyz(float m[4][4], xyz_t *in, xyz_t *out, float *w);
extern void             load_matrix(float mtx[4][4], u32 flags);
extern void             pop_matrix(u32 flags);
extern void             ident_mtx(float mf[4][4]);
extern void             perspective_mtx(float mf[4][4], float fovy,
                                float aspect, float near,float far);
extern void             ortho_mtx(float mf[4][4], float l, float r,
                                float b, float t, float n, float f);
extern void             lookat_mtx(float mf[4][4],
                                float xEye, float yEye, float zEye,
                                float xAt,  float yAt,  float zAt,
                                float xUp,  float yUp,  float zUp);
extern void             scale_mtx(float mf[4][4], float x, float y, float z);
extern void             rotate_mtx(float mf[4][4], float a, float x, float y, float z);
extern void             translate_mtx(float mf[4][4], float x, float y, float z);
extern void             transpose_mtx(float om[4][4], float tm[4][4]);
extern void             invert_mtx(float om[4][4], float im[4][4]);
extern void             vector_add(xyz_t *result, xyz_t *v1, xyz_t *v2);
extern void             vector_sub(xyz_t *result, xyz_t *v1, xyz_t *v2);
extern void             vector_scale(xyz_t *result, xyz_t *v, float s);
extern void             vector_cross(xyz_t *result, xyz_t *v1, xyz_t *v2);
extern float            vector_dot(xyz_t v1, xyz_t v2);
extern void             make_normal(xyz_t *n, xyz_t *p1, xyz_t *p2, xyz_t *p3);
extern void             vector_normalize(xyz_t *n);

/* from bmp_util.c */
extern int      	read_bmp(const char *filename, Texture_t *tex);
extern int      	write_bmp(const char *filename, int width, int height);

/* from matrix.c */
	/* see matrix.h */

#endif
/* __RP_EXTERNS_H__ */


