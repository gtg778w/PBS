#define the build commands
CC=gcc
CFLAGS=-c -Wall -Wmissing-prototypes
AR=ar
ARFLAGS= -rsv

MODDIR=../pbs_module

LIBOBJS= SRT.o

all: libSRT.a

libSRT.a: $(LIBOBJS)
	$(AR) $(ARFLAGS) libSRT.a $(LIBOBJS)

SRT.o: SRT.c SRT.h
	$(CC) $(CFLAGS) -I $(MODDIR) -o SRT.o SRT.c

clean:
	rm -f libSRT.a $(LIBOBJS)

