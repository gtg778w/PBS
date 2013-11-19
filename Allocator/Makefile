#define the build commands
CC=gcc
CFLAGS=-Wall -O3
LIBFLAGS=-lrt -lm

BINDIR=../bin
SRCDIR=./
MODDIR=../pbs_module

CSRC=$(SRCDIR)/Allocator.c $(SRCDIR)/Allocator_logging.c \
$(SRCDIR)/Allocator_setup.c $(SRCDIR)/Allocator_budget.c \
$(SRCDIR)/Allocator_modeladapters.c $(SRCDIR)/LMSVS.c

all: $(BINDIR)/pbsAllocator

$(BINDIR)/pbsAllocator: $(CSRC) $(SRCDIR)/Allocator.h $(SRCDIR)/LMSVS.h
	$(CC) $(CFLAGS) -I $(MODDIR) -o $(BINDIR)/Allocator $(CSRC) $(LIBFLAGS)

clean:
	rm -rf $(BINDIR)/Allocator

