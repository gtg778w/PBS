#define the build commands
CC=gcc
CFLAGS=-Wall

pbsSRT_LIBDIR=../libpbsSRT
pbsSRT_INCDIR=../libpbsSRT

pred_LIBDIR=../libPredictor
pred_INCDIR=../libPredictor

APP_NAME=pbsSRT
APP_LIBFLAGS1=-L $(pbsSRT_LIBDIR) -L $(pred_LIBDIR)
APP_LIBFLAGS2=-lpbsSRT -lPredictor -lrt -lm
APP_OBJS=pbsSRT_PeSoRTA.o
APP_BINDIR=../bin

PeSoRTADIR=../../PeSoRTA
PeSoRTA_LIBDIR=$(PeSoRTADIR)/lib
PeSoRTA_INCDIR=$(PeSoRTADIR)/include

all: PeSoRTA_apps

libpbsSRT.a:
	$(MAKE) -C $(pbsSRT_LIBDIR)

libPredictor.a:
	$(MAKE) -C $(pred_LIBDIR)

pbsSRT_PeSoRTA.o: pbsSRT_PeSoRTA.c libpbsSRT.a libPredictor.a
	$(CC) $(CFLAGS) -I $(pbsSRT_INCDIR) -I $(pred_INCDIR) -I $(PeSoRTA_INCDIR) \
	-c -o pbsSRT_PeSoRTA.o pbsSRT_PeSoRTA.c

include $(PeSoRTADIR)/PeSoRTA_APP.mk

libpbsSRT_clean:
	$(MAKE) -C $(pbsSRT_LIBDIR) clean

libPredictor_clean:
	$(MAKE) -C $(pred_LIBDIR) clean

clean: PeSoRTA_apps_clean libpbsSRT_clean libPredictor_clean
	rm -rf pbsSRT_PeSoRTA.o

