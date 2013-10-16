all:
	$(MAKE) -C governor/
	$(MAKE) -C pbs_module/
	$(MAKE) -C pbsAllocator/
	$(MAKE) -C pbsSRT_sqrwav/
	$(MAKE) -C TSP_Predictor_test/
	$(MAKE) -C pbsSRT_PeSoRTA/
	$(MAKE) -C pbs_util_apps/

setup_expt: all
	sh system_scripts/moderate_enable.sh
	sh system_scripts/rt_limit_disable.sh
	insmod governor/LAMbS_governor.ko
	insmod pbs_module/pbs_module.ko

clean:
	$(MAKE) -C pbs_util_apps/ clean
	$(MAKE) -C pbsSRT_PeSoRTA/ clean
	$(MAKE) -C TSP_Predictor_test/ clean
	$(MAKE) -C pbsSRT_sqrwav/
	$(MAKE) -C pbsAllocator/ clean
	$(MAKE) -C pbs_module/ clean
	$(MAKE) -C governor/ clean

