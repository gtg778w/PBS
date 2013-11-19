#define the build commands
CC=gcc
CFLAGS=-Wall
LIBFLAGS=-lSRT -lPredictor -lrt -lm

BINDIR=../bin
SRCDIR=./
SRTDIR=../libSRT
PREDDIR=../libPredictor

all: $(BINDIR)/sqrwavSRT

libSRT.a:
	$(MAKE) -C $(SRTDIR)

libPredictor.a:
	$(MAKE) -C $(PREDDIR)

$(BINDIR)/sqrwavSRT: $(SRCDIR)/SRT_sqrwav.c $(SRCDIR)/loadgen.h $(SRCDIR)/sqrwav.h libSRT.a libPredictor.a
	$(CC) $(CFLAGS) -L $(SRTDIR) -L $(PREDDIR) -I $(SRTDIR) -I $(PREDDIR) -o $(BINDIR)/sqrwavSRT $(SRCDIR)/SRT_sqrwav.c $(LIBFLAGS)

clean:
	rm -rf $(BINDIR)/sqrwavSRT
	$(MAKE) -C $(SRTDIR) clean
	$(MAKE) -C $(PREDDIR) clean
	
