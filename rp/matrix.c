
/*
 * File:	matrix.c
 *
 * Some useful matrix routines. This code is
 * decades old, kept getting re-written, so I abstracted it into this utilit file.
 *
 * This version is OpenGL - inspired, implementing a matrix stack like a GPU
 * driver might.
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
#include <math.h>

#include "rp.h"

/* matrix state: */
float 	near_plane;
float 	far_plane;

/* load these from input (top of stack) */
float	m_mtx[4][4];	/* model */
float	v_mtx[4][4];	/* viewing */
float	p_mtx[4][4];	/* projection */

/* compute these on push/pop ops */
float	mv_mtx[4][4];	/* m * v */
float	mvp_mtx[4][4];	/* m * v * p */
float	l_mtx[4][4];	/* lighting transform */

/* matrix stacks, each 16 matrices deep */
float	m_stack[16*16];
int	m_stackp = 0;

float	p_stack[16*16];
int	p_stackp = 0;

float	v_stack[16*16];
int	v_stackp = 0;

#ifdef DEBUG_MATRIX
/* debugging utility */
static void
print_matrix(char *title, u32 flags, float m[4][4])
{
    int		i, j;

    fprintf(stderr,"%s : (%08x)\n",title,flags);
    for (i=0; i<4; i++) {
	fprintf(stderr,"\t");
	for (j=0; j<4; j++) {
	    fprintf(stderr,"%f ",m[i][j]);
	}
	fprintf(stderr,"\n");
    }
}
#endif

static void
push_matrix(float m[4][4], u32 flags)
{
    int		i, j, k;

    if (Flagged(flags,MTX_TYPE_PROJ)) {
	k = p_stackp;
	for (i=0; i<4; i++) {
	    for (j=0; j<4; j++) {
		p_stack[k++] = m[i][j];
	    }
	}
	p_stackp += 16;
    } else if (Flagged(flags,MTX_TYPE_VIEW)) {
	k = v_stackp;
	for (i=0; i<4; i++) {
	    for (j=0; j<4; j++) {
		v_stack[k++] = m[i][j];
	    }
	}
	v_stackp += 16;
    } else if (Flagged(flags,MTX_TYPE_MODEL)) {
	k = m_stackp;
	for (i=0; i<4; i++) {
	    for (j=0; j<4; j++) {
		m_stack[k++] = m[i][j];
	    }
	}
	m_stackp += 16;
    }
}

void
cat_matrix(float mf[4][4], float nf[4][4], float res[4][4])
{
    int     i, j, k;
    float   temp[4][4];
    
    for (i=0; i<4; i++) {
	for (j=0; j<4; j++) {
	    temp[i][j] = 0.0;
	    for (k=0; k<4; k++) {
		temp[i][j] += mf[i][k] * nf[k][j];
	    }
	}
    }
    
    /* make sure we handle case where result is an input */
    for (i=0; i<4; i++) {
	for (j=0; j<4; j++) {
	    res[i][j] = temp[i][j];
	}
    }
}

void
transform_xyz(float m[4][4], xyz_t *vin, xyz_t *vout, float *wout)
{
    xyz_t	tmp;

    tmp.x = (m[0][0] * vin->x +
             m[1][0] * vin->y +
             m[2][0] * vin->z +
             m[3][0] * 1.0f);
    tmp.y = (m[0][1] * vin->x +
             m[1][1] * vin->y +
             m[2][1] * vin->z +
             m[3][1] * 1.0f);
    tmp.z = (m[0][2] * vin->x +
             m[1][2] * vin->y +
             m[2][2] * vin->z +
             m[3][2] * 1.0f);
    *wout = (m[0][3] * vin->x +
             m[1][3] * vin->y +
             m[2][3] * vin->z +
             m[3][3] * 1.0f);

    vout->x = tmp.x;
    vout->y = tmp.y;
    vout->z = tmp.z;
}

/* called from parser, this loads a new matrix */
void
load_matrix(float mtx[4][4], u32 flags)
{
    int		i, j;
    float	tmp_mtx[4][4];

#ifdef DEBUG_MATRIX
    print_matrix("MATRIX", flags, mtx);
#endif

    if (Flagged(flags,MTX_TYPE_PROJ)) {

	if (Flagged(flags,MTX_FLAG_PUSH)) {
	    push_matrix(p_mtx, flags);
	}

	if (Flagged(flags,MTX_FLAG_MULT)) {
	    cat_matrix(mtx, p_mtx, p_mtx);
	} else {	/* LOAD */
	    for (i=0; i<4; i++) {
		for (j=0; j<4; j++) {
		    p_mtx[i][j] = mtx[i][j];
		}
	    }
	}
    } else if (Flagged(flags,MTX_TYPE_VIEW)) { /* no stack for view, and no mult */
	for (i=0; i<4; i++) {
	    for (j=0; j<4; j++) {
		v_mtx[i][j] = mtx[i][j];
	    }
	}
    } else {	/* MODEL */
	
	if (Flagged(flags,MTX_FLAG_PUSH)) {
	    push_matrix(m_mtx, flags);
	}

	if (Flagged(flags,MTX_FLAG_MULT)) {
	    cat_matrix(mtx, m_mtx, m_mtx);
	} else {	/* LOAD */
	    for (i=0; i<4; i++) {
		for (j=0; j<4; j++) {
		    m_mtx[i][j] = mtx[i][j];
		}
	    }
	}
    }

    cat_matrix(m_mtx, v_mtx, mv_mtx);
    cat_matrix(mv_mtx, p_mtx, mvp_mtx);	/* compute new mvp matrix */

#ifdef DEBUG_MATRIX
      fprintf(stderr,"\nload_matrix() %d\n*********\n",flags);
      print_matrix("M MATRIX", 0, m_mtx);
      print_matrix("V MATRIX", 0, v_mtx);
      print_matrix("P MATRIX", 0, p_mtx);
      print_matrix("MV MATRIX", 0, mv_mtx);
      print_matrix("MVP MATRIX", 0, mvp_mtx);
      fprintf(stderr,"*********\n\n");
#endif


    /* compute new transform for lighting */
    ident_mtx(l_mtx);
    invert_mtx(mv_mtx, tmp_mtx);
    for (i=0; i<3; i++) {
	for (j=0; j<3; j++) {
	    l_mtx[i][j] = tmp_mtx[j][i];
	}
    }
}


void
pop_matrix(u32 flags)
{
    int		i, j, k;
    float	tmp_mtx[4][4];

    if (Flagged(flags,MTX_TYPE_PROJ)) {
	k = p_stackp;
	for (i=0; i<4; i++) {
	    for (j=0; j<4; j++) {
		p_mtx[i][j] = p_stack[k++];
	    }
	}
	p_stackp += 16;
    } else if (Flagged(flags,MTX_TYPE_VIEW)) {
	k = v_stackp;
	for (i=0; i<4; i++) {
	    for (j=0; j<4; j++) {
		v_mtx[i][j] = v_stack[k++];
	    }
	}
	v_stackp += 16;
    } else if (Flagged(flags,MTX_TYPE_MODEL)) {
	k = m_stackp;
	for (i=0; i<4; i++) {
	    for (j=0; j<4; j++) {
		m_mtx[i][j] = m_stack[k++];
	    }
	}
	m_stackp += 16;
    }

    cat_matrix(m_mtx, v_mtx, mv_mtx);
    cat_matrix(mv_mtx, p_mtx, mvp_mtx);	/* compute new mvp matrix */

    /* compute new transform for lighting */
    ident_mtx(l_mtx);
    invert_mtx(mv_mtx, tmp_mtx);
    for (i=0; i<3; i++) {
	for (j=0; j<3; j++) {
	    l_mtx[i][j] = tmp_mtx[i][j];
	}
    }
}

/* normalize a vector */
static void
normalize(float *x, float *y, float *z)
{
    float   m;

    m = 1/sqrtf((*x)*(*x) + (*y)*(*y) + (*z)*(*z));
    *x *= m;
    *y *= m;
    *z *= m;
}

/* generate an identity matrix */
void
ident_mtx(float mf[4][4])
{
    int     i, j;

    for (i=0; i<4; i++)
	for (j=0; j<4; j++)
	    if (i == j) mf[i][j] = 1.0;
	    else mf[i][j] = 0.0;
}

/* generate a perspective projection matrix */
void 
perspective_mtx(float mf[4][4], float fovy, float aspect, float near,float far)
{
    float   cot;

    far_plane = far;

    ident_mtx(mf);

    fovy *= 3.1415926 / 180.0;
    cot = cosf (fovy/2) / sinf (fovy/2);

    mf[0][0] = cot / aspect;
    mf[1][1] = cot;
    mf[2][2] = (near + far) / (near - far);
    mf[2][3] = -1;
    mf[3][2] = (2 * near * far) / (near - far);
    mf[3][3] = 0;
}

/* generate an orthographic projection matrix */
void
ortho_mtx(float mf[4][4], float l, float r, float b, float t, float n, float f)
{
    ident_mtx(mf);

    mf[0][0] = 2/(r-l);
    mf[1][1] = 2/(t-b);
    mf[2][2] = -2/(f-n);
    mf[3][0] = -(r+l)/(r-l);
    mf[3][1] = -(t+b)/(t-b);
    mf[3][2] = -(f+n)/(f-n);
    mf[3][3] = 1;
}

/* generate a viewing matrix */
void
lookat_mtx(float mf[4][4], float xEye, float yEye, float zEye,
	   float xAt,  float yAt,  float zAt,
	   float xUp,  float yUp,  float zUp)
{
    float   len, xLook, yLook, zLook, xRight, yRight, zRight;

    ident_mtx(mf);

    xLook = xAt - xEye;
    yLook = yAt - yEye;
    zLook = zAt - zEye;

	/* negate because +z is behind us (?) */
    len = -1.0 / sqrtf (xLook*xLook + yLook*yLook + zLook*zLook);
    xLook *= len;
    yLook *= len;
    zLook *= len;

    /* Right = Up x Look */

    xRight = yUp * zLook - zUp * yLook;
    yRight = zUp * xLook - xUp * zLook;
    zRight = xUp * yLook - yUp * xLook;
    len = 1.0 / sqrtf (xRight*xRight + yRight*yRight + zRight*zRight);
    xRight *= len;
    yRight *= len;
    zRight *= len;

    /* Up = Look x Right */

    xUp = yLook * zRight - zLook * yRight;
    yUp = zLook * xRight - xLook * zRight;
    zUp = xLook * yRight - yLook * xRight;
    len = 1.0 / sqrtf (xUp*xUp + yUp*yUp + zUp*zUp);
    xUp *= len;
    yUp *= len;
    zUp *= len;

    mf[0][0] = xRight;
    mf[1][0] = yRight;
    mf[2][0] = zRight;
    mf[3][0] = -(xEye * xRight + yEye * yRight + zEye * zRight);

    mf[0][1] = xUp;
    mf[1][1] = yUp;
    mf[2][1] = zUp;
    mf[3][1] = -(xEye * xUp + yEye * yUp + zEye * zUp);

    mf[0][2] = xLook;
    mf[1][2] = yLook;
    mf[2][2] = zLook;
    mf[3][2] = -(xEye * xLook + yEye * yLook + zEye * zLook);

    mf[0][3] = 0;
    mf[1][3] = 0;
    mf[2][3] = 0;
    mf[3][3] = 1;
}

/* generate a scale modeling matrix */
void
scale_mtx(float mf[4][4], float x, float y, float z)
{
    ident_mtx(mf);

    mf[0][0] = x;
    mf[1][1] = y;
    mf[2][2] = z;
    mf[3][3] = 1;
}

/* generate a rotation modeling matrix */
void
rotate_mtx(float mf[4][4], float a, float x, float y, float z)
{
    static float    dtor = 3.1415926 / 180.0;
    float   sine;
    float   cosine;
    float   ab, bc, ca, t;

    while (a >= 360.0)
	a -= 360.0;

    while (a <= 360.0)
	a += 360.0;

    normalize(&x, &y, &z);
    a *= dtor;
    sine = sinf(a);
    cosine = cosf(a);
    t = (1-cosine);
    ab = x*y*t;
    bc = y*z*t;
    ca = z*x*t;

    ident_mtx(mf);

    t = x*x;
    mf[0][0] = t+cosine*(1-t);
    mf[2][1] = bc-x*sine;
    mf[1][2] = bc+x*sine;

    t = y*y;
    mf[1][1] = t+cosine*(1-t);
    mf[2][0] = ca+y*sine;
    mf[0][2] = ca-y*sine;

    t = z*z;
    mf[2][2] = t+cosine*(1-t);
    mf[1][0] = ab-z*sine;
    mf[0][1] = ab+z*sine;
}

/* generate a translate modeling matrix */
void
translate_mtx(float mf[4][4], float x, float y, float z)
{
    ident_mtx(mf);

    mf[3][0] = x;
    mf[3][1] = y;
    mf[3][2] = z;
}

/* transposes original matrix om, to new matrix tm */
void 
transpose_mtx(float om[4][4], float tm[4][4])
{
    float	tmp[4][4];
    int		i, j;

	/* handle input and outpt are the same matrix */
    for (i=0; i<4; i++) {
	for (j=0; j<4; j++) {
	    tmp[i][j] = om[j][i];
	}
    }

    for (i=0; i<4; i++) {
	for (j=0; j<4; j++) {
	    tm[i][j] = tmp[i][j];
	}
    }
}

/* inverts original matrix om, to new matrix im */
void 
invert_mtx(float om[4][4], float im[4][4])
{
    int 	i, j, k;
    float 	mult, m[4][4];

    for (i=0; i<4; i++) {
	for(j=0; j<4; j++) {
	    if (i == j) 
		im[i][j] = 1.0;
	    else 
		im[i][j] = 0.0;
	    m[i][j] = om[i][j];
	}
    }

    for(i=0; i<3; i++) {
	for(j=i+1; j<4; j++) {
	    mult = m[j][i]/m[i][i];
	    m[j][i] = 0.0;
	    for(k=i+1; k<4; k++) {
		m[j][k] -= mult*m[i][k];
	    }
	    for(k=0; k<4; k++) {
		im[j][k] -= mult*im[i][k];
	    }
	}
    }

    for (i=3; i>0; i--) {
	for (j=i-1; j>=0; j--) {
	    mult = m[j][i]/m[i][i];
	    m[j][i] = 0.0;
	    for (k=3; k>=0; k--) {
		im[j][k] -= mult*im[i][k];
	    }
	}
    }

    for (i=0; i<4; i++) {
	mult = 1.0/m[i][i];
	m[i][i] = 1.0;
	for (j=0; j<4; j++) {
	    im[i][j] *= mult;
	}
    }
}


/* useful vector routines: */

void
vector_add(xyz_t *result, xyz_t *v1, xyz_t *v2)
{
    result->x = v1->x + v2->x; result->y = v1->y + v2->y; result->z = v1->z + v2->z;
}

void
vector_sub(xyz_t *result, xyz_t *v1, xyz_t *v2)
{
    result->x = v1->x - v2->x; result->y = v1->y - v2->y; result->z = v1->z - v2->z;
}

void
vector_scale(xyz_t *result, xyz_t *v, float s)
{
    result->x = s * v->x; result->y = s * v->y; result->z = s * v->z;
}

void
vector_cross(xyz_t *p, xyz_t *a, xyz_t *b)
{
    p->x = (a->y * b->z) - (a->z * b->y);
    p->y = (a->z * b->x) - (a->x * b->z);
    p->z = (a->x * b->y) - (a->y * b->x);
}

float
vector_dot(xyz_t v1, xyz_t v2)
{
    return (v1.x*v2.x + v1.y*v2.y + v1.z*v2.z);
}

void
make_normal(xyz_t *n, xyz_t *p1, xyz_t *p2, xyz_t *p3)
{
    float	d1x, d1y, d1z, d2x, d2y, d2z;

    d1x = p2->x - p1->x; d1y = p2->y - p1->y; d1z = p2->z - p1->z;
    d2x = p3->x - p2->x; d2y = p3->y - p2->y; d2z = p3->z - p2->z;

    n->x = (d1y * d2z) - (d1z * d2y);
    n->y = (d1z * d2x) - (d1x * d2z);
    n->z = (d1x * d2y) - (d1y * d2x);
}

void
vector_normalize(xyz_t *n)
{
    float       len;

    len = sqrtf(Sqr(n->x) + Sqr(n->y) + Sqr(n->z));
    if (len != 0.0f) {
        len = 1.0f / len;
        n->x *= len; n->y *= len; n->z *= len;
    }
}

