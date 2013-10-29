%The purpose of this script is to generate a single csv file containing the rms error
%associated with each of the predictors for each of the workloads

    report_name = '../prdtst_reports/ful_prdtst_report.csv';
    fid = fopen(report_name, 'w');

    fprintf(fid, 'application.config,\t mean,\t var,\t predictor,\t overhead,\t rms error\n');

    timing_summary_dir = '../timing_data_summary/';
    prdtst_summary_dir = '../prdtst_data_summary';

    rootdir_filelist = readdir(prdtst_summary_dir);
    rootdir_filecount= length(rootdir_filelist);
    
    %loop through each file in the root directory
    for d1 = 1:rootdir_filecount
    
        %get the file name
        subdir_name = rootdir_filelist{d1};
        subdir_path = sprintf('%s/%s', prdtst_summary_dir, subdir_name);
        
        %check if it is a directory
        if ~ isdir(subdir_path)
            %not a directory. move onto the next file
            continue;
        end
        
        %make sure the directory is not . or ..
        if strcmp(subdir_name, '.') || strcmp(subdir_name, '..')
            %if it is either the . or .. directory, move onto the next file
            continue;
        end
        
        %each valid directory is the name of an application
        application = subdir_name;

        %Get the path name for the same application in the timing summary directory
        timing_subdir_path = sprintf('%s/%s', timing_summary_dir, subdir_name);
        
        config_filelist = readdir(subdir_path);
        config_filecount= length(config_filelist);
        %loop through each file in the subdirectory
        for d2 = 1:config_filecount
        
            %get the file name
            subsubdir_name = config_filelist{d2};
            subsubdir_path = sprintf('%s/%s', subdir_path, subsubdir_name);

            %check if it is a directory
            if ~ isdir(subsubdir_path)
                %not a directory. move onto the next file
                continue;
            end

            %make sure the directory is not . or ..
            if strcmp(subsubdir_name, '.') || strcmp(subsubdir_name, '..')
                %if it is either the . or .. directory, move onto the next file
                continue
            end

            %each valid directory is the name of a configuration
            config = subsubdir_name;

            %Get the path name for the same application in the timing summary directory
            timing_file_name = sprintf('%s/%s.%s.mat',  timing_subdir_path, ...
                                                        application, ...
                                                        config);
            load(timing_file_name);
            timing_summary = summary;
            fprintf(fid, '%s.%s,\t %.3e,\t %.3e\n', application, config, ...
                                                    timing_summary.mean, ...
                                                    timing_summary.std);
                        
            predictor_filelist = readdir(subsubdir_path);
            predictor_filecount= length(predictor_filelist);
            for d3 = 1:predictor_filecount
            
                predsummary_name  = predictor_filelist{d3};
                predsummary_path = sprintf('%s/%s', subsubdir_path, predsummary_name);
                
                %check if it is a directory
                if isdir(predsummary_path)
                    %is a directory. we are now looking for files
                    continue;
                end

                %Check if the file name is long enough to do the remaining tests without errors
                if(length(predsummary_name) < 4)
                    %The file name is not long enough
                    %move on to the next file
                    continue;            
                end
            
                %Check if the next file has a '.csv' suffix
                if(1 ~= strcmp(predsummary_name(end-3:end), '.mat'))
                    %file needs to end in the ".csv" suffix
                    %move on to the next file
                    continue;
                end
                
                load(predsummary_path);
                prediction_summary = summary;
                fprintf(fid, ',\t ,\t ,\t %s,\t %.3e,\t %.3e\n', 
                            prediction_summary.predictor, ...
                            prediction_summary.p99_overhead, ...
                            prediction_summary.rms_error);
            end
        end
    end
    
    fclose(fid);
    
