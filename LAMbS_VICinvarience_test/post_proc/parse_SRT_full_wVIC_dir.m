function [SRT_records] = parse_SRT_full_wVIC_dir(logdir)
    
    file_name_list = readdir(logdir);
    
    file_count = length(file_name_list);
    
    SRT_records = [];
    
    for i = 1:file_count
    
        next_file_name = file_name_list{i};
    
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
        
        full_file_name = sprintf('%s/%s', logdir, next_file_name);
        SRT_record = parse_csv_file(full_file_name);
        SRT_records = [SRT_records, SRT_record];
    end
    
    mat_name = [logdir, '.mat'];
    save('-v7', mat_name, 'SRT_records');
    
end

