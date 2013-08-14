
    test two different workloads
        - constant execution time with a 10% noize
        - H264 decoding of a portion of the BigBuckBunny video
    
    test the workloads under three different configurations:
    
        1) Constant CPU frequency and CPU-time based budget allocation
        2) Oscillating CPU frequency and CPU-time-based budget allocation
        3) Oscillating CPU frequency and VIC-based budget allocation
        
    For each workload run in each configuration report the following:
    
        config 1 & 2) Execution time, VFT error, Deadline missrate,
        config 3) VIC, modified VFT error, Deadline missrate,
        
    Each experiment should be repeated N times.
    
    Total number of experiments: 2x3xN
    
    Estimated duration (assuming 5 minute workloads) = 30xN minutes

