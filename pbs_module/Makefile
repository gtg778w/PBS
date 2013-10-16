
GOVERNOR_DIR = ../governor
EXTRA_CFLAGS=-I$(src)/$(GOVERNOR_DIR) -DVIC_Budget=$(VIC_BUDGET)

pbs_module-objs = module_main.o bw_mgt.o jb_mgt.o pbs_timing.o pbs_mmap.o \
                  pbs_active.o LAMbS.o \
                  pbs_budget.o pbs_budget_ns_helper.o pbs_budget_VIC_helper.o \
                  LAMbS_icount.o LAMbS_energy.o LAMbS_models.o LAMbS_VIC.o \
                  LAMbS_VICtimer.o

KERNELDIR ?= /lib/modules/$(shell uname -r)/build
GOVSYMS=`pwd`/$(GOVERNOR_DIR)/Module.symvers
PWD := $(shell pwd)

obj-m += pbs_module.o

all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) KBUILD_EXTRA_SYMBOLS=$(GOVSYMS) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) ./ clean
	$(RM) -r Module.markers modules.order Module.symvers .tmp_versions/
	
