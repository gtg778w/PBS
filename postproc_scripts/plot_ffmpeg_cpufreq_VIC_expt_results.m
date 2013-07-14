
    logdir = '~/Desktop/log/ffmpeg/dec.bigbuckbunnyfull.mov';

    workloads = { 'freqconsthigh', 'freqconstmed', 'freqconstlow', 'freqcycle' };
                
    workload_count = length(workloads);
    
    workload_log_dir =  sprintf('%s/%s', logdir, 'freqconstmed');
    [SRT_records] =  parse_SRT_full_wVIC_dir(workload_log_dir);
    [summ_SRT_record] = summarizeResults_SRT_run_full_wVIC( SRT_records, ...
                                                            20, 80, 3);
    std_C = summ_SRT_record.avg_C1;
    std_VIC=summ_SRT_record.avg_VIC1;
    
    close all;
    
    for i = 1: workload_count
        workload_log_dir =  sprintf('%s/%s', logdir, workloads{i});
        [SRT_records] = parse_SRT_full_wVIC_dir(workload_log_dir);
        
        Caxis_vals = [0, 2.7e+11, 0, 7e+07];
        VICaxis_vals= [0, 2.7e+11, 0, 1.35e+08];
        plot_C_vs_VIC(  SRT_records(1), 2, workloads{i}, logdir, ...
                        Caxis_vals, VICaxis_vals);
                        
        [summ_SRT_record] =summarizeResults_SRT_run_full_wVIC(  SRT_records, ...
                                                                20, 80, 3);
                                                                
        norm_plot_filename =sprintf('norm_%s', workloads{i});
        
        Caxis_vals = [0, 2.7e+11, 0, 2];
        VICaxis_vals= [0, 2.7e+11, 0, 2];
        plot_norm_C_vs_VIC( summ_SRT_record, std_C, std_VIC, ...
                            norm_plot_filename, logdir, ...
                            Caxis_vals, VICaxis_vals);
    end
    
