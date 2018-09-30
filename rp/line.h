
/*
 * File:        line.h
 *
 * Header file for line.c, draws Bresenham style lines on a frame buffer.
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

#ifndef _line_h_
#define _line_h_ 1

typedef void (*PixelPlotProc) (int x, int y, int color);

extern void	wuline(int x1, int y1, int x2, int y2, PixelPlotProc PlotPixel);
extern void	bresline(int x1, int y1, int x2, int y2, PixelPlotProc PlotPixel);
extern void	abresline(int x1, int y1, int x2, int y2, PixelPlotProc PlotPixel);
extern void	brescircle(int xcenter, int ycenter, int radius, 
			PixelPlotProc PlotPixel);

#endif
