
    figure;
    root_dir = '../budget_data/ffmpeg/';
    workload_list = {   'dec.arthur.spx', ...
                        'dec.mozart.mp3', ...
                        'dec.sintel.mp4', ...
                        'enc.arthur.wav.amr', ...
                        'enc.beethoven.wav.mp3', ...
                        'enc.highway.y4m.mp4'};
    
    workload_count = 6;
    
    colors = {[1.0, 0, 0], [0, 0, 1.0], [0, 0, 0]};
        
    for w_i = 1:workload_count
    
        workload_dir = sprintf('%s/%s', root_dir, workload_list{w_i});
        series_set = get_series_set(workload_dir);
        
        subplot(2, 3, w_i);
        [handles] = scatter_series_set( series_set.series_names, ...
                                        series_set.miss_rates, ...
                                        series_set.allocated_budget, ...
                                        colors);
        title(workload_list{w_i});
    end

