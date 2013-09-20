function [fcfreq_series] = get_fcfreq_series(fcfreq_folder)

    file_name_list = readdir(fcfreq_folder);    
    file_count = length(file_name_list);

    fcfreq_series = {};

    array_i = 1;
    for k = 1:file_count
        
        next_file_name = file_name_list{k};
        
        %check that it is a number
        if( ~isdigit(next_file_name(1)))
            %propper log files begin with digits
            %move on to the next file
            continue;            
        end
        
        log_dirname = [fcfreq_folder, '/', next_file_name];

        %check that it is a folder
        if( ~isdir(log_dirname))
            continue;
        end
        
        fcfreq_series(array_i) = get_SRTlogrepetition_summary(log_dirname);
        fcfreq_series{array_i}.fcfreq = str2num(next_file_name);
        array_i = array_i + 1;
    end

end
