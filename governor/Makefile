SRCS	= LAMbS_mo.c LAMbS_molookup.c LAMbS_mostat.c LAMbS_governor_main.c

obj-m += LAMbS_governor.o
LAMbS_governor-objs := $(SRCS:.c=.o)
all:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	$(RM) Module.markers modules.order
