
/*
 * File:        edge.c
 *
 * create and process edge pairs (spans) across a scanline
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
#include "scan.h"

extern int	avg_epp;

/*
 * this version is a little messy, quick and sloppy:
 *
 *  - Each scanline has a linked list of edgepairs of all polygons that touch that scanline.
 *  - uses screen-space bounding rectangles for polygon span extents
 *  - no line to line (or pixel to pixel) coherence of edgepair lists
 *    (active edgepair list is rebuilt for each pixel)
 *  - edgepair memory allocated when building the scanline bucket lists... free()'d
 *    when that edgepair is to the left of x in the scanline processing. 
 * 
 */


/* allocate and return an edgepair structure */
ep_t *
new_ep(void)
{
    ep_t	*ep;

    ep = (ep_t *) calloc(1, sizeof(ep_t));

    return (ep);
}

/* walk the edgepair list, free'ing all elements. This is used to free up the 
 * active edgepair list after each pixel is rendered.
 */
void
free_eplist(ep_t **eplist)
{
    ep_t	*e = *eplist, *prev = (ep_t *) NULL;

    if (e == (ep_t *) NULL)
	return;

    if (e->next == (ep_t *) NULL) {
	free(e);
	e = (ep_t *) NULL;
	return;
    }

    while (e != (ep_t *) NULL) {
	prev = e;
  	e = e->next;
	free(prev);
    }
}

/* insert puts an ep into it's sorted position in an edgepair list */
/* ep list is **ep_t to handle adding first element to an empty list */
void
insert_edgepair(ep_t **eplist, ep_t *ep)
{
    ep_t	*e = *eplist, *prev;

    if (e == (ep_t *) NULL) {	 /* list empty, make ep the first element */
	*eplist = ep;
	return;
    }

    if (ep->min.x <= e->min.x) { /* before first element, make ep the first element */
	ep->next = e;
	*eplist = ep;
	return;
    }

    prev = e;
    e = e->next;

    while (e != (ep_t *) NULL) {       /* find right place to insert ep into list */
	if (e->min.x <= ep->min.x) {   /* not here, check the next one */
	    prev = e;
	    e = e->next;
        } else {		       /* insert here */
	    prev->next = ep;
	    ep->next = e;
	    return;
        }
    }

    prev->next = ep;		/* else insert at the tail */
    ep->next = e;
}

/* add a copy of an edgepair to the end of an unsorted list, used for short/fast active list */
/* allocated memory free'd after the pixel is rendered with call to free_eplist() */ 
static ep_t *
add_edgepair(ep_t *eplist, ep_t *ep)
{
    ep_t	*e, *newp, *prev = (ep_t *) NULL;

    newp = new_ep();
    bcopy((const void *)ep, (void *)newp, sizeof(ep_t));
    newp->next = (ep_t *) NULL;

    if (eplist == (ep_t *) NULL) {
        return (newp);
    }

    prev = eplist;
    e = eplist->next;
    while (e != (ep_t *) NULL) {
	prev = e;
	e = e->next;
    }

    prev->next = newp;

    return (eplist);
}

/* builds and returns a list of active edgepairs (from the original scanline edgepair list)
 * does NOT allocate memory or make copies, just copies the pointers 
 * but it DOES free() edgepairs once they are to the left of current x
 */
static ep_t *
active_edgepairs(int x, ep_t *eplist)
{
    ep_t	*e = eplist, *active = (ep_t *) NULL;

    while (e != (ep_t *)NULL) {
        if (x >= e->min.x && x <= e->max.x)
	    active = add_edgepair(active, e);
        e = e->next;
    }

    return(active);
}

/* for debugging. how many edgepairs in this list? */
int
count_edgepairs(ep_t *eplist)
{
    ep_t	*e = eplist;
    int		retval = 0;

    while (e != NULL) {
	e = e->next;
	retval++;
    }

    return (retval);
}

/* starts with the full eplist for this scanline.
 *  - for each x:
 *       -  identify edgpairs to resolve
 *       -  resolve/render that pixel
 * 
 * this is sloppy; rebuilds active ep list for each pixel
 */
void
process_edgepairs(int y, ep_t **eplist)
{
    ep_t	*active;
    int 	i;

    for (i=0; i<RPScene.xres; i++) {

	active = active_edgepairs(i, *eplist);

	avg_epp += count_edgepairs(active);
        
	cast_primary_ray(i, y, active);

	free_eplist(&active);
    }
}

void
print_edgepairs(ep_t *eplist)
{
    ep_t	*e = eplist;

    while (e != (ep_t *) NULL) {
	fprintf(stderr,"[(%d), %d-%d]   ",e->id,e->min.x,e->max.x);
	e = e->next;
    }
    fprintf(stderr,"\n");
}


