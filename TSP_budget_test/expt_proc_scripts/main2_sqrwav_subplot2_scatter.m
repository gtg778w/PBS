
    screen_size = get(0, 'ScreenSize');
    xsize = (screen_size(3)*0.9);
    ysize = (screen_size(3)/2.5);
    print_Soption = sprintf('-S%i,%i', xsize, ysize);
        
    root_dir = '../budget_data/sqrwav/';
    workload_list = {   'example'};

    plotname_list = {   'sqrwav'};
    
    outfile_name = 'alphas.txt';
    fid = fopen(outfile_name, 'w');
    
    workload_count = 1;

    sizes  =    {[12], [12], [12]};
    colors =    {[0.5, 0, 0], [0, 0, 0.9], [0, 0, 0]};
    styles =    {'x', '+', 'o'};
        
    for w_i = 1:workload_count
    
        workload_dir = sprintf('%s/%s', root_dir, workload_list{w_i});
        series_set = get_series_set(workload_dir);

        fprintf(fid, '%s: \n', disp(workload_list{w_i}));
        for s_i = 1:length(series_set.series_names)
            fprintf(fid, '\t%s: \n', series_set.series_names{s_i});
            
            [alphas, sort_i]= sort(series_set.alphas{s_i});
            mean_miss_rates = series_set.mean_miss_rates{s_i}(sort_i);
            budget          = series_set.mean_allocated_budget{s_i}(sort_i);
            
            for a_i = 1:length(series_set.alphas{s_i})
                fprintf(fid, '\t%f,\t%f,\t%f\n',alphas(a_i), ...
                                                mean_miss_rates(a_i), ...
                                                budget(a_i));
            end
        end

        fig_handle = figure;
        set(fig_handle, 'Position', [0 0 xsize ysize ] );
        plot_name = sprintf('../plots/scatter_%s.jpg', plotname_list{w_i});

        subplot(1, 2, 1);
        scatter_series_set( series_set.miss_rates, ...
                            series_set.normalized_allocated_budget, ...
                            sizes, colors, styles);
        title('(a)');
        xlabel('miss rate');
        ylabel('Mean Per-job Budget (ns)');
        
        subplot(1, 2, 2);
        scatter_series_set( series_set.miss_rates, ...
                            series_set.RMS_VFT_error, ...
                            sizes, colors, styles);
        title('(b)');
        xlabel('miss rate');
        ylabel('Normalized RMS VFT error');
        
        print(fig_handle, plot_name, '-djpg', print_Soption);
    end
    
    fclose(fid);

