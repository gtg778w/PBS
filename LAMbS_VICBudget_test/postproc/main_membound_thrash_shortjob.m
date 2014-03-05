
    membound_shortjob_dir = '../data/membound/thrash_shortjob/';
    figure_dir = [membound_shortjob_dir, '/figures'];
    alpha = '1.25';

    time_dir = sprintf('%s/oscil_freq/time/%s/', membound_shortjob_dir, alpha);
    %find the first CSV file in the directory
    file_name_list = readdir(time_dir);
    file_count = length(file_name_list);
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

        logname = [time_dir, '/', next_file_name];

        time_data = parse_csv_file(logname);
    end
    plot_VFT_error(time_data, 'CPU time');
    
    VIC_dir = sprintf('%s/oscil_freq/VIC/%s/', membound_shortjob_dir, alpha);
    %find the first CSV file in the directory
    file_name_list = readdir(VIC_dir);
    file_count = length(file_name_list);
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

        logname = [VIC_dir, '/', next_file_name];

        VIC_data = parse_csv_file(logname);
    end
    plot_VFT_error(VIC_data, 'VIC');

