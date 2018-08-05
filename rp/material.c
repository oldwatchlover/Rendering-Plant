
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
static int	CurrentMaterialIndex = 0;

/* create a new material structure: */
Material_t *
RPNewMaterial(void)
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

/* free the current material */
void
RPFreeMaterial(int index)
{
    if (RPScene.material_list[index] != (Material_t *)NULL) {
	if (RPScene.material_list[index]->name != (char *) NULL) {
	    free (RPScene.material_list[index]->name);
	}

	/* don't free texture, we expect RPCleanupTextures() to do that */

	free (RPScene.material_list[index]);
	RPScene.material_list[index] = (Material_t *) NULL;
    }
}

void
RPCleanupMaterials(void)
{
    int         i;

    for (i=0; i<RPScene.material_count; i++) {

        RPFreeMaterial(i);
    }

    RPScene.material_count = 0;
}

int
RPFindMaterial(char *name)
{
    Material_t   *m;
    int         i, index = -1;

    if (name == (char *) NULL)
        return (index);

    for (i=0; i<RPScene.material_count && index<0; i++) {
        m = RPScene.material_list[i];

        if (m != (Material_t *) NULL) {
            if (m->name != (char *) NULL) {
                if (strcmp(m->name, name) == 0) {
                    index = i;
                }
            }
        }
    }

    return (index);
}

/* set current material name */
void
RPSetMaterialName(char *name)
{
    int		index;

    if (name == NULL) {
	return;
    }

    index = RPFindMaterial(name);

    if (index < 0) {
	index = RPScene.material_count++;
    }

    if (index >= MAX_MATERIALS) {
	fprintf(stderr,"%s : ERROR : too many materials! (%d)\n",
		program_name, index);
	index = 0;
    }

    if (RPScene.material_list[index] == (Material_t *) NULL) {
	RPScene.material_list[index] = RPNewMaterial();
	CurrentMaterialIndex++;
    }

    RPScene.material_list[index]->name = (char *) malloc(strlen(name)+1);
    strcpy(RPScene.material_list[index]->name, name);

    CurrentMaterialIndex = index;
}

/* all parameters are in the range 0-1.0 */
void
RPSetMaterialColor(Colorf_t color)
{
    Material_t	*m;

    if (RPScene.material_list[CurrentMaterialIndex] == (Material_t *)NULL) {
	RPScene.material_list[CurrentMaterialIndex] = RPNewMaterial();
	RPScene.material_count++;
    }

    m = RPScene.material_list[CurrentMaterialIndex];
    m->color.r = color.r;
    m->color.g = color.g;
    m->color.b = color.b;
    m->color.a = color.a;
}

void
RPSetMaterialAmbient(Colorf_t color)
{
    Material_t *m;

    if (RPScene.material_list[CurrentMaterialIndex] == (Material_t *)NULL) {
	RPScene.material_list[CurrentMaterialIndex] = RPNewMaterial();
	RPScene.material_count++;
    }

    m = RPScene.material_list[CurrentMaterialIndex];
    m->amb.r = color.r;
    m->amb.g = color.g;
    m->amb.b = color.b;
    m->amb.a = color.a;
}

void
RPSetMaterialDiffuse(Colorf_t color)
{
    Material_t *m;

    if (RPScene.material_list[CurrentMaterialIndex] == (Material_t *)NULL) {
	RPScene.material_list[CurrentMaterialIndex] = RPNewMaterial();
	RPScene.material_count++;
    }

    m = RPScene.material_list[CurrentMaterialIndex];
    m->diff.r = color.r;
    m->diff.g = color.g;
    m->diff.b = color.b;
    m->diff.a = color.a;
}

void
RPSetMaterialSpecular(Colorf_t color)
{
    Material_t *m;

    if (RPScene.material_list[CurrentMaterialIndex] == (Material_t *)NULL) {
	RPScene.material_list[CurrentMaterialIndex] = RPNewMaterial();
	RPScene.material_count++;
    }

    m = RPScene.material_list[CurrentMaterialIndex];
    m->spec.r = color.r;
    m->spec.g = color.g;
    m->spec.b = color.b;
    m->spec.a = color.a;
}

void
RPSetMaterialHighlight(Colorf_t color)
{
    Material_t *m;

    if (RPScene.material_list[CurrentMaterialIndex] == (Material_t *)NULL) {
	RPScene.material_list[CurrentMaterialIndex] = RPNewMaterial();
	RPScene.material_count++;
    }

    m = RPScene.material_list[CurrentMaterialIndex];
    m->highlight.r = color.r;
    m->highlight.g = color.g;
    m->highlight.b = color.b;
    m->highlight.a = color.a;
}

void
RPSetMaterialShiny(float value)
{
    Material_t *m;

    if (RPScene.material_list[CurrentMaterialIndex] == (Material_t *)NULL) {
	RPScene.material_list[CurrentMaterialIndex] = RPNewMaterial();
	RPScene.material_count++;
    }

    m = RPScene.material_list[CurrentMaterialIndex];
    m->shiny = value;
}

void
RPSetMaterialReflection(float value)
{
    Material_t *m;

    if (RPScene.material_list[CurrentMaterialIndex] == (Material_t *)NULL) {
	RPScene.material_list[CurrentMaterialIndex] = RPNewMaterial();
	RPScene.material_count++;
    }

    m = RPScene.material_list[CurrentMaterialIndex];
    m->Krefl = value;
}

void
RPSetMaterialRefraction(float value)
{
    Material_t *m;

    if (RPScene.material_list[CurrentMaterialIndex] == (Material_t *)NULL) {
	RPScene.material_list[CurrentMaterialIndex] = RPNewMaterial();
	RPScene.material_count++;
    }

    m = RPScene.material_list[CurrentMaterialIndex];
    m->Krefr = value;
}

void
RPSetMaterialTexture(char *name, int channel)
{
    int		texnum;
    Material_t  *m;

    if (RPScene.material_list[CurrentMaterialIndex] == (Material_t *)NULL) {
	RPScene.material_list[CurrentMaterialIndex] = RPNewMaterial();
	RPScene.material_count++;
    }

    m = RPScene.material_list[CurrentMaterialIndex];

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

