all:
	$(MAKE) -C pbs_module/
	$(MAKE) -C allocator/
	$(MAKE) -C sqrwavSRT/
	$(MAKE) -C poll_pbs_actv/

clean:
	$(MAKE) -C poll_pbs_actv/ clean
	$(MAKE) -C sqrwavSRT/ clean
	$(MAKE) -C allocator/ clean
	$(MAKE) -C pbs_module/ clean

