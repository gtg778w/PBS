SRCS	= gov_test.c freq_change_test.c
OBJS	= $(SRCS:.c=.o)

obj-m += $(OBJS)

all:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	$(RM) Module.markers modules.order
