#
#
# Makefile for moray, a simple ray tracing renderer
#
# MIT License
# 
# Copyright (c) 2018 Steve Anderson
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#

include ./Makerules.mk

CFLAGS +=		-I./include

moray: CFLAGS +=	-DMORAY -I./ray
paint: CFLAGS +=	-DPAINT -I./painters
scan:  CFLAGS +=	-DSCAN -I./scanline

COMMON_LDFLAGS =	-L/usr/local/opt/flex/lib -L/usr/local/opt/gettext/lib -L./rp -L./objread 

moray: LDFLAGS +=	$(COMMON_LDFLAGS) -L./ray
paint: LDFLAGS +=	$(COMMON_LDFLAGS) -L./painters
scan:  LDFLAGS +=	$(COMMON_LDFLAGS) -L./scanline -L./ray

MORAY_OBJS =		moray.o
PAINT_OBJS =		paint.o
SCAN_OBJS =		scan.o

moray: LDLIBS =		-lrp -lobj -lray -lfl -lm 
paint: LDLIBS =		-lrp -lobj -lpaint -lfl -lm 
scan: LDLIBS =		-lrp -lobj -lscan -lray -lfl -lm

LIBOBJ =		objread/libobj.a
LIBRP 	=		rp/librp.a
LIBRAY 	=		ray/libray.a
LIBPAINT =		painters/libpaint.a
LIBSCAN =		scanline/libscan.a


default:	all 

all:	$(LIBOBJ) $(LIBRP) $(LIBRAY) $(LIBPAINT) $(LIBSCAN) moray paint scan 

moray:	$(MORAY_OBJS) $(LIBOBJ) $(LIBRP) $(LIBRAY) 
	$(CC) -o $@ $(MORAY_OBJS) $(LDFLAGS) $(LDLIBS)

paint:	$(PAINT_OBJS) $(LIBOBJ) $(LIBRP) $(LIBPAINT) 
	$(CC) -o $@ $(PAINT_OBJS) $(LDFLAGS) $(LDLIBS)

scan:	$(SCAN_OBJS) $(LIBOBJ) $(LIBRP) $(LIBRAY) $(LIBSCAN) 
	$(CC) -o $@ $(SCAN_OBJS) $(LDFLAGS) $(LDLIBS)

clean:
	+$(MAKE) -C rp clean
	+$(MAKE) -C ray clean
	+$(MAKE) -C painters clean
	+$(MAKE) -C scanline clean
	+$(MAKE) -C objread clean
	rm -f moray $(MORAY_OBJS)
	rm -f paint $(PAINT_OBJS)
	rm -f scan $(SCAN_OBJS)

checkpoint:
	cp -v M* *.[ch] .save
	cp -v rp/M* rp/*.[ch] rp/.save
	cp -v include/*.[ch] include/.save
	cp -v objread/M* objread/*.[ch] objread/.save
	cp -v ray/M* ray/*.[ch] ray/.save
	cp -v scanline/M* scanline/*.[ch] scanline/.save
	cp -v painters/M* painters/*.[ch] painters/.save

moray.o:	main.c
	$(CC) $(CFLAGS) -o $@ -c $<

paint.o:	main.c
	$(CC) $(CFLAGS) -o $@ -c $<

scan.o:		main.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(LIBRP):	rp/*.c
	+$(MAKE) -C rp librp.a 

$(LIBRAY):	ray/*.c
	+$(MAKE) -C ray libray.a

$(LIBPAINT):	painters/*.c
	+$(MAKE) -C painters libpaint.a

$(LIBSCAN):	scanline/*.c
	+$(MAKE) -C scanline libscan.a

$(LIBOBJ):	objread/*.c
	+$(MAKE) -C objread libobj.a


