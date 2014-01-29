
PBS_DIR = ../../../pbs_module

MOCsample_module-objs = MOCsample_main.o MOCsample_alloc.o MOCsample_perf.o MOCsample_nsec.o MOCsample_VIC.o

KERNELDIR ?= /lib/modules/$(shell uname -r)/build
VICSYMS=`pwd`/$(PBS_DIR)/Module.symvers
PWD := $(shell pwd)

obj-m += MOCsample_module.o

all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) KBUILD_EXTRA_SYMBOLS=$(VICSYMS) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
	$(RM) -r Module.markers modules.order Module.symvers .tmp_versions/
	
