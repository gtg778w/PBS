function [summary] = get_SRTlogrepetition_summary(SRT_folder)

    file_name_list = readdir(SRT_folder);
    
    file_count = length(file_name_list);
    
    missrate_array = [];
    budgetutil_array = [];
    normbudget_array = [];
    NRMSprederror_array = [];
    
    array_i = 1;
    for k = 1:file_count
        
        next_file_name = file_name_list{k};
        
        %Check if the file name is long enough to do the remaining tests without errors
        if(length(next_file_name) < 4)
            %The file name is not long enough
            %move on to the next file
            continue;            
        end
    
        %Check if the next file has a '.log' suffix
        if(1 ~= strcmp(next_file_name(end-3:end), '.log'))
            %file needs to end in the ".log" suffix
            %move on to the next file
            continue;
        end
        
        %Check if the file starts with a run number
        if(1 ~= isdigit(next_file_name(1)))
            %propper log files begin with digits
            %move on to the next file
            continue;            
        end

        logname = [SRT_folder, '/', next_file_name];

        parsed_data = parse_csv_file(logname);
        
        missrate_array(array_i)     = parsed_data.miss_rate;
        budgetutil_array(array_i)   = parsed_data.budget_util;
        normbudget_array(array_i)   = parsed_data.normalized_budget_allocation;
        NRMSprederror_array(array_i)= parsed_data.NRMSpred_error;
        array_i                 = array_i + 1;
    end

    summary.missrate_array  = missrate_array;
    summary.missrate_max    = max(missrate_array);
    summary.missrate_min    = min(missrate_array);
    summary.missrate_mean   = mean(missrate_array);

    summary.budgetutil_array= budgetutil_array;
    summary.budgetutil_max  = max(budgetutil_array);
    summary.budgetutil_min  = min(budgetutil_array);
    summary.budgetutil_mean = mean(budgetutil_array);

    summary.normbudget_array= normbudget_array;
    summary.normbudget_max  = max(normbudget_array);
    summary.normbudget_min  = min(normbudget_array);
    summary.normbudget_mean = mean(normbudget_array);

    summary.NRMSprederror_array = NRMSprederror_array;
    summary.NRMSprederror_max   = max(NRMSprederror_array);
    summary.NRMSprederror_min   = min(NRMSprederror_array);
    summary.NRMSprederror_mean  = mean(NRMSprederror_array); 
end

