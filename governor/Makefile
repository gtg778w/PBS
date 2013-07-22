SRCS	= LAMbS_mo.c LAMbS_molookup.c LAMbS_mostat.c LAMbS_governor.c
OBJS	= $(SRCS:.c=.o)

obj-m += $(OBJS)

all:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	$(RM) Module.markers modules.order
