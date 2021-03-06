#
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

include ../Makerules.mk

# needed for non-standard flex install on mac:
CPPFLAGS +=	-I/usr/local/opt/flex/include -I/usr/local/opt/gettext/include

CFLAGS +=	-I../include

# more info while processing .obj files...
#CFLAGS +=      -DDEBUG_LIBOBJ

# yacc flags:
YFLAGS = -v -d -t
# lex flags:
LFLAGS = -P 

GENERATED_FILES = y.tab.h y.tab.c y.output lex.zz.c

#
# source code files: 
#
OBJREAD_CFILES =	objread.c 	\
			vertex.c 	\
			face.c	 	\
			state.c	 	\
			texture.c 	\
			object.c 	\
			material.c 

PARSER_OBJS =	parser.lx.o parser.g.o

OBJREAD_OBJECTS =	$(OBJREAD_CFILES:.c=.o) $(PARSER_OBJS)

TARGET =	libobj.a

default: $(TARGET)

clean:
	/bin/rm -rf $(TARGET) $(OBJREAD_OBJECTS) $(GENERATED_FILES)

$(TARGET): $(OBJREAD_OBJECTS)
	ar rcs $(TARGET) $(OBJREAD_OBJECTS)

parser.g.o: parser.g.y y.tab.h y.tab.c
	$(CC) -c $(INCLUDES) $(CFLAGS) y.tab.c -o parser.g.o

parser.lx.o: parser.lx.l y.tab.h
#	lex parser.lx.l
	flex parser.lx.l
	$(CC) -c $(INCLUDES) $(CFLAGS) lex.zz.c -o parser.lx.o

y.tab.h : parser.g.y
#	yacc $(YFLAGS) parser.g.y
	bison -y $(YFLAGS) parser.g.y

y.tab.c: y.tab.h

