#define the build commands
CC=gcc
CFLAGS=-c -Wall -Werror

BINDIR=../../bin

MOCsample_DIR=./MOCsample
MOCsample_timed_DIR=./MOCsample_timed
MOCsample_module_DIR=./MOCsample_module

all: MOCsample_target MOCsample_timed_target MOCsample_module_target

MOCsample_target:
	$(MAKE) -C $(MOCsample_DIR)/

MOCsample_timed_target:
	$(MAKE) -C $(MOCsample_timed_DIR)/

MOCsample_module_target:
	$(MAKE) -C $(MOCsample_module_DIR)/
	mv $(MOCsample_module_DIR)/MOCsample_module.ko $(BINDIR)/

MOCsample_clean:
	$(MAKE) -C $(MOCsample_DIR)/ clean

MOCsample_timed_clean:
	$(MAKE) -C $(MOCsample_timed_DIR)/ clean

MOCsample_module_clean:
	$(MAKE) -C $(MOCsample_module_DIR)/ clean
	rm -rf $(BINDIR)/MOCsample_module.ko

clean: MOCsample_clean MOCsample_timed_clean MOCsample_module_clean

