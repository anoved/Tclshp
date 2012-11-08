# Path to Shapelib distribution directory.
# Tested with Shapelib 1.3.0, made with no modifications.
SHAPELIB_PREFIX = ../shapelib-1.3.0/

# Path to directory containing /include and /lib subdirectories
# containing Tcl headers and shared library, respectively.
TCL_PREFIX = /usr

SHAPELIB_OBJS = $(SHAPELIB_PREFIX)/shpopen.o \
				$(SHAPELIB_PREFIX)/dbfopen.o \
				$(SHAPELIB_PREFIX)/safileio.o

CFLAGS = -g -fPIC -Wall -Werror
CC = gcc

.PHONY: all clean test

all: Tclshp.so

tclshp.o: tclshp.c
	$(CC) $(CFLAGS) -c tclshp.c \
			-I$(SHAPELIB_PREFIX) \
			-I$(TCL_PREFIX)/include

Tclshp.so: tclshp.o $(SHAPELIB_OBJS)
	$(CC) -shared -W1,-soname,Tclshp \
			-o Tclshp.so tclshp.o $(SHAPELIB_OBJS) \
			-L$(TCL_PREFIX)/lib -ltcl8.5

clean:
	rm -f Tclshp.so tclshp.o

test:
	@./test.tcl > test.out
	@if test "`diff test.out test.txt`" = '' ; then \
		echo "Test succeeded."; \
		rm test.out testshp.shp testshp.shx testshp.dbf; \
	else \
		echo "Test failed:"; \
		diff test.out test.txt; \
	fi
