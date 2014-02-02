What the experiment is trying to show?
    In short, the invariance of different MOCs under change in CPU frequency.
    
    This test is to be performed using a memory-bound workload and a CPU bound workload, that are otherwise equivalent in every other way. Also, to perform the test with an MPEG4 video-decoding workload.
    
    Graphs to show average MOC consumed against CPU frequency.
    
    If an MOC is invariant under change in the mode of operation, the MOC consumed should remain the same for all frequencies.
    If the MOC is not invariant, the MOC should change.
        - in the case of nanoseconds, the MOC should decrease
        - in the case of cycles, the MOC should remain the same for CPU bound and increase for membound
        - in the case of user-level instructions, the MOC should remain the same in both cases.
        - in the case of all-level instructions, the MOC should decrease slightly.
        - in the case of VIC, the MOC should remain roughly the same



start the allocator to run indefinitely
for each CPU frequency
    change to the given frequency
    start the background task
for each configuration (cpu bound and memory bound)
    for each CPU frequency
        for each MOC
            start the timing application
stop the allocator

