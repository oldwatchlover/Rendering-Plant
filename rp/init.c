
/*
 * File:	init.c
 *
 * This file initializes the Render Plant library.
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
#include <locale.h>

#include "rp.h"

char		*program_name;
static int	program_flags;

void
RPInit(char *pname, u32 flags)
{
    setlocale(LC_ALL,"");       /* for prettier printing */

    if (pname == (char *) NULL) {
	program_name = (char *) calloc(MAX_FILENAME_LENGTH+1, sizeof(char));
	strncpy(program_name, getprogname(), MAX_FILENAME_LENGTH);
    } else {
	program_name = (char *) calloc(strlen(pname)+1, sizeof(char));
	strcpy(program_name, pname);
    }

    program_flags = flags;

}

