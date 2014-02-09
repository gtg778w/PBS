
    %for each workload
        %for each measure of computation
        
            %initialize an empty array
            %for each CPU frequency
            
                %for each repetition

                    %load the CSV file
                    %average the consumed CPU capacity
                    %store the average value in the array
                %end
            %end
            
            %create a figure of a specific size
            %plot a scatter diagram of the average consumed CPU capacity against CPU frequency with formatted color, shape, and size
            %set tick positions
            %set axis labels
        %end
    %end
    
    %Names of relevant directories and configurations
    data_dir = '../data';
    plot_dir = '../plots/set2';
    
    application = 'membound';
    configuration_array = {'cacheline', 'L1cache', 'L2cache', 'L3cache', 'thrash'};
    freq_name   = '2300000';
    
    MOC_array = {'nsec', 'userinst'};
    ylabel_array = {'Execution time (ns)', 'URIC'};

    %Get information on screen size and stuff
    screen_size = get(0, 'ScreenSize');
    xsize = (screen_size(3)*0.5);
    ysize = screen_size(3)*0.20;
    print_Soption = sprintf('-S%i,%i', xsize, ysize*0.70);
    
    %Iterate through the measures of computation (MOC) directories in this application/configuration directory
    for m = 1:length(MOC_array);
            
        MOC = MOC_array{m};

        %initialize the array of data points
        c_array = [];
        CPUcapacity_array   = [];
        CPUcapacity_array_max = [];
        CPUcapacity_array_min = [];
        
        %Iterate through the applications and configuration
        for c = 1:length(configuration_array)
            
            configuration = configuration_array{c};
            
            app_dir = sprintf('%s/%s/%s/', data_dir, application, configuration);
            MOC_dir = sprintf('%s/%s', app_dir, MOC);
            freq_path = sprintf('%s/%s', MOC_dir, freq_name);
                                        
            %open the directory associated with the frequency
            rep_filelist = readdir(freq_path);
            rep_filecount= length(rep_filelist);
            
            raw_data_array = [];
            %iterate through the data files in this frequency directory
            for r = 1:rep_filecount
                
                %get the path to the file
                rep_filename = rep_filelist{r};
                rep_path = sprintf('%s/%s', freq_path, rep_filename);
                
                %check that it is a file and not a directory
                if( isdir(rep_path) )
                    %this is not a file
                    continue;
                end
                
                %check that it is a csv file
                if (1 ~= strcmp(rep_filename(end-3:end), '.csv'))
                    %this is not a csv file
                    continue;
                end
                
                %Read the raw data and compute the mean
                raw = load(rep_path);
                raw_data_array = [raw_data_array; raw];                
            end %rep loop
            
            mean_consumed_capacity = mean(raw_data_array);
            
            %Append the computed mean and the frequency to the list of existing data points
            CPUcapacity_array = [CPUcapacity_array, mean_consumed_capacity];
            c_array = [c_array, c];
        end %freq loop
            
        %setup the figure
        fig_handle = figure();
        set(fig_handle, 'Position', [0 0 xsize (ysize*0.5)] );
        ylabel(ylabel_array{m});

        %Draw the actual diagram
        hold on;
        
        plot_handle = bar( (c_array*2), CPUcapacity_array);
        set(plot_handle, 'edgecolor', [0.05, 0.05, 0.05]);
        set(plot_handle, 'facecolor', [0.5, 0.5, 0.5]);

        axis_handle = gca;
        %set(axis_handle, 'fontsize', 18);
        set(axis_handle,  'xtick', 2:2:10);
        set(axis_handle,  'xticklabel', configuration_array)
        %set(axis_handle, 'ytick', y_ticks);
        set(axis_handle, 'position' , [0.20, 0.20, 0.75, 0.70]);

        %set(plot_handle, 'barwidth', 0.8);        
        hold off;
        
        %Generate a name for the image that will store the plot
        plot_name = sprintf('%s/consumed_%s.jpg',plot_dir, ...
                                                 MOC);
        
        %Print the figure to a jpeg image
        print(fig_handle, plot_name, '-djpg', print_Soption);
        
        %close the figure
        close(fig_handle);
            
    end %MOC loop
    
