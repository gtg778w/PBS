#define the build commands
CC=gcc
CFLAGS=-c -Wall

BINDIR=../bin

pred_LIBDIR=../libPredictor
pred_INCDIR=../libPredictor

SRT_INCDIR=../libSRT

PRDTST_SRCDIR=./prdtst_src

all: predictor_test

libPredictor.a:
	$(MAKE) -C $(pred_LIBDIR)
	
predictor_test: libPredictor.a $(PRDTST_SRCDIR)/predictor_test.c
	$(CC) -Wall -Wmissing-prototypes -L $(pred_LIBDIR) -I $(SRT_INCDIR) -I $(pred_LIBDIR) -o $(BINDIR)/predictor_test $(PRDTST_SRCDIR)/predictor_test.c -lPredictor -lrt

libPredictor_clean:
	$(MAKE) -C $(pred_LIBDIR) clean

clean: libPredictor_clean
	rm -rf $(BINDIR)/predictor_test

