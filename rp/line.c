
/*
 * File:        line.h
 *
 * This file holds algorithms to draw lines on a frame buffer.
 * Originally written for an embedded system that did not have such
 * higher-level graphics primitives, I've found it useful for debugging
 * in all sorts of graphics software.
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
#include "line.h"

#ifndef Abs
#   define Abs(a)	((a) < 0 ? -(a) : (a))
#endif
#ifndef Clamp1
#   define Clamp1(a)	((a) < 1.0 ? (a) : 1.0)
#endif



/*
 * Wu's anti-aliased line algorithm.
 *
 * Coded from the wikipedia entry
 *
 */
void
wuline(int x0, int y0, int x1, int y1, PixelPlotProc PlotPixel)
{
    int		steep, xpix1, xpix2, ypix1, ypix2, x;
    float	dx, dy, xend, yend, xgap, gradient, intery;

#define wuPlot(x, y, c)	(*PlotPixel)((int)(x), (int)(y), (int)((c)*255));
#define Round(x)	((int) ((float)(x)+0.5)) 
#define iPart(x)	((int)(x))
#define fPart(x)	((float)(x) - (float)iPart((x)))
#define rfPart(x)	(1.0 - fPart((x)))
#define Swap(a, b)	do { __typeof__(a) tmp = b; b = a; a = tmp; } while (0);

    if (Abs(y1 - y0) > Abs(x1 - x0)) {
        steep = 1;
    } else {
        steep = 0;
    }

    if (steep) {
	Swap(x0, y0);
	Swap(x1, y1);
    }

    if (x0 > x1) {
	Swap(x0, x1);
	Swap(y0, y1);
    }

    dx = x1 - x0;
    dy = y1 - y0;

    if (dx == 0.0) { 
 	gradient = 1.0;
    } else {
        gradient = dy / dx;
    }

	/* handle first endpoint: */
    xend = Round(x0);
    yend = y0 + gradient * (xend - x0);
    xgap = rfPart(x0 + 0.5);
    xpix1 = xend;
    ypix1 = iPart(yend);

    if (steep) {
	wuPlot(ypix1, xpix1,   rfPart(yend) * xgap);
	wuPlot(ypix1+1, xpix1, fPart(yend) * xgap);
    } else {
	wuPlot(xpix1, ypix1,   rfPart(yend) * xgap);
	wuPlot(xpix1, ypix1+1, fPart(yend) * xgap);
    }
    intery = yend + gradient;

	/* handle second endpoint: */
    xend = Round(x1);
    yend = y1 + gradient * (xend - x1);
    xgap = rfPart(x1 + 0.5);
    xpix2 = xend;
    ypix2 = iPart(yend);

    if (steep) {
	wuPlot(ypix2, xpix2,   rfPart(yend) * xgap);
	wuPlot(ypix2+1, xpix2, fPart(yend) * xgap);
    } else {
	wuPlot(xpix2, ypix2,   rfPart(yend) * xgap);
	wuPlot(xpix2, ypix2+1, fPart(yend) * xgap);
    }

 	/* main loop */   

    if (steep) {
	for (x = xpix1+1; x<xpix2; x++) {
	    wuPlot(iPart(intery), x, rfPart(intery));
	    wuPlot(iPart(intery)+1, x, fPart(intery));
	    intery = intery + gradient;
 	}
    } else {
	for (x = xpix1+1; x<xpix2; x++) {
	    wuPlot(x, iPart(intery), rfPart(intery));
	    wuPlot(x, iPart(intery)+1, fPart(intery));
	    intery = intery + gradient;
 	}
    }

}



/*
 * bresline()
 *
 * Draws an un-anti-aliased line using Bresenham's algorithm.
 * The original reference for Bresenham's algoritm is:
 *
 *	Bresenham, J., E., "Algorithm for Computer Control of a
 *		Digital Plotter", IBM System Journal, Vol. 4, 
 *		pp. 25-30, 1965.
 *
 * This code was not based on the original paper, but is similar to the
 * presentation of the algorithm in the classic textbook:
 *
 *	Rogers, D., F., "Procedural Elements for Computer Graphics",
 *		McGraw-Hill Book Co., 1985, section 2-5, pp. 40-42.
 *
 */
void
bresline(int x1, int y1, int x2, int y2, PixelPlotProc PlotPixel)
{
    register int	d, i, inc1, inc2, inc3, inc4, dy, dx, x, y;

    dx = Abs(x2-x1);
    dy = Abs(y2-y1);
    x = x1; y = y1;
    (*PlotPixel)(x1, y1, 255);

    if (dy > dx) {		/* range over y axis */
	inc1 = (dx << 1);
     	d = inc1 - dy;
	inc2 = (dx-dy) << 1;
	inc3 = (y1 > y2) ? -1 : 1;
	inc4 = (x1 > x2) ? -1 : 1;
	for (i=y1; i != y2; i += inc3) {
	    if (d < 0) {
		d += inc1;
	    } else {
		d += inc2;
		x += inc4;
	    }
	    (*PlotPixel)(x, i, 255);
	}
    } else {			/* range over x axis */
    	inc1 = (dy << 1);
    	d = inc1 - dx;
    	inc2 = (dy-dx) << 1;
	inc3 = (x1 > x2) ? -1 : 1;
	inc4 = (y1 > y2) ? -1 : 1;
	for (i=x1; i != x2; i += inc3) {
	    if (d < 0) {
		d += inc1;
	    } else {
		d += inc2;
		y += inc4;
	    }
	    (*PlotPixel)(i, y, 255);
	}
    }
}

/*
 * abresline()
 *
 * Draws an anti-aliased line using a modified Bresenham's algorithm.
 * This algorithm is an extension of the one presented in:
 *
 *	Pitteway,, M.L.V., Watkinson, D.J., "Bresenham's Algorithm
 *		with Grey Scale", Communications of the ACM, Vol. 23,
 *		No. 11, November 1980, pp. 625-626.
 *
 * This paper treats the problem as a area boundary edge condition but does
 * not address the case where the line covers two pixels, the most
 * crucial part of the algorithm.
 *
 */
void
abresline(int x1, int y1, int x2, int y2, PixelPlotProc PlotPixel)
{
    register int	inc;
    int			I, dy, dx, x, y;
    float	 	m, w, d, w1, w2;

    dx = Abs(x2-x1);
    dy = Abs(y2-y1);

    I = 255;
    d = 1.0;

#define CALCWEIGHTS	\
    w1 = d;		\
    w2 = (1.0 - (d/2));	\
    w1 = Clamp1(w1+(w2/4));

    if (dy > dx) {			/* range over y axis */
    	if (y1 > y2) {
    	    x = x1; x1 = x2; x2 = x;
    	    x = y1; y1 = y2; y2 = x;
    	}

	if (x1 < x2)
	    inc = 1;
	else
	    inc = -1;

	x = x1;	y = y1;
    	m = (float) dx/dy;
    	w = 1.0 - m;

	while (y <= y2) {
	    if (d < w) {
		d = d + m;
		CALCWEIGHTS;
		(*PlotPixel)(x, y, (int) (w1*(float)I));
		(*PlotPixel)(x-inc, y, (int) (w2*(float)I));
	    } else {
		d = d - w;
		CALCWEIGHTS;
		(*PlotPixel)(x, y, (int) (w2*(float)I));
		x += inc;
		(*PlotPixel)(x, y, (int) (w1*(float)I));
	    }
	    y++;
	}
    } else {			/* range over x axis */
    	if (x1 > x2) {
    	    x = x1; x1 = x2; x2 = x;
    	    x = y1; y1 = y2; y2 = x;
    	}

	if (y1 < y2)
	    inc = 1;
	else
	    inc = -1;

     	if (dx == 0) {
	    (*PlotPixel)(x1, y1, I);
	    return;
	}

	x = x1;	y = y1;
    	m = (float) dy/dx;
    	w = 1.0 - m;

	while (x <= x2)	{
	    if (d < w) {
		d = d + m;
		CALCWEIGHTS;
		(*PlotPixel)(x, y, (int) (w1*(float)I));
		(*PlotPixel)(x, y-inc, (int) (w2*(float)I));
	    } else {
		d = d - w;
		CALCWEIGHTS;
		(*PlotPixel)(x, y, (int) (w2*(float)I));
		y += inc;
		(*PlotPixel)(x, y, (int) (w1*(float)I));
	    }
	    x++;
	}
    }
}


/*
 * circle_points()
 * 
 * private painting routine for circles.
 */
static void
circle_points(int xcen, int ycen, int x, int y, PixelPlotProc PlotPixel)
{
    (*PlotPixel)(xcen-x, ycen+y, 255);
    (*PlotPixel)(xcen+x, ycen+y, 255);

    (*PlotPixel)(xcen-x, ycen-y, 255);
    (*PlotPixel)(xcen+x, ycen-y, 255);

    (*PlotPixel)(xcen-y, ycen-x, 255);
    (*PlotPixel)(xcen+y, ycen-x, 255);

    (*PlotPixel)(xcen-y, ycen+x, 255);
    (*PlotPixel)(xcen+y, ycen+x, 255);
}

/*
 * brescircle()
 *
 * Bresenham's circle plotting routine.
 *
 */
void
bresircle(int xcenter, int ycenter, int radius, PixelPlotProc PlotPixel)
{
    register int	x, y, d;

    d = 3-(2*radius);
    x = radius;
    y = 0;

    while (y<x) {
	circle_points(xcenter, ycenter, x, y, PlotPixel);

	if (d < 0) {
	    d = d+(4*y)+6;
	} else {
	    d = d+(4*(y-x))+10;
	    x = x-1;
	}
	y++;
    }

    if (x == y) {
	circle_points(xcenter, ycenter, x, y, PlotPixel);
    }
}


