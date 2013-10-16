#define the build commands
CC=gcc
CFLAGS=-c -Wall -Wmissing-prototypes
AR=ar
ARFLAGS= -rsv

PREDICTORS=libPredictor_template.o MABank.o MAVSLMSBank.o

LIBOBJS= libPredictor.o $(PREDICTORS)

all: libPredictor.a

libPredictor.a: $(LIBOBJS)
	$(AR) $(ARFLAGS) libPredictor.a $(LIBOBJS)

libPredictor.o: libPredictor.c libPredictor.h
	$(CC) $(CFLAGS) -o libPredictor.o libPredictor.c

libPredictor_template.o: libPredictor_template.c
	$(CC) $(CFLAGS) -o libPredictor_template.o libPredictor_template.c

MABank.o: MABank.c MABank.h
	$(CC) $(CFLAGS) -g -O3 -o MABank.o MABank.c

MAVSLMSBank.o: MAVSLMSBank.c MAVSLMSBank.h
	$(CC) $(CFLAGS) -g -O3 -o MAVSLMSBank.o MAVSLMSBank.c

predictor_test: libPredictor.a predictor_test.c
	$(CC) -Wall -Wmissing-prototypes -L . -g -o predictor_test predictor_test.c -lPredictor

clean:
	rm -f libPredictor.a $(LIBOBJS)

