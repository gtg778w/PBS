
    logdir = '../data/sqrwav/constant_med';

    workloads = { 'freqcycle/ns', 'freqcycle/VIC'};
                
    workload_count = length(workloads);
    
    Caxis_vals = [0, 1.2e+11, 9e+06, 4e+07];
    VICaxis_vals= [0, 1.2e+11, 2.5e+07, 4.5e+07];
    
    for i = 1: workload_count
        workload_log_dir = sprintf('%s/%s', logdir, workloads{i});
        [SRT_records] = parse_SRT_full_wVIC_dir(workload_log_dir);
        plot_C_vs_VIC(  SRT_records(1), 2, workloads{i}, logdir, ...
                        Caxis_vals, VICaxis_vals);
    end
    
