function [SRT_records] = process_SRT_rundir(dir_name)

    %get the list of files in the directory
    dir_filelist    = readdir(dir_name);
    dir_filecount   = length(dir_filelist);
    
    %initialize the output variable
    SRT_records = struct();
    
    %repetitions index
    r = 0;
    %SRT_records.pid = zeros([1, dir_filecount]);
    SRT_records.period  = zeros([1, dir_filecount]);
    SRT_records.reservation_period  = zeros([1, dir_filecount]);
    SRT_records.budget_type = zeros([1, dir_filecount]);
    SRT_records.estimated_mean_exectime = zeros([1, dir_filecount]);
    SRT_records.job_count   = zeros([1, dir_filecount]);
    SRT_records.allocated_budget= zeros([1, dir_filecount]);
    SRT_records.consumed_budget_ns  = zeros([1, dir_filecount]);
    SRT_records.consumed_budget_VIC = zeros([1, dir_filecount]);
    SRT_records.total_misses    = zeros([1, dir_filecount]);
    SRT_records.total_budget_capacity   = zeros([1, dir_filecount]);
    SRT_records.RMS_VFT_error   = zeros([1, dir_filecount]);
    
    %loop through each file in the root directory
    for f_i = 1:dir_filecount
    
        file_name = dir_filelist{f_i};
    
        file_path = sprintf('%s/%s', dir_name, file_name);
        
        %check if it is a file
        if isdir(file_path)
            %its a directory. move onto the next file
            continue;
        end
        
        r = r+1;
        
        raw_mat = csvread(file_path);
        
        %SRT_records.pid(r) = raw_mat(1, 1);
        SRT_records.period(r) = raw_mat(1, 2);
        SRT_records.reservation_period(r) = raw_mat(1, 3);
        SRT_records.budget_type(r) = raw_mat(1, 4);
        SRT_records.estimated_mean_exectime(r) = raw_mat(1, 5);
        SRT_records.job_count(r) = raw_mat(1, 6);
        SRT_records.allocated_budget(r) = raw_mat(1, 7);
        SRT_records.consumed_budget_ns(r) = raw_mat(1, 8);
        SRT_records.consumed_budget_VIC(r) = raw_mat(1, 9);
        SRT_records.total_misses(r) = raw_mat(1, 10);
        SRT_records.total_budget_capacity(r) = raw_mat(1, 11);
        
        %Skip the first two rows
        raw_mat = raw_mat(3:end,:);
        
        abs_release_time = raw_mat(:, 4);
        abs_LFT = raw_mat(:, 5);
        LFT = abs_LFT - abs_release_time;
        budget_allocated = raw_mat(:, 6);
        budget_used = raw_mat(:, 7);
        budget_remaining = budget_allocated - budget_used;
        VFT = LFT .- ((budget_remaining ./ budget_allocated) * SRT_records.reservation_period(r));
        VFT_error = VFT - SRT_records.period(r);
        
        SRT_records.RMS_VFT_error(r) = sqrt(mean(VFT_error .* VFT_error));
    end

    %SRT_records.pid = SRT_records.pid(1:r);
    SRT_records.period      = SRT_records.period(1);
    SRT_records.reservation_period  = SRT_records.reservation_period(1);
    SRT_records.budget_type = SRT_records.budget_type(1);
    SRT_records.estimated_mean_exectime = SRT_records.estimated_mean_exectime(1:r);
    SRT_records.job_count   = SRT_records.job_count(1);
    SRT_records.allocated_budget    = SRT_records.allocated_budget(1:r);
    SRT_records.consumed_budget_ns  = SRT_records.consumed_budget_ns(1:r);
    SRT_records.consumed_budget_VIC = SRT_records.consumed_budget_VIC(1:r);
    SRT_records.total_misses= SRT_records.total_misses(1:r);
    SRT_records.total_budget_capacity   = SRT_records.total_budget_capacity(1:r);
    SRT_records.RMS_VFT_error       = SRT_records.RMS_VFT_error(1:r);
end
