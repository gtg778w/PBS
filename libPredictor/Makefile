#define the build commands
CC=gcc
CFLAGS=-c -Wall -Wmissing-prototypes
AR=ar
ARFLAGS= -rsv

PREDICTORS=libPredictor_template.o MABank.o MAVSLMSBank.o MA.o

LIBOBJS= libPredictor.o libPredictor_variance.o $(PREDICTORS)

all: libPredictor.a

libPredictor.a: $(LIBOBJS)
	$(AR) $(ARFLAGS) libPredictor.a $(LIBOBJS)

libPredictor.o: libPredictor.c libPredictor.h
	$(CC) $(CFLAGS) -o libPredictor.o libPredictor.c

libPredictor_variance.o: libPredictor_variance.c libPredictor_variance.h
	$(CC) $(CFLAGS) -o libPredictor_variance.o libPredictor_variance.c

libPredictor_template.o: libPredictor_template.c
	$(CC) $(CFLAGS) -o libPredictor_template.o libPredictor_template.c

MA.o: MA.c MA.h libPredictor_variance.o
	$(CC) $(CFLAGS) -g -O3 -o MA.o MA.c
	
MABank.o: MABank.c MABank.h
	$(CC) $(CFLAGS) -g -O3 -o MABank.o MABank.c

MAVSLMSBank.o: MAVSLMSBank.c MAVSLMSBank.h
	$(CC) $(CFLAGS) -g -O3 -o MAVSLMSBank.o MAVSLMSBank.c

clean:
	rm -f libPredictor.a $(LIBOBJS)

