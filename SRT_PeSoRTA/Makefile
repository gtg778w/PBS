#define the build commands
CC=gcc
CFLAGS=-Wall

SRT_LIBDIR=../libSRT
SRT_INCDIR=../libSRT

pred_LIBDIR=../libPredictor
pred_INCDIR=../libPredictor

APP_NAME=SRT
APP_LIBFLAGS1=-L $(SRT_LIBDIR) -L $(pred_LIBDIR)
APP_LIBFLAGS2=-lSRT -lPredictor -lrt -lm
APP_OBJS=SRT_PeSoRTA.o
APP_BINDIR=../bin

PeSoRTADIR=../../PeSoRTA
PeSoRTA_LIBDIR=$(PeSoRTADIR)/lib
PeSoRTA_INCDIR=$(PeSoRTADIR)/include

all: PeSoRTA_apps

libSRT.a:
	$(MAKE) -C $(SRT_LIBDIR)

libPredictor.a:
	$(MAKE) -C $(pred_LIBDIR)

SRT_PeSoRTA.o: SRT_PeSoRTA.c libSRT.a libPredictor.a
	$(CC) $(CFLAGS) -I $(SRT_INCDIR) -I $(pred_INCDIR) -I $(PeSoRTA_INCDIR) \
	-c -o SRT_PeSoRTA.o SRT_PeSoRTA.c

include $(PeSoRTADIR)/PeSoRTA_APP.mk

libSRT_clean:
	$(MAKE) -C $(SRT_LIBDIR) clean

libPredictor_clean:
	$(MAKE) -C $(pred_LIBDIR) clean

clean: PeSoRTA_apps_clean libSRT_clean libPredictor_clean
	rm -rf SRT_PeSoRTA.o

