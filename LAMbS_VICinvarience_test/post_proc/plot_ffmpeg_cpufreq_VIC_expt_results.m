
    logdir = '../data/ffmpeg/dec.sintelfull.720p.mkv';

    workloads = { 'freqconstmed', 'freqcycle' };
                
    workload_count = length(workloads);

    workload_count = length(workloads);
    
    workload_log_dir =  sprintf('%s/%s', logdir, 'freqconstmed/ns/1.4');
    [SRT_records] =  parse_SRT_full_wVIC_dir(workload_log_dir);
    [summ_SRT_record] = summarizeResults_SRT_run_full_wVIC( SRT_records, ...
                                                            20, 80, 3);
    std_C = summ_SRT_record.avg_cpuusage_ns;
    std_VIC=summ_SRT_record.avg_cpuusage_VIC;
    
    close all;
    
    for i = 1: workload_count
        workload_log_dir =  sprintf('%s/%s/ns/1.4', logdir, workloads{i});
        [SRT_records] = parse_SRT_full_wVIC_dir(workload_log_dir);
      
        plot_C_vs_VIC(  SRT_records(1), 2, workloads{i}, logdir);
      
        [summ_SRT_record] =summarizeResults_SRT_run_full_wVIC(  SRT_records, ...
                                                                20, 80, 3);
                                                        
        norm_plot_filename =sprintf('norm_%s', workloads{i});
        
        Caxis_vals  =   [0, summ_SRT_record.rel_release_time(end), 0, 2];
        VICaxis_vals=   [0, summ_SRT_record.rel_release_time(end), 0, 2];
        plot_norm_C_vs_VIC( summ_SRT_record, std_C, std_VIC, ...
                            norm_plot_filename, logdir, ...
                            Caxis_vals, VICaxis_vals);
    end
    
