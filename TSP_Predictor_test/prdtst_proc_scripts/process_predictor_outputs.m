function [prediction_summary] = process_predictor_outputs(  prdtst_dir, ...
                                                            application, ...
                                                            config, ...
                                                            predictor)
    
    prediction_summary.application  = application;
    prediction_summary.config       = config;
    prediction_summary.predictor    = predictor;
    
    workload_dir = sprintf('%s/%s/%s/%s', prdtst_dir, application, config, predictor);
    file_name_list = readdir(workload_dir);

    workload_exectime = [];
    predictor_error = [];
    predictor_overhead  = [];
    
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
        
        exec_times = raw(:, 1);
        %shift exec_times earlier by one so that the predictions and predicted 
        %values are aligned
        exec_times = exec_times(2:end);
        
        pred_valid = (raw(:, 2) == 1.0);
        predictions= raw(:, 3);
        overhead   = raw(:, 4);
        
        %remove the last rows since there is no corresponding predicted 
        %value
        predictions= predictions(1:end-1);
        pred_valid = pred_valid(1:end-1);
        overhead   = overhead(1:end-1);
        
        %extract the valid rows;
        exec_times = exec_times(pred_valid);
        predictions= predictions(pred_valid);
        overhead   = overhead(pred_valid);
        
        %compute the prediction error
        local_pred_error = exec_times - predictions;
        
        predictor_error = [predictor_error; local_pred_error];
        predictor_overhead  = [predictor_overhead; overhead];
        workload_exectime = [workload_exectime; exec_times];
    end

    size(predictor_error);
    
    prediction_summary.rms_error = sqrt(mean( predictor_error .* predictor_error ));
    prediction_summary.rms_exectimes = sqrt(mean( workload_exectime .* workload_exectime ));
    prediction_summary.var_exectimes = std(workload_exectime);
    prediction_summary.mean_overhead = mean(predictor_overhead);
    prediction_summary.p99_overhead = prctile(predictor_overhead, 99);
end

