function [timing_summary] = process_timing_wrkloads(    timing_dir, ...
                                                        application, ...
                                                        config)

    timing_summary.application  = application;
    timing_summary.config       = config;
    
    workload_dir = sprintf('%s/%s/%s/', timing_dir, application, config);
    file_name_list = readdir(workload_dir);
    
    execution_times = [];    
    n = 0;
    
    %Go through all files in the workload directory and parse the ones that are valid
    file_count = length(file_name_list);    
    for r = 1:file_count
        
        next_file_name = sprintf('%s/%s', workload_dir, file_name_list{r});
        
        %Check if the file name is long enough to do the remaining tests without errors
        if(length(next_file_name) < 4)
            %The file name is not long enough
            %move on to the next file
            continue;            
        end
    
        %Check if the next file has a '.csv' suffix
        if(1 ~= strcmp(next_file_name(end-3:end), '.csv'))
            %file needs to end in the ".csv" suffix
            %move on to the next file
            continue;
        end

        raw = load(next_file_name);
        
        n = n + 1;
        execution_times(:, n) = raw;
    end
    
    timing_acor = autocor(execution_times, 100);
    timing_acor = timing_acor(2:end, :);
    [timing_autocor_max, timing_autocor_max_lag] = max(timing_acor);
    
    timing_summary.max_acor = timing_autocor_max;
    timing_summary.max_acor_lag = timing_autocor_max_lag;
    
    execution_times = execution_times(1:end)';
    
    timing_summary.mean = mean(execution_times);
    timing_summary.std  = std(execution_times);
    timing_summary.min  = min(execution_times);
    timing_summary.max  = max(execution_times);
    timing_summary.p25  = prctile(execution_times, 25);
    timing_summary.p50  = prctile(execution_times, 50);
    timing_summary.p75  = prctile(execution_times, 75);
    
end
