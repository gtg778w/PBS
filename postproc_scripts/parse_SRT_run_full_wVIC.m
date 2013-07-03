function [SRT_records] = parse_SRT_run_full_wVIC(dir_name, log_name)

    SRT_records = struct();

    %parse the log name
    
    %check that the name has enough parts
    i = strchr(log_name, '_');
    if(length(i) < 9)
        error(  ['Invalid name "%s" for a log file. Must contain at least 9 parts ' ...
                ' seperated by underscores("_").'], log_name);
    end
    
    name_prefix= log_name(1:end-4);
    split_name = strsplit(name_prefix, '_');
    name_parts = length(split_name);
    
    %${run#}"_SRT_PeSoRTA_"${APPNAME}"_"${CONFIGNAME}"_"${J1}"_"${A1}"_"${p1}"_"${c1}"_"${alpha}".log"
    SRT_records.run_no  = str2double(split_name{1});
    SRT_records.alpha   = str2double(split_name{name_parts});
    SRT_records.c1      = str2double(split_name{(name_parts-1)});
    %SRT_records.period = split_name{(name_parts-2)};
    SRT_records.predictor = split_name{(name_parts-3)};
    %SRT_records.job_count = split_name{(name_parts-4)};
    
    %combine the remaining parts to represent the workload name
    split_name = split_name(2:end-4);
    workload_name = split_name{1};
    for i = 2:length(split_name)
        workload_name = [workload_name, '_', split_name{i}];
    end
    SRT_records.workload_name = workload_name;
    
    full_path = sprintf('%s/%s', dir_name, log_name);
    raw_mat     = csvread(full_path);
    
    [raw_rows, raw_cols] = size(raw_mat);
    
    if(raw_cols < 8)
        error(  ['The csv file "%s" is incorrectly formatted. The number of ' ...
                ' columns is less than 8.'], log_name);
    end
    
    SRT_records.pid = raw_mat(1, 1);
    SRT_records.period = raw_mat(1, 2);
    SRT_records.job_count = raw_mat(1, 4);
    SRT_records.cumulative_budget = raw_mat(1, 5);
    SRT_records.cumulative_budget_sat = raw_mat(1, 6);
    SRT_records.consumed_budget = raw_mat(1, 7);
    SRT_records.estimated_mean_exectime = raw_mat(1, 3);
    SRT_records.total_misses = raw_mat(1, 8);

    SRT_records.C1 = raw_mat(2:end, 1);
    SRT_records.C2 = raw_mat(2:end, 2);
    SRT_records.VIC1 = raw_mat(2:end, 3);
    SRT_records.VIC2 = raw_mat(2:end, 4);
    miss = raw_mat(2:end, 5);
    
    SRT_records.abs_release_time = raw_mat(2:end, 6) - raw_mat(2, 6);
    SRT_records.abs_LFT_time = raw_mat(2:end, 7) - raw_mat(2, 6);
    
    SRT_records.last_sp_budget_allocated = raw_mat(2:end, 8);
    SRT_records.last_sp_budget_used = raw_mat(2:end, 9);
    SRT_records.C0hat_u = raw_mat(2:end, 10);
    SRT_records.C0hat_var = raw_mat(2:end, 11);
    SRT_records.Clhat_u = raw_mat(2:end, 12);
    SRT_records.Clhat_var = raw_mat(2:end, 13);
    
end
