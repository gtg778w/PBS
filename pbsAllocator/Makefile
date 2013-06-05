#define the build commands
CC=gcc
CFLAGS=-Wall -O3
LIBFLAGS=-lrt -lm

BINDIR=../bin
SRCDIR=./
MODDIR=../pbs_module

CSRC=$(SRCDIR)/pbsAllocator.c $(SRCDIR)/pbsAllocator_logging.c \
$(SRCDIR)/pbsAllocator_setup.c $(SRCDIR)/pbsAllocator_budget.c 

all: $(BINDIR)/pbsAllocator

$(BINDIR)/pbsAllocator: $(CSRC) $(SRCDIR)/pbsAllocator.h
	$(CC) $(CFLAGS) -I $(MODDIR) -o $(BINDIR)/pbsAllocator $(CSRC) $(LIBFLAGS)

clean:
	rm -rf $(BINDIR)/pbsAllocator

