#define the build commands
CC=gcc
CFLAGS=-Wall
LIBFLAGS=-lpbsSRT -lrt -lm

BINDIR=../bin
SRCDIR=./
LIBDIR=../libpbsSRT

all: $(BINDIR)/sqrwavSRT

libpbsSRT.a:
	$(MAKE) -C $(LIBDIR)

$(BINDIR)/sqrwavSRT: $(SRCDIR)/sqrwav_SRT.c $(SRCDIR)/loadgen.h $(SRCDIR)/sqrwav.h libpbsSRT.a
	$(CC) $(CFLAGS) -L $(LIBDIR) -I $(LIBDIR) -o $(BINDIR)/sqrwavSRT $(SRCDIR)/sqrwav_SRT.c $(LIBFLAGS)

clean:
	rm -rf $(BINDIR)/sqrwavSRT
	$(MAKE) -C $(LIBDIR) clean
	
