
/*
 * File:        state.c
 *
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

#include "objread.h"

char	current_object[80];
group_t	grouplist[80];
int	groupcount = 0;
group_t	*last_group;
group_t	*current_group;
int	current_smooth = 0;

void
state_init(void)
{
    strcpy(current_object, "default");

    groupcount = 0;
    current_group = &(grouplist[0]);
    last_group = (group_t *)NULL;
    strcpy(current_group->name, "default");
    current_smooth = 0;
}


/*
 * name the current object
 *
 * an object can have one name. if more than one "o" command is in a .obj
 * file, we should generate multiple objects
 *
 */

void
add_object(char *name)
{
    debug_printf(stderr,"set object : \t[%s]\n",name);
}

/*
 * groups are logical names applying to certain geometry.
 *
 * a polygon can belong to multiple groups.
 *
 * all polygons added after this call belong to these groups.
 *
 * groups are optional, and information only
 *
 */
void
add_group(int n, char *names[])
{
    int		i;

    debug_printf(stderr,"set group : \t");
    for (i=0; i<n; i++) {
        debug_printf(stderr,"[%s] ",names[i]);

#if 0
	strcpy(current_groups[current_groupcount].name, names[i]);
	current_groups[current_groupcount].first = 0; /* next triangle */
	/* check last group, see if we must set last tri pointer */
#endif
    }
    debug_printf(stderr,"\n");

    groupcount++;
}


/*
 * all vertices belong to a smooth group...
 *
 * 0 means no smooth.
 *
 * all vertices added after this is set, will belong to the 
 * group <val> 
 *
 * after reading in all data, we will compute vertex normals for each
 * smooth group in the object.
 *
 */
int current_smooth_group = 0;

void
add_smooth(int val)
{
    debug_printf(stderr,"set smooth : \t%d\n",val);

    current_smooth = val;
}

