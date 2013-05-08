#define the build commands
CC=gcc
CFLAGS=-c -Wall
LIBFLAGS=-lrt -lm

BINDIR=./bin
SRCDIR=./allocator

all: $(BINDIR)/allocator

$(BINDIR)/allocator: $(SRCDIR)/pbs_ul_allocator.c $(SRCDIR)/pbs.h
	$(CC) $(CFLAGS) -o $(BINDIR)/allocator $(SRCDIR)/pbs_ul_allocator.c $(LIBFLAGS)

clean:
	rm -rf $(BINDIR)/allocator

