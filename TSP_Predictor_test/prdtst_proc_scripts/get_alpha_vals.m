function [alpha_vals] = get_alpha_vals( prdtst_dir, ...
                                        application, ...
                                        config, ...
                                        predictor, ...
                                        miss_rates)
    
    predictor_error = [];
    
    workload_dir = sprintf('%s/%s/%s/%s', prdtst_dir, application, config, predictor);
    file_name_list = readdir(workload_dir);
    
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
        
        %remove the last rows since there is no corresponding predicted 
        %value
        predictions= predictions(1:end-1);
        pred_valid = pred_valid(1:end-1);
        
        %extract the valid rows;
        exec_times = exec_times(pred_valid);
        predictions= predictions(pred_valid);
        
        %compute the prediction error
        local_pred_error = exec_times - predictions;
        
        predictor_error = [predictor_error; local_pred_error];
    end

    size(predictor_error);

    std_pred_error = std(predictor_error);
    mean_pred_error = mean(predictor_error);

    hit_rates = 100 - miss_rates;
    hit_ptiles = prctile(predictor_error, hit_rates);

    alpha_vals = (hit_ptiles - mean_pred_error)/std_pred_error;
    
end

