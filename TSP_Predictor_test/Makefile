#define the build commands
CC=gcc
CFLAGS=-c -Wall

BINDIR=../bin

PeSoRTADIR=../../PeSoRTA
PeSoRTA_LIBDIR=$(PeSoRTADIR)/lib
PeSoRTA_INCDIR=$(PeSoRTADIR)/include

pred_LIBDIR=../libPredictor
pred_INCDIR=../libPredictor

PRDTST_SRCDIR=./prdtst_src
TIMING_SRCDIR=./timing_src

APP_BINDIR=$(BINDIR)
APP_NAME=timing
APP_LIBFLAGS1=
APP_LIBFLAGS2=-lrt
APP_OBJS=$(TIMING_SRCDIR)/workload_timing.o

all: PeSoRTA_apps predictor_test

libPredictor.a:
	$(MAKE) -C $(pred_LIBDIR)
	
predictor_test: libPredictor.a $(PRDTST_SRCDIR)/predictor_test.c
	$(CC) -Wall -Wmissing-prototypes -L $(pred_LIBDIR) -I $(pred_LIBDIR) -o $(BINDIR)/predictor_test $(PRDTST_SRCDIR)/predictor_test.c -lPredictor

$(TIMING_SRCDIR)/workload_timing.o: $(TIMING_SRCDIR)/workload_timing.c $(PeSoRTA_INCDIR)/PeSoRTA.h
	$(CC) $(CFLAGS) -I $(PeSoRTA_INCDIR) -o $(TIMING_SRCDIR)/workload_timing.o \
	$(TIMING_SRCDIR)/workload_timing.c

include $(PeSoRTADIR)/PeSoRTA_APP.mk

libPredictor_clean:
	$(MAKE) -C $(pred_LIBDIR) clean

clean: PeSoRTA_apps_clean libPredictor_clean
	rm -rf $(TIMING_SRCDIR)/workload_timing.o $(BINDIR)/predictor_test

