
/*
 * simplified wrapper around random() function.
 * returns a float value in the range of 0.0 - 1.0
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
#include <math.h>
#include <time.h>

static int my_randinit = 0;

/* this is Mac specific... what is the right test to check for presence
 * of arc4random()? 
 */
#if defined (__APPLE__) && defined(__MACH__)
#   define USE_ARC4
#   define MY_RAND_MAX  (2147483647) /* 2^31 - 1, eigth Mersenne prime */
#endif

float
RPRandom(void)
{
    double      dval;

    if (!my_randinit) {    /* seed random number generator the first time it gets called */
#ifndef USE_ARC4
        srandom(time(NULL));
#endif
        my_randinit = 1;
    }

#ifdef USE_ARC4
    dval = (double) ((double)arc4random_uniform(MY_RAND_MAX)/((double)MY_RAND_MAX));
#else
    dval = (double)random()/(double)RAND_MAX;
#endif
    dval = (dval < 0.0f) ? 0.0f : dval; /* paranoid clamp */
    dval = (dval > 1.0f) ? 1.0f : dval; /* paranoid clamp */

    return((float) dval);
}


