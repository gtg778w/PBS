
PBSMODDIR := ../../pbs_module
GOVMODDIR := ../../governor

EXTRA_CFLAGS := -I$(src)/$(PBSMODDIR) -I$(src)/$(GOVMODDIR)

obj-m += hrtimer_response_test.o 
obj-m += VICtimer_response_test.o

KERNELDIR ?= /lib/modules/$(shell uname -r)/build

PBSSYMS=`pwd`/$(PBSMODDIR)/Module.symvers
GOVSYMS=`pwd`/$(GOVMODDIR)/Module.symvers

modules:
	$(MAKE) -C $(KERNELDIR) M=`pwd`/ KBUILD_EXTRA_SYMBOLS="$(PBSSYMS) $(GOVSYMS)" modules

clean:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	$(RM) Module.markers modules.order
