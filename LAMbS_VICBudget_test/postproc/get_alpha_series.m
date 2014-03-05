function [alpha_series] = get_alpha_series(alpha_folder)

    file_name_list = readdir(alpha_folder);    
    file_count = length(file_name_list);

    alpha_series = {};

    array_i = 1;
    for k = 1:file_count
        
        next_file_name = file_name_list{k};
        
        %check that it is a number
        if( ~isdigit(next_file_name(1)))
            %propper log files begin with digits
            %move on to the next file
            continue;            
        end
        
        log_dirname = [alpha_folder, '/', next_file_name];
        
        %check that it is a folder
        if( ~isdir(log_dirname))
            continue;
        end
        
        alpha_series(array_i) = get_SRTlogrepetition_summary(log_dirname);
        array_i = array_i + 1;
    end

end
