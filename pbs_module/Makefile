ifneq ($(KERNELRELEASE),)
# call from kernel build system

obj-m	:= pbs_module.o
pbs_module-objs := module_main.o bw_mgt.o jb_mgt.o pbs_timing.o pbs_mmap.o pbs_active.o pba.o

else

KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD       := $(shell pwd)

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

endif

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions modules.order Module.symvers

