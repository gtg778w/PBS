all:
	$(MAKE) -C allocator/
	$(MAKE) -C sqrwavSRT/

clean:
	$(MAKE) -C allocator/ clean
	$(MAKE) -C sqrwavSRT/ clean

