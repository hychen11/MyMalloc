CC=gcc
CFLAGS=-O3 -fPIC
DFLAGS=-g -O0 -fPIC
MALLOC_VERSION=BF
WDIR=..

all: mymalloc_test

mymalloc_test: mymalloc_test.c
	$(CC) $(DFLAGS) -I$(WDIR) -L$(WDIR) -D$(MALLOC_VERSION) -Wl,-rpath=$(WDIR) -o $@ mymalloc_test.c -lmymalloc -lrt

clean:
	rm -f *~ *.o mymalloc_test

clobber:
	rm -f *~ *.o
