
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
    plot_dir = '../plots/set1';
    
    application_array = {'membound', 'membound', 'ffmpeg'};
    configuration_array = {'cacheline', 'thrash', 'dec.sintelfull.720p.mkv'};
    outputplot_name = {'membound_cacheline', 'membound_thrash', 'ffmpeg_videodecode'};
    
    MOC_array = {'nsec', 'cycl', 'inst', 'userinst', 'VIC'};
    ylabel_array = {'CPU time (ns)', 'Clock cycles', 'RIC', 'URIC', 'VIC'};

    %Get information on screen size and stuff
    screen_size = get(0, 'ScreenSize');
    xsize = screen_size(3)*0.5;
    ysize = screen_size(3)*0.20;
    print_Soption = sprintf('-S%i,%i', xsize, ysize);
    
    %Iterate through the applications and configuration
    for a = 1:3
        
        application = application_array{a};
        configuration = configuration_array{a};
        
        app_dir = sprintf('%s/%s/%s/', data_dir, application, configuration);
        
        %Iterate through the measures of computation (MOC) directories in this application/configuration directory
        for m = 1:length(MOC_array);
            
            MOC = MOC_array{m};
            MOC_dir = sprintf('%s/%s', app_dir, MOC);
            
            %initialize the array of data points
            freq_array = [];
            CPUcapacity_array = [];
            x_ideal = [];
            y_ideal = [];
            x_bestfit = [];
            y_bestfit = [];
            
            %get the list of files in the MOC directory
            freq_filelist    = readdir(MOC_dir);
            freq_filecount   = length(freq_filelist);
            
            %Iterate through the frequency directories in this MOC directory.
            freq_count = 0;
            freq_names = {};
            for f = 1:freq_filecount
                
                %get the file name
                freq_name = freq_filelist{f};
                
                %get the file path
                freq_path = sprintf('%s/%s', MOC_dir, freq_name);
                
                %verify that the file is indeed a frequency file
                %check if it is a directory
                if ~ isdir(freq_path)
                    %its not a directory. move onto the next entry
                    continue;
                end
                
                %check if it is the current directory or parent directory
                if( strcmp(freq_name, '.') || strcmp(freq_name, '..'))
                    %its the '.' or '..' directory. move on to the next rnt
                    continue;
                end
        
                %add to the list of valid known frequencies
                freq_count = freq_count + 1;
                freq_names{freq_count} = freq_name;
                
                %open the directory associated with the frequency
                rep_filelist = readdir(freq_path);
                rep_filecount= length(rep_filelist);
                
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
                    mean_consumed_capacity = mean(raw);
                    
                    %Append the computed mean and the frequency to the list of existing data points
                    CPUcapacity_array = [CPUcapacity_array, mean_consumed_capacity];
                    freq_array = [freq_array, str2num(freq_name)];
                    
                end %rep loop
            end %freq loop
            
            %Disregard the max frequency data point due to the turbo boost madness
            max_freq = max(freq_array);
            max_freq_loc = (freq_array == max_freq);
            other_loc = ~max_freq_loc;
            freq_array = freq_array(other_loc);
            CPUcapacity_array = CPUcapacity_array(other_loc);
            
            %Change the units of CPU clock frequency to GHz
            freq_array= freq_array /1000000;
            
            %Determine the actual and ideal best-fit line through the data using
            %simple linear regression.
            if(strcmp(MOC, 'nsec'))
                x = 1 ./ freq_array;
            else
                x = freq_array;
            end
            y = CPUcapacity_array;
            xy_mean = mean(y .* x);
            y_mean  = mean(y);
            x_mean  = mean(x);
            xy_cov  = xy_mean - x_mean * y_mean;
            x_var   = var(x);
            beta_hat= xy_cov/x_var;
            alpha_hat   = y_mean - beta_hat * x_mean;
            x_hat = min(freq_array):(max(freq_array) - min(freq_array))/100:max(freq_array);
            if(strcmp(MOC, 'nsec'))
                y_hat   = alpha_hat + beta_hat ./ x_hat;
                y_ideal = (y_hat(1) * x_hat(1)) ./ x_hat;
            else
                y_hat   = alpha_hat + beta_hat * x_hat;
                y_ideal = y_hat(1) * ones(size(x_hat));
            end
            
            %determine the range of the data
            x_max = max(freq_array);
            x_min = min(freq_array);
            x_range =   x_max - x_min;
            x_max = x_max + 0.05 * x_range;
            x_min = x_min - 0.05 * x_range;
            x_range =   x_max - x_min;
            x_ticks =   x_min : x_range/3 : x_max;
            
            y_max = max([max(CPUcapacity_array), max(y_ideal), max(y)]);
            y_min = min([min(CPUcapacity_array), min(y_ideal), min(y)]);
            y_range =   y_max - y_min;
            y_max = y_max + 0.05 * y_range;
            y_min = y_min - 0.05 * y_range;
            y_range =   y_max - y_min;
            y_ticks =   y_min : y_range/3 : y_max;

            
            %setup the figure
            fig_handle = figure();
            set(fig_handle, 'Position', [0 0 xsize ysize] );
            axis([x_min, x_max, y_min, y_max]);
            xlabel('CPU frequency (GHz)');
            ylabel(ylabel_array{m});

            axis_handle = gca;
            set(axis_handle, 'fontsize', 18);
            set(axis_handle, 'xtick', x_ticks);
            set(axis_handle, 'ytick', y_ticks);
            set(axis_handle, 'position' , [0.25, 0.25, 0.70, 0.70]);

            %Draw the actual diagram
            hold on;

            %Draw the ideal best-fit line
            ideal_plot_handle = plot(x_hat, y_ideal);
            set(ideal_plot_handle, 'linewidth', 1.5);
            set(ideal_plot_handle, 'color', [0.5, 0.5, 0.5]);
            
            %Draw the actual best-fit line
            fit_plot_handle = plot(x_hat, y_hat);
            set(fit_plot_handle, 'linestyle', '-');
            set(fit_plot_handle, 'linewidth', 2.5);
            set(fit_plot_handle, 'color', [0, 0, 0]);            
            
            %Scatter the data points
            plot_handle = plot(freq_array, CPUcapacity_array);
            set(plot_handle, 'linestyle', 'none');
            set(plot_handle, 'marker', '+');
            set(plot_handle, 'markersize', 12);
            set(plot_handle, 'color', [0, 0, 0]);

            hold off;
                                    
            %Generate a name for the image that will store the plot
            plot_name = sprintf('%s/consumed_%s_%s.jpg',plot_dir, ...
                                                        outputplot_name{a}, ...
                                                        MOC);
            
            %Print the figure to a jpeg image
            print(fig_handle, plot_name, '-djpg', print_Soption);
            
            %close the figure
            close(fig_handle);
            
        end %MOC loop
    end %application/configuration loop
    
