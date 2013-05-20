#define the build commands
CC=gcc
CFLAGS=-Wall
LIBFLAGS=-lrt -lm

BINDIR=../bin
SRCDIR=./

MODULEDIR=../pbs_module

all: $(BINDIR)/sqrwavSRT

$(BINDIR)/sqrwavSRT: $(SRCDIR)/sqrwav_SRT.c $(SRCDIR)/pbsuser.c $(SRCDIR)/loadgen.h $(SRCDIR)/pbsuser.h $(SRCDIR)/sqrwav.h
	$(CC) $(CFLAGS) -I $(MODULEDIR) -o $(BINDIR)/sqrwavSRT $(SRCDIR)/sqrwav_SRT.c $(SRCDIR)/pbsuser.c $(LIBFLAGS)

clean:
	rm -rf $(BINDIR)/sqrwavSRT

