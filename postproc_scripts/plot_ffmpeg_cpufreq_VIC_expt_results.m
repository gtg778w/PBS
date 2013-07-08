
    logdir = '~/Desktop/log/ffmpeg/dec.bigbuckbunnyfull.mov';

    workloads = { 'freqconsthigh', 'freqconstlow', 'freqcycle' };
                
    workload_count = length(workloads);
    
    Caxis_vals = [0, 6e+11, 0, 7e+07];
    VICaxis_vals= [0, 6e+11, 0, 1.35e+08];
    
    for i = 1: workload_count
        workload_log_dir = sprintf('%s/%s', logdir, workloads{i});
        [SRT_records] = parse_SRT_full_wVIC_dir(workload_log_dir);
        plot_C_vs_VIC(  SRT_records(1), 2, workloads{i}, logdir, ...
                        Caxis_vals, VICaxis_vals);
    end
    
