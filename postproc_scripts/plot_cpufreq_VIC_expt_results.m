
    logdir = '~/Desktop/log/cpufreq_VIC_expt/';

    workloads = {   '2301000', '2300000', '2200000', '2100000', '2000000', '1900000', ...
                    '1800000', '1700000', '1600000', '1500000', '1400000', '1300000', ...
                    '1200000'};
                
    workload_count = length(workloads);
    
    Caxis_vals = 
    VICaxis_vals
    
    for i = 1: workload_count
        workload_log_dir = sprintf('%s/%s', logdir, workloads{i});
        [SRT_records] = parse_SRT_full_wVIC_dir(workload_log_dir);
        plot_C_vs_VIC(SRT_records(1), 2, workloads{i}, logdir);
    end
    
