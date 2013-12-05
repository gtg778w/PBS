
    screen_size = get(0, 'ScreenSize');
    xsize = (screen_size(3)*0.9);
    ysize = (screen_size(3)/2.5);
    print_Soption = sprintf('-S%i,%i', xsize, ysize);
        
    plotname    = 'sqrwav_exectime';
    workload_dir= '../budget_data/sqrwav/example';
    
    pred_list = {'ma', 'mabank', 'mavslmsbank'};
    alpha_list = {  '0.90902', ...
                    '0.90902', ...
                    '0.85044'};
    r_list = [5, 5, 5];

    for s_i = 1:length(alpha_list)
        rep_dir = sprintf('%s/%s/%s', workload_dir, pred_list{s_i}, alpha_list{s_i});
        file_list = readdir(rep_dir);
        
        file_path = sprintf('%s/%s', rep_dir, file_list{r_list(s_i)});            
            
        figure;
        plot_exectime(file_path, [240, 1500]);
        title(pred_list{s_i});
        figure;
        plot_VFTerror(file_path, [240, 1500]);
        title(pred_list{s_i});
    end

