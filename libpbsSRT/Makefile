#define the build commands
CC=gcc
CFLAGS=-c -Wall -Wmissing-prototypes
AR=ar
ARFLAGS= -rsv

MODDIR=../pbs_module

PREDICTORS=pbsSRT_predictor_template.o MABank.o MAVSLMSBank.o

LIBOBJS= pbsuser.o pbsSRT_predictor.o $(PREDICTORS)

all: libpbsSRT.a

libpbsSRT.a: $(LIBOBJS)
	$(AR) $(ARFLAGS) libpbsSRT.a $(LIBOBJS)

pbsuser.o: pbsuser.c pbsuser.h
	$(CC) $(CFLAGS) -I $(MODDIR) -o pbsuser.o pbsuser.c

pbsSRT_predictor.o: pbsSRT_predictor.c pbsSRT_predictor.h
	$(CC) $(CFLAGS) -o pbsSRT_predictor.o pbsSRT_predictor.c

pbsSRT_predictor_template.o: pbsSRT_predictor_template.c
	$(CC) $(CFLAGS) -o pbsSRT_predictor_template.o pbsSRT_predictor_template.c

MABank.o: MABank.c MABank.h
	$(CC) $(CFLAGS) -g -O3 -o MABank.o MABank.c

MAVSLMSBank.o: MAVSLMSBank.c MAVSLMSBank.h
	$(CC) $(CFLAGS) -g -O3 -o MAVSLMSBank.o MAVSLMSBank.c

predictor_test: libpbsSRT.a predictor_test.c
	$(CC) -Wall -Wmissing-prototypes -L . -g -o predictor_test predictor_test.c -lpbsSRT

clean:
	rm -f libpbsSRT.a $(LIBOBJS)

