#define the build commands
CC=gcc
CFLAGS=-c -Wall -Werror

BINDIR=../../../bin

PeSoRTADIR=../../../../PeSoRTA
PeSoRTA_LIBDIR=$(PeSoRTADIR)/lib
PeSoRTA_INCDIR=$(PeSoRTADIR)/include

MOCsample_SRCDIR=./
MOCsample_MODDIR=../MOCsample_module/

APP_BINDIR=$(BINDIR)
APP_NAME=mocsample_timed
APP_LIBFLAGS1=
APP_LIBFLAGS2=-lrt
APP_OBJS=$(MOCsample_SRCDIR)/workload_MOCsample_timed.o

all: PeSoRTA_apps
	
$(MOCsample_SRCDIR)/workload_MOCsample_timed.o: $(MOCsample_SRCDIR)/workload_MOCsample_timed.c $(MOCsample_SRCDIR)/MOCsample_timer_user.h $(PeSoRTA_INCDIR)/PeSoRTA.h 
	$(CC) $(CFLAGS) -I $(PeSoRTA_INCDIR) -I $(MOCsample_MODDIR)/ -o $(MOCsample_SRCDIR)/workload_MOCsample_timed.o \
	$(MOCsample_SRCDIR)/workload_MOCsample_timed.c

include $(PeSoRTADIR)/PeSoRTA_APP.mk

libPredictor_clean:
	$(MAKE) -C $(pred_LIBDIR) clean

clean: PeSoRTA_apps_clean
	rm -rf $(MOCsample_SRCDIR)/workload_MOCsample_timed.o

