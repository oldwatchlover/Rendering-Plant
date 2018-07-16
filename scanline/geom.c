
/*
 * File:        geom.c
 *
 * does some extra geometry processing
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
#include "scan.h"

/* everything is in world space when this is called...
 * multiply by the projection matrix and get screen coordinates,
 * clipcodes, etc. 
 *
 */
void
scan_obj_process(void)
{
    Object_t	*op;
    int		i;

    for (i=0; i<RPScene.obj_count; i++) {

	op = RPScene.obj_list[i];

        if (op->type == OBJ_TYPE_SPHERE) {

                /* can't handle these yet */

        } else if (op->type == OBJ_TYPE_POLY) {

	    RPProjectAllVertices(op->vert_count, op->verts);

	}	/* obj type is polygon */
    }		/* for all objects */
}


