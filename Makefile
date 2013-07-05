all:
	$(MAKE) -C pbs_module/
	$(MAKE) -C pbsAllocator/
	$(MAKE) -C sqrwavSRT/
	$(MAKE) -C pbsSRT_PeSoRTA/
	$(MAKE) -C pbs_util_apps/

setup_expt: all
	sh system_scripts/moderate_enable.sh
	sh system_scripts/rt_limit_disable.sh
	insmod pbs_module/pbs_module.ko

clean:
	$(MAKE) -C pbs_util_apps/ clean
	$(MAKE) -C pbsSRT_PeSoRTA/ clean
	$(MAKE) -C sqrwavSRT/ clean
	$(MAKE) -C pbsAllocator/ clean
	$(MAKE) -C pbs_module/ clean
