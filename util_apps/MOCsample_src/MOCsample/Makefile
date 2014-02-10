#define the build commands
CC=gcc
CFLAGS=-c -Wall -Werror

BINDIR=../../../bin

PeSoRTADIR=../../../../PeSoRTA
PeSoRTA_LIBDIR=$(PeSoRTADIR)/lib
PeSoRTA_INCDIR=$(PeSoRTADIR)/include

MOCsample_SRCDIR=./

APP_BINDIR=$(BINDIR)
APP_NAME=mocsample
APP_LIBFLAGS1=
APP_LIBFLAGS2=-lrt
APP_OBJS=$(MOCsample_SRCDIR)/workload_MOCsample.o

all: PeSoRTA_apps
	
$(MOCsample_SRCDIR)/workload_MOCsample.o: $(MOCsample_SRCDIR)/workload_MOCsample.c $(PeSoRTA_INCDIR)/PeSoRTA.h
	$(CC) $(CFLAGS) -I $(PeSoRTA_INCDIR) -o $(MOCsample_SRCDIR)/workload_MOCsample.o \
	$(MOCsample_SRCDIR)/workload_MOCsample.c

include $(PeSoRTADIR)/PeSoRTA_APP.mk

libPredictor_clean:
	$(MAKE) -C $(pred_LIBDIR) clean

clean: PeSoRTA_apps_clean
	rm -rf $(MOCsample_SRCDIR)/workload_MOCsample.o

