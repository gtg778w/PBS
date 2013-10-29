
    prdtst_summary_dir = '../prdtst_data_summary/';
    mkdir(prdtst_summary_dir);

    prdtst_dir = '../prdtst_data/'
    rootdir_filelist = readdir(prdtst_dir);
    rootdir_filecount= length(rootdir_filelist);
    
    %loop through each file in the root directory
    for d1 = 1:rootdir_filecount
            
        %get the file name
        subdir_name = rootdir_filelist{d1};
        subdir_path = sprintf('%s/%s', prdtst_dir, subdir_name);
        
        %check if it is a directory
        if ~ isdir(subdir_path)
            %not a directory. move onto the next file
            'not a dir'
            continue;
        end
        
        %make sure the directory is not . or ..
        if strcmp(subdir_name, '.') || strcmp(subdir_name, '..')
            %if it is either the . or .. directory, move onto the next file
            subdir_name
            continue;
        end
        
        %each valid directory is the name of an application
        application = subdir_name
        
        %make a directory to store the output
        application_summary_dir = sprintf('%s/%s', prdtst_summary_dir, application);
        mkdir(application_summary_dir);
        
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
            config = subsubdir_name
            
            %make a directory to store the output
            config_summary_dir = sprintf('%s/%s', application_summary_dir, config);
            mkdir(config_summary_dir);
            
            predictor_filelist = readdir(subsubdir_path);
            predictor_filecount= length(predictor_filelist);
            
            for d3 = 1:predictor_filecount
            
                subsubsubdir_name = predictor_filelist{d3};
                subsubsubdir_path = sprintf('%s/%s', subsubdir_path, subsubsubdir_name);
                
                %check if it is a directory
                if ~ isdir(subsubsubdir_path)
                    %not a directory. move onto the next file
                    continue;
                end

                %make sure the directory is not . or ..
                if strcmp(subsubsubdir_name, '.') || strcmp(subsubsubdir_name, '..')
                    %if it is either the . or .. directory, move onto the next file
                    continue
                end
                
                %each valid directory is the name of a predictor
                predictor = subsubsubdir_name;
                
                %get summarized data on the outputs of the predictors
                summary = process_predictor_outputs(  prdtst_dir, ...
                                                      application, ...
                                                      config, ...
                                                      predictor);
                                                      
                %set the output file name
                outfile_name = sprintf('%s/%s.%s.%s.mat',   config_summary_dir, ...
                                                            application, ...
                                                            config, ...
                                                            predictor);
                
                %save the summary structure to the output file name
                save(outfile_name, 'summary');                
            end
        end
    end

