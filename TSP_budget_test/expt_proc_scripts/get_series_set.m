function [series_set] = get_series_set(workload_dir)
    
    %get the list of files in the directory
    dir_filelist    = readdir(workload_dir);
    dir_filecount   = length(dir_filelist);
    
    series_names = cell(dir_filecount, 1);
    alphas = cell(dir_filecount, 1);
    miss_rates = cell(dir_filecount, 1);
    mean_miss_rates = cell(dir_filecount, 1);
    mean_allocated_budget = cell(dir_filecount, 1);
    RMS_VFT_error = cell(dir_filecount, 1);
    allocated_budget = cell(dir_filecount, 1);
    budget_utilization = cell(dir_filecount, 1);
    
    s = 0;
    %loop through each file in the root directory
    for ser_i = 1:dir_filecount
        
        %get the file name
        file_name = dir_filelist{ser_i};
    
        %get the file path
        file_path = sprintf('%s/%s', workload_dir, file_name);
        
        %check if it is a directory
        if ~ isdir(file_path)
            %its not a directory. move onto the next entry
            continue;
        end
        
        %check if it is the current directory or parent directory
        if( strcmp(file_name, '.') || strcmp(file_name, '..'))
            %its the '.' or '..' directory. move on to the next rnt
            continue;
        end
        
        s = s+1;
        series_names{s} = file_name;
        
        dir_filelist2 = readdir(file_path);
        dir_filecount2= length(dir_filelist2);
        
        alphas{s} = zeros(dir_filecount2, 1);
        mean_miss_rates{s} = zeros(dir_filecount2, 1);
        mean_allocated_budget{s} = zeros(dir_filecount2, 1);
        
        miss_rates{s} = [];
        RMS_VFT_error{s} = [];
        allocated_budget{s} = [];
        budget_utilization{s} = [];
        
        a = 0;
        %loop through each file in the series directory
        for alpha_i = 1:dir_filecount2

            %get the file name
            file_name2 = dir_filelist2{alpha_i};
        
            %get the file path
            file_path2 = sprintf('%s/%s', file_path, file_name2);
            
            %check if it is a directory
            if ~ isdir(file_path2)
                %its not a directory. move onto the next entry
                continue;
            end
            
            %check if it is the current directory or parent directory
            if( strcmp(file_name2, '.') || strcmp(file_name2, '..'))
                %its the '.' or '..' directory. move on to the next entry
                continue;
            end
        
            a = a + 1;
            (alphas{s})(a) = str2num(file_name2);
            
            SRT_records = process_SRT_rundir(file_path2);
            
            local_miss_rates = (SRT_records.total_misses/SRT_records.job_count);
            miss_rates{s}       = [miss_rates{s}, local_miss_rates];
            mean_miss_rates{s}(a)  = mean(local_miss_rates);
            
            local_allocated_budget = SRT_records.allocated_budget/SRT_records.job_count;
            allocated_budget{s} = [allocated_budget{s}, SRT_records.allocated_budget/SRT_records.job_count];
            mean_allocated_budget{s}(a) = mean(local_allocated_budget);
            
            RMS_VFT_error{s}    = [RMS_VFT_error{s}, SRT_records.RMS_VFT_error];
            budget_utilization{s}   = [budget_utilization{s}, (SRT_records.consumed_budget_ns./SRT_records.allocated_budget)];
        end
        (alphas{s}) = (alphas{s})(1:a);
        (mean_miss_rates{s}) = (mean_miss_rates{s})(1:a);
        (mean_allocated_budget{s}) = (mean_allocated_budget{s})(1:a);
    end

    series_names= series_names(1:s);
    alphas      = alphas(1:s);
    miss_rates  = miss_rates(1:s);
    mean_miss_rates = mean_miss_rates(1:s);
    mean_allocated_budget = mean_allocated_budget(1:s);
    RMS_VFT_error   = RMS_VFT_error(1:s);
    allocated_budget= allocated_budget(1:s);
    budget_utilization  = budget_utilization(1:s);
    series_set.series_names = series_names;
    series_set.alphas       = alphas;
    series_set.miss_rates   = miss_rates;
    series_set.mean_miss_rates  = mean_miss_rates;
    series_set.mean_allocated_budget = mean_allocated_budget;
    series_set.RMS_VFT_error= RMS_VFT_error;
    series_set.allocated_budget = allocated_budget;
    series_set.budget_utilization   = budget_utilization;
end

