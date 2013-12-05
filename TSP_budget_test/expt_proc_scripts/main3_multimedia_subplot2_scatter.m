
    screen_size = get(0, 'ScreenSize');
    xsize = (screen_size(3)*0.9);
    ysize = (screen_size(3)/2.5);
    print_Soption = sprintf('-S%i,%i', xsize, ysize);
        
    root_dir = '../budget_data/ffmpeg/';
    workload_list = {   'dec.arthur.spx', ...
                        'dec.mozart.mp3', ...
                        'dec.sintel.mp4', ...
                        'enc.arthur.wav.amr', ...
                        'enc.beethoven.wav.mp3', ...
                        'enc.highway.y4m.mp4'};

    plotname_list = {   'speech_decode', ...
                        'audio_decode', ...
                        'video_decode', ...
                        'speech_encode', ...
                        'audio_encode', ...
                        'video_encode'};
    
    outfile_name = 'alphas.txt';
    fid = fopen(outfile_name, 'w');
    
    workload_count = 6;

    sizes  =    {[14], [14], [14]};
    colors =    {[0.5, 0, 0], [0, 0, 0.9], [0, 0, 0]};
    styles =    {'x', '+', 'o'};

    focus_a_list = {2, 2, 1};

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
        title('(a)', 'fontname', 'FreeSans', 'fontsize', 15);
        xlabel('miss rate', 'fontname', 'FreeSans', 'fontsize', 15);
        ylabel('Normalized Budget Allocation', 'fontname', 'FreeSans', 'fontsize', 15);
        h_a = gca;
        set(h_a, 'fontname', 'FreeSans');
        set(h_a, 'fontsize', 15);

        subplot(1, 2, 2);
        scatter_series_set( series_set.miss_rates, ...
                            series_set.RMS_VFT_error, ...
                            sizes, colors, styles);
        title('(b)', 'fontname', 'FreeSans', 'fontsize', 15);
        xlabel('miss rate', 'fontname', 'FreeSans', 'fontsize', 15);
        ylabel('Normalized RMS VFT error', 'fontname', 'FreeSans', 'fontsize', 15);
        legend(series_set.series_names);
        h_a = gca;
        set(h_a, 'fontname', 'FreeSans');
        set(h_a, 'fontsize', 15);
                
        print(fig_handle, plot_name, '-djpg', print_Soption);

    end
    
    fclose(fid);

