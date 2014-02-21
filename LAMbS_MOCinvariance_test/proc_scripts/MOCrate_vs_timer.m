    %Get information on screen size and stuff
    screen_size = get(0, 'ScreenSize');
    xsize = screen_size(3)*0.5;
    ysize = screen_size(3)*0.15;
    print_Soption = sprintf('-S%i,%i', xsize, ysize);
    
    %Names of relevant directories and configurations
    data_rootdir = '../data2/';
    plot_dir = '../plots/set3';
    
    APPNAME     = 'ffmpeg';
    CONFIGNAME  = 'dec.sintelfull.720p.mkv';
    
    %Plot instruction-rate against time
    MOC = 'userinst';
    data_dir = sprintf('%s/%s/%s/%s',   data_rootdir,
                                        APPNAME,
                                        CONFIGNAME,
                                        MOC);
    
    filename = sprintf('%s/%s_%s_%s.csv', data_dir, 
                                          MOC, 
                                          CONFIGNAME,
                                          APPNAME);
                                          
    plotfile_name = sprintf('%s/MOCrate_%s_videodecode_%s.jpg', plot_dir, 
                                                                MOC,
                                                                APPNAME);
    
    raw_data1 = load(filename);
    time1 = raw_data1(:,1);
    userinst = raw_data1(:,2);
    [duserinst, ut1] = get_dbydt(userinst, time1);
    
    %setup the figure
    fig_handle = figure();
    set(fig_handle, 'Position', [0 0 xsize ysize] );
    ylabel('instructions/ns');
    xlabel('time (ns)');
    
    axis_handle = gca;
    set(axis_handle, 'fontsize', 14);
    set(get(axis_handle, 'xlabel'), 'fontsize', 12);
    set(get(axis_handle, 'ylabel'), 'fontsize', 12);
    set(axis_handle, 'position' , [0.15, 0.30, 0.80, 0.60]);

    hold on;
    
    %plot and format
    plot_handle = plot(ut1, duserinst);
    set(plot_handle, 'color', [0.4, 0.4, 0.4]);    
    hold off;
    
    %Output the plot to file
    print(fig_handle, plotfile_name, '-djpg', print_Soption);    
    
    
    %Plot cycle-rate against time
    MOC = 'cycl';
    data_dir = sprintf('%s/%s/%s/%s',   data_rootdir,
                                        APPNAME,
                                        CONFIGNAME,
                                        MOC);
    
    filename = sprintf('%s/%s_%s_%s.csv', data_dir, 
                                          MOC, 
                                          CONFIGNAME,
                                          APPNAME);

    plotfile_name = sprintf('%s/MOCrate_%s_videodecode_%s.jpg', plot_dir, 
                                                                MOC,
                                                                APPNAME);
    
    raw_data2 = load(filename);
    time2 = raw_data2(:,1);
    cycls = raw_data2(:,2);
    [dcycls, ut2] = get_dbydt(cycls, time2);
    
    %setup the figure
    fig_handle = figure();
    set(fig_handle, 'Position', [0 0 xsize ysize] );
    ylabel('cycles/ns');
    xlabel('time (ns)');
    
    axis_handle = gca;
    set(axis_handle, 'fontsize', 14);
    set(get(axis_handle, 'xlabel'), 'fontsize', 12);
    set(get(axis_handle, 'ylabel'), 'fontsize', 12);
    set(axis_handle, 'position' , [0.15, 0.30, 0.80, 0.60]);
    set(axis_handle, 'ytick', 2.291:0.001:2.296);
    
    hold on;
    
    %plot and format
    plot_handle = plot(ut2, dcycls);
    set(plot_handle, 'color', [0.4, 0.4, 0.4]);    
    hold off;
    
    %Output the plot to file
    print(fig_handle, plotfile_name, '-djpg', print_Soption);
    
