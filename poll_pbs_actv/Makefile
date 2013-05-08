#define the build commands
CC=gcc
CFLAGS=-c -Wall
LIBFLAGS=

BINDIR=../bin
SRCDIR=./

all: $(BINDIR)/poll_pbs_actv

$(BINDIR)/poll_pbs_actv: $(SRCDIR)/poll_pbs_actv.c
	$(CC) $(CFLAGS) -o $(BINDIR)/poll_pbs_actv $(SRCDIR)/poll_pbs_actv.c

clean:
	rm -rf $(BINDIR)/poll_pbs_actv

