#define the build commands
CC=gcc
CFLAGS=-Wall
LIBFLAGS=-lrt -lm

BINDIR=../bin
SRCDIR=./
MODDIR=../pbs_module

all: $(BINDIR)/allocator

$(BINDIR)/allocator: $(SRCDIR)/pbs_ul_allocator.c
	$(CC) $(CFLAGS) -I $(MODDIR) -o $(BINDIR)/allocator $(SRCDIR)/pbs_ul_allocator.c $(LIBFLAGS)

clean:
	rm -rf $(BINDIR)/allocator

