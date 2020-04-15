GCC=gcc

CFLAGS=-g3

OFLAGS=-O2 -ffast-math

LFLAGS=-lm -lX11

#
all: clean
	$(GCC) $(CFLAGS) $(OFLAGS) flame.c brick.c -o brick $(LFLAGS)

#
clean:
	rm -Rf *~ brick
