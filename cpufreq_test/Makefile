ifneq ($(KERNELRELEASE),)
# call from kernel build system

obj-m	:= cpufreq_test.o
cpufreq_test-objs := module_main.o

else

KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD       := $(shell pwd)

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

endif

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions modules.order Module.symvers

