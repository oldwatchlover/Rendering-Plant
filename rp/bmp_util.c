
/*
 * File:	bmp_util.c
 *
 * Routines to read/write MS .bmp images for texture and frame buffer output.
 *
 * Coded from the BMP file format wikipedia page... makes no attempt to support
 * all variations of this format.
 *
 * Writes out legal BMP files that have been readable with the few other programs I've tried.
 * Reads in 24-bit uncompressed BMP files, which seems pretty common with textures I've
 * grabbed off the net, or at least converted using programs like Gimp.
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
#include <errno.h>

#include "rp.h"

/* required file header, 14 bytes */
typedef struct {
    char 	type[2];        /* BM = Windows bitmap image */
    int 	filesize;       /* file size in bytes */
    int 	reserved;       /* set to 0 */
    int 	header_offset;	/* offset in bytes to pixel data (54 bytes) */
} BMPHeader_t;

/* 7 various DIB headers exist; we are using Windows NT or later (40 bytes) */
typedef struct {
    int 	DIB_size;           	/* size of this header (40 bytes) */
    int 	xres;          		/* width of image in pixels */
    int 	yres;         		/* height of image in pixels */
    short 	planes;       		/* number of color planes (must be 1) */
    short 	bpp;     		/* bits per pixel */
    int 	compress_type;    	/* compression method (0 if no compression) */
    int 	image_size;      	/* image size in bytes (0 if no compression) */
    int 	hppm;  			/* horizontal resolution in pixels per meter */
    int 	vppm;  			/* vertical resolution in pixels per meter */
    int 	colors_used;        	/* number of colors in palette (0 for full color) */
    int 	colors_important;	/* number of important colors (?) (0) */
} DIBHeader_t;

/* pixel storage:
 *
 * pixels are packed in rows, each row must be a multiple of 4 bytes: 
 *
 *    RowSize = ((bpp * xres + 31) / 32 ) * 4
 *
 * origin of the image, 0,0 is the lower left hand corner
 *
 * Pixel value order: blue, green, red, alpha (32b pixel)
 *
 */

/* 
 * read a BMP image (for textures)... only reads a fairly restrictive subset of legal 
 * BMP files...
 */
int
read_bmp(const char *filename, Texture_t *tex)
{
    FILE 		*file;
    BMPHeader_t 	bmph;
    DIBHeader_t 	dibh;
    int 		bpl, i, j, readval=0;
    unsigned char 	*line;

    if ((file=fopen(filename,"r")) == NULL) { 
	fprintf(stderr,"ERROR : %s : cannot open file [%s] errno = %d\n",
		__FILE__,filename,errno); 
	return(FALSE);
    }

	/* read BMP header: */
    i = fread(&bmph.type, 2, 1, file);
    i = fread(&bmph.filesize, 4, 1, file);
    i = fread(&bmph.reserved, 4, 1, file);
    i = fread(&bmph.header_offset, 4, 1, file);

	/* read DIB header: */
    i = fread(&dibh, sizeof(DIBHeader_t), 1, file);

#ifdef DEBUG_BMP
    fprintf(stderr,"\nread DIB Header: %d bytes\n",(int)(i*sizeof(DIBHeader_t)));
    fprintf(stderr,"BMP: dump BMP header:\n");
    fprintf(stderr,"----------------------------------------------------\n");
    fprintf(stderr,"\tType = %c%c\n",bmph.type[0],bmph.type[1]);
    fprintf(stderr,"\tSize = %d\n",bmph.filesize);
    fprintf(stderr,"\tReserved = %d\n",bmph.reserved);
    fprintf(stderr,"\tOffBits = %d\n",bmph.header_offset);
    
    fprintf(stderr,"\nBMP: dump DIB header:\n");
    fprintf(stderr,"----------------------------------------------------\n");
    fprintf(stderr,"\tSize = %d\n",dibh.DIB_size);
    fprintf(stderr,"\tXres = %d\n",dibh.xres);
    fprintf(stderr,"\tYres = %d\n",dibh.yres);
    fprintf(stderr,"\tPlanes = %d\n",dibh.planes);
    fprintf(stderr,"\tBpp = %d\n",dibh.bpp);
    fprintf(stderr,"\tCompression = %d\n",dibh.compress_type);
    fprintf(stderr,"\tImage Size = %d\n",dibh.image_size);
    fprintf(stderr,"\tH Pix Per Meter = %d\n",dibh.hppm);
    fprintf(stderr,"\tV Pix Per Meter = %d\n",dibh.vppm);
    fprintf(stderr,"\tColors Used = %d\n",dibh.colors_used);
    fprintf(stderr,"\tColors Important = %d\n",dibh.colors_important);
#endif

	/* check to see if there is more header besides the BMP and DIB header before 
         * pixel data... 
         */
    i = sizeof(BMPHeader_t)+sizeof(DIBHeader_t);	/* 54 bytes */
    if (bmph.header_offset > i) { 
	line = (unsigned char *) malloc(bmph.header_offset);
        i = bmph.header_offset - (sizeof(BMPHeader_t)+sizeof(DIBHeader_t));
	fprintf(stderr,"%s : %s : extra data after header... %d bytes\n",
		program_name,__FILE__,i);
	readval = fread(line, 1, i, file);	/* skip this space */
   	if (readval != i) {
	    fprintf(stderr,"ERROR : premature end of BMP file %s feof = %d, ferror = %d\n",
		filename,feof(file),ferror(file));
	    free(line);
	    return (FALSE);
	}
	free(line);
    }

	/* bit of a hack... read image into a tex structure (most common case) */

    tex->xres = dibh.xres; 
    tex->yres = dibh.yres; 

    bpl = dibh.image_size / dibh.yres;

    if (dibh.bpp != 24) {
	fprintf(stderr,"sorry, only 24 bit BMP files supported\n");
	return (FALSE);
    }

    tex->tmem = (rgba_t **) malloc(sizeof(rgba_t *) * tex->yres);
    for (i=0; i<tex->yres; i++) {
        tex->tmem[i] = (rgba_t *) malloc(sizeof(rgba_t) * tex->xres);
    }

    line = malloc(bpl);
    if (line == NULL) {
        fprintf(stderr, "Can't allocate memory for BMP file.\n");
        return(FALSE);
    }

    	/* read in the BMP pixel data */
    for (i=0; i<tex->yres; i++) {
        readval = fread(line, 1, bpl, file);
   	if (readval != bpl) {
	    fprintf(stderr,
		"ERROR : premature end of BMP file %s reading line %d got %d bytes\n",
		filename, i, readval);
	    fprintf(stderr,"ERROR : feof = %d ferror = %d\n",feof(file),ferror(file));
	    free(line);
	    return (FALSE);
	}
        for (j=0; j<tex->xres; j++) {
            tex->tmem[i][j].b = line[3*j+0];
            tex->tmem[i][j].g = line[3*j+1];
            tex->tmem[i][j].r = line[3*j+2];
            tex->tmem[i][j].a = 255;
        }
    }

    free(line);
    fclose(file);

    return(TRUE);
}

/* write out a 24 bit uncompressed BMP image file */
int 
write_bmp(const char *filename, int xres, int yres)
{
    FILE 		*file;
    BMPHeader_t 	bmph;
    DIBHeader_t 	dibh;
    int 		i, j, bpl;
    unsigned char 	*line;

    	/* length of each line must be a multiple of 4 bytes */
    bpl = (3 * (xres + 1) / 4) * 4;

	/* fill in BMP header: (14 bytes) */
    bmph.type[0] = 'B';
    bmph.type[1] = 'M';
    bmph.header_offset = 54;
    bmph.filesize = bmph.header_offset + bpl * yres;
    bmph.reserved = 0;

	/* fill in DIB header: (40 bytes) */
    dibh.DIB_size = 40;
    dibh.xres = xres;
    dibh.yres = yres;
    dibh.planes = 1;
    dibh.bpp = 24; /* no alpha saved for now */
    dibh.compress_type = 0;
    dibh.image_size = bpl * yres;
    dibh.hppm = 0;
    dibh.vppm = 0;
    dibh.colors_used = 0;       
    dibh.colors_important = 0; 

    if ((file=fopen(filename,"wb")) == NULL) 
	return(FALSE);
  
    /*
     * write out the header...
     *
     * can't do this in one operation because of the alignment issues 
     * in the structure (thanks Microsoft! /s)
     */
	/* write BMP header: */
    fwrite(&bmph.type, 2, 1, file);
    fwrite(&bmph.filesize, 4, 1, file);
    fwrite(&bmph.reserved, 4, 1, file);
    fwrite(&bmph.header_offset, 4, 1, file);

	/* write DIB header: */
    fwrite(&dibh, sizeof(DIBHeader_t), 1, file);

    line = malloc(bpl);
    if (line == NULL) {
        fprintf(stderr, "Can't allocate memory for BMP file.\n");
        return(FALSE);
    }

    for (i=(yres-1); i>=0; i--) {	/* bmp image upside down w.r.t. our cfb */
        for (j=0; j<xres; j++) {
	    line[3*j+0] = RPColorFrameBuffer[i][j].b;
	    line[3*j+1] = RPColorFrameBuffer[i][j].g;
	    line[3*j+2] = RPColorFrameBuffer[i][j].r;

        }
        fwrite(line, 1, bpl, file);
    }

    free(line);
    fclose(file);

    return(TRUE);
}

