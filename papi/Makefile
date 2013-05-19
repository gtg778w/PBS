PAPIINC = /home/mikel/workspace/safayet/papi-5.1.0/src/
PAPILIB = /home/mikel/workspace/safayet/papi-5.1.0/src/libpapi.a
CC = gcc
CFLAGS += -I$(PAPIINC)
OS = $(shell uname)

TARGETS_NTHD = test_code

TARGETS = $(TARGETS_NTHD)
LDFLAGS = $(PAPILIB)

all: $(TARGETS)

$(TARGETS_NTHD): %:%.o
	$(CC) -o $@ $(CFLAGS) $^ $(LDFLAGS)
