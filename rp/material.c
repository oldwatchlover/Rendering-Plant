
/*
 * File:	material.c
 *
 * holds current material state 
 *
 * most functions are called from the parser to set material values
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
#include <string.h>
#include <math.h>

#include "rp.h"

/* there is very weak / no error or range checking as a user may desire
 * to "mis-use" shading parameters for artistic effects...
 *
 * (we do clamp final pixel value to range of 0-255 as it gets stored as
 * an u8 in the output file
 */

/* current material */
static Material_t       *CurrentMaterial = (Material_t *) NULL;
#if 0
static Material_t	*CurrentMaterialArray[MAX_OBJ_MATERIALS];
#endif

/* create a new, default material: */
static Material_t *
NewMaterial(void)
{
    Material_t	*m;

    m = (Material_t *) calloc(1, sizeof(Material_t));

	/* calloc() insures everything is zero, except we want: */
    m->color.r = 1.0;
    m->color.g = 1.0;
    m->color.b = 1.0;
    m->color.a = 1.0;

    m->highlight.r = 1.0;
    m->highlight.g = 1.0;
    m->highlight.b = 1.0;
    m->highlight.a = 1.0;

    m->shiny = 1.0;

    return(m);
}

/* return the current material (the one the input parser is modifying) */
Material_t *
RPGetCurrentMaterial(void)
{
    return (CurrentMaterial);
}

/* free the current material */
void
RPFreeCurrentMaterial(void)
{
    if (CurrentMaterial != (Material_t *)NULL) {
	free (CurrentMaterial);
	CurrentMaterial = (Material_t *) NULL;
    }
}

/* all parameters are in the range 0-1.0 */
void
RPSetMaterialColor(Colorf_t color)
{
    if (CurrentMaterial == (Material_t *)NULL) {
	CurrentMaterial = NewMaterial();
    }

    Material_t *m = CurrentMaterial;

    m->color.r = color.r;
    m->color.g = color.g;
    m->color.b = color.b;
    m->color.a = color.a;
}

void
RPSetMaterialAmbient(Colorf_t color)
{
    if (CurrentMaterial == (Material_t *)NULL) {
	CurrentMaterial = NewMaterial();
    }

    Material_t *m = CurrentMaterial;

    m->amb.r = color.r;
    m->amb.g = color.g;
    m->amb.b = color.b;
    m->amb.a = color.a;
}

void
RPSetMaterialDiffuse(Colorf_t color)
{
    if (CurrentMaterial == (Material_t *)NULL) {
	CurrentMaterial = NewMaterial();
    }

    Material_t *m = CurrentMaterial;

    m->diff.r = color.r;
    m->diff.g = color.g;
    m->diff.b = color.b;
    m->diff.a = color.a;
}

void
RPSetMaterialSpecular(Colorf_t color)
{
    if (CurrentMaterial == (Material_t *)NULL) {
	CurrentMaterial = NewMaterial();
    }

    Material_t *m = CurrentMaterial;

    m->spec.r = color.r;
    m->spec.g = color.g;
    m->spec.b = color.b;
    m->spec.a = color.a;
}

void
RPSetMaterialHighlight(Colorf_t color)
{
    if (CurrentMaterial == (Material_t *)NULL) {
	CurrentMaterial = NewMaterial();
    }

    Material_t *m = CurrentMaterial;

    m->highlight.r = color.r;
    m->highlight.g = color.g;
    m->highlight.b = color.b;
    m->highlight.a = color.a;
}

void
RPSetMaterialShiny(float value)
{
    if (CurrentMaterial == (Material_t *)NULL) {
	CurrentMaterial = NewMaterial();
    }

    Material_t *m = CurrentMaterial;

    m->shiny = value;
}

void
RPSetMaterialReflection(float value)
{
    if (CurrentMaterial == (Material_t *)NULL) {
	CurrentMaterial = NewMaterial();
    }

    Material_t *m = CurrentMaterial;

    m->Krefl = value;
}

void
RPSetMaterialRefraction(float value)
{
    if (CurrentMaterial == (Material_t *)NULL) {
	CurrentMaterial = NewMaterial();
    }

    Material_t *m = CurrentMaterial;

    m->Krefr = value;
}

void
RPSetMaterialTexture(char *name, int channel)
{
    int		texnum;

    if (CurrentMaterial == (Material_t *)NULL) {
	CurrentMaterial = NewMaterial();
    }

    Material_t *m = CurrentMaterial;

    if ((channel < MATERIAL_COLOR) || (channel > MATERIAL_CHANNELS)) {
	channel = MATERIAL_COLOR;
    } 

    if (name == (char *) NULL) {	/* clear texture from material */
        m->texture[channel] = (Texture_t *) NULL;
    }

    texnum = RPFindTexture(name);

    if (texnum < 0) {
        fprintf(stderr,"ERROR : %s : %d : could not bind texture [%s] (%d)\n",
                __FILE__,__LINE__,name,texnum);
        m->texture[channel] = (Texture_t *) NULL;
    } else {
	m->texture[channel] = RPScene.texture_list[texnum];
    }
}

