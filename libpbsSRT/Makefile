#define the build commands
CC=gcc
CFLAGS=-c -Wall -Wmissing-prototypes
AR=ar
ARFLAGS= -rsv

MODDIR=../pbs_module
PREDDIR=../libPredictor

LIBOBJS= pbsuser.o

all: libpbsSRT.a

libpbsSRT.a: $(LIBOBJS)
	$(AR) $(ARFLAGS) libpbsSRT.a $(LIBOBJS)

pbsuser.o: pbsuser.c pbsuser.h
	$(CC) $(CFLAGS) -I $(MODDIR) -I $(PREDDIR) -o pbsuser.o pbsuser.c

clean:
	rm -f libpbsSRT.a $(LIBOBJS)

