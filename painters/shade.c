
/*
 * File:        shade.c
 *
 * Pixel shade code for the painter's algorithm.
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
#include <string.h>

#include "rp.h"
#include "paint.h"

/*
 * 
 * tex contrib, etc. happens in the rasterizer. This calculates
 * only the lighting factor for appication there.
 *
 */
void
shade_pixel(Object_t *op, Tri_t *tri, xyz_t *norm, xyz_t *point, xyz_t *view, Colorf_t *shade)
{
    Material_t		*m;
    Light_t		*light;
    xyz_t		N, L, H;
    float       	NdotL, NdotH;
    int			i;

    m = &(op->materials[tri->material_id]);

    shade->r = 0.0; shade->g = 0.0; shade->b = 0.0; shade->a = 0.0;

    for (i=0; i<RPScene.light_count; i++) {

        light = RPScene.light_list[i];

             /* calculate N vector */
        N.x = norm->x; N.y = norm->y; N.z = norm->z;
        vector_normalize(&N);

             /* calculate L vector */
        vector_sub(&L, &(light->pos), point);
        vector_normalize(&L);

             /* calculate H vector */
        vector_add(&H, &L, view);
        vector_normalize(&H);

        NdotL = Clamp0x(vector_dot(N,L), 1.0f);
        NdotH = Clamp0x(vector_dot(N,H), 1.0f);

        NdotH = powf(NdotH, m->shiny);

        shade->r += (m->amb.r * light->color.r) +
                    (m->diff.r * NdotL * light->color.r) +
                    (m->spec.r * NdotH * m->highlight.r * light->color.r);
        shade->g += (m->amb.g * light->color.g) +
                    (m->diff.g * NdotL * light->color.g) +
                    (m->spec.g * NdotH * m->highlight.g * light->color.g);
        shade->b += (m->amb.b * light->color.b) +
                    (m->diff.b * NdotL * light->color.b) +
                    (m->spec.b * NdotH * m->highlight.b * light->color.b);
        shade->a += (m->amb.a * light->color.r) +
                    (m->diff.a * NdotL * light->color.a) +
                    (m->spec.a * NdotH * m->highlight.a * light->color.a);
    }
}


