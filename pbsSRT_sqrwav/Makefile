#define the build commands
CC=gcc
CFLAGS=-Wall
LIBFLAGS=-lpbsSRT -lPredictor -lrt -lm

BINDIR=../bin
SRCDIR=./
PBSDIR=../libpbsSRT
PREDDIR=../libPredictor

all: $(BINDIR)/sqrwavSRT

libpbsSRT.a:
	$(MAKE) -C $(PBSDIR)

libPredictor.a:
	$(MAKE) -C $(PREDDIR)

$(BINDIR)/sqrwavSRT: $(SRCDIR)/pbsSRT_sqrwav.c $(SRCDIR)/loadgen.h $(SRCDIR)/sqrwav.h libpbsSRT.a libPredictor.a
	$(CC) $(CFLAGS) -L $(PBSDIR) -L $(PREDDIR) -I $(PBSDIR) -I $(PREDDIR) -o $(BINDIR)/sqrwavSRT $(SRCDIR)/pbsSRT_sqrwav.c $(LIBFLAGS)

clean:
	rm -rf $(BINDIR)/sqrwavSRT
	$(MAKE) -C $(PBSDIR) clean
	$(MAKE) -C $(PREDDIR) clean
	
