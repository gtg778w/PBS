function [data] = parse_csv_file(filename)

    raw_data = csvread(filename);
    row1 = raw_data(1, :);
    body = raw_data(2:end,:);
    
    data.pid                = row1(1, 1);
    data.period             = row1(1, 2);
    data.reservation_period = row1(1, 3);
    data.budget_type        = row1(1, 4);
    data.initial_exec_time  = row1(1, 5);
    data.job_count          = row1(1, 6);                       
    data.allocated_budget   = row1(1, 7);
    data.consumed_budget_ns = row1(1, 8);
    data.consumed_budget_VIC= row1(1, 9);
    data.total_misses       = row1(1, 10);
    
    data.miss_rate          = data.total_misses / data.job_count;
    
    #BUDGET_TYPE_VIC = 0;
    #BUDGET_TYPE_NS  = 1;
    if(data.budget_type == 1)
        consumed_budget     = data.consumed_budget_ns;
    elseif (data.budget_type == 0)
        consumed_budget     = data.consumed_budget_VIC;
    else
        error('File contains unknown budget_type (%i)', data.budget_type );
    end
    data.budget_util        = consumed_budget / data.allocated_budget;
    
    data.cpuusage_ns        = body(:,1);
    data.cpuusage_VIC       = body(:,2);
    data.miss               = body(:,3);
    data.releaseTime        = body(:,4);
    data.LFT                = body(:,5);
    data.budget_allocated   = body(:,6);
    data.budget_used        = body(:,7);
    data.u_c0               = body(:,8);
    data.var_c0             = body(:,9);
    data.u_cl               = body(:,10);
    data.var_cl             = body(:,11);
    
    if(data.budget_type == 1)
        data.prediction_error   = data.cpuusage_ns - data.u_c0;
        mean_cpuusage = mean(data.cpuusage_ns);
        data.norm_pred_error    = data.prediction_error / mean_cpuusage;
    elseif (data.budget_type == 0)
        data.prediction_error   = data.cpuusage_VIC - data.u_c0;
        mean_cpuusage = mean(data.cpuusage_VIC);
        data.norm_pred_error    = data.prediction_error / mean_cpuusage;
    else
        error('File contains unknown budget_type (%i)', data.budget_type );
    end
    
    relative_LFT    = (data.LFT - data.releaseTime);
    data.VFT        = relative_LFT - ...
                      ((1 - (data.budget_used./data.budget_allocated)) * ...
                      data.reservation_period);
    data.VFT_error  = data.VFT - data.period;
    data.norm_VFT_error = data.VFT_error / data.period;
end

