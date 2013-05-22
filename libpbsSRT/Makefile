#define the build commands
CC=gcc
CFLAGS=-c -Wall -Wmissing-prototypes
AR=ar
ARFLAGS= -rsv

MODDIR=../pbs_module

PREDICTORS=

all: libpbsSRT.a

libpbsSRT.a: pbsuser.o $(PREDICTORS)
	$(AR) $(ARFLAGS) libpbsSRT.a pbsuser.o $(PREDICTORS)

pbsuser.o: pbsuser.c pbsuser.h
	$(CC) $(CFLAGS) -I $(MODDIR) -o pbsuser.o pbsuser.c

clean:
	rm -f libpbsSRT.a pbsuser.o $(PREDICTORS)

