
    ffmpeg_sintel_dir = '../data/ffmpeg/sintel';
    figure_dir = [ffmpeg_sintel_dir, '/figures'];

    maxfreq_time_dir = [ffmpeg_sintel_dir, '/max_freq/time'];
    aseries_maxfreq_time = get_alpha_series(maxfreq_time_dir);

    minfreq_time_dir = [ffmpeg_sintel_dir, '/min_freq/time'];
    aseries_minfreq_time = get_alpha_series(minfreq_time_dir);
    
    oscilfreq_time_dir = [ffmpeg_sintel_dir, '/oscil_freq/time'];
    aseries_oscilfreq_time = get_alpha_series(oscilfreq_time_dir);
    
    maxfreq_VIC_dir = [ffmpeg_sintel_dir, '/max_freq/VIC'];
    aseries_maxfreq_VIC = get_alpha_series(maxfreq_VIC_dir);

    minfreq_VIC_dir = [ffmpeg_sintel_dir, '/min_freq/VIC'];
    aseries_minfreq_VIC = get_alpha_series(minfreq_VIC_dir);
    
    oscilfreq_VIC_dir = [ffmpeg_sintel_dir, '/oscil_freq/VIC'];
    aseries_oscilfreq_VIC = get_alpha_series(oscilfreq_VIC_dir);
    
    figure;
    plot_missrate_vs_normbudget(aseries_maxfreq_time, 'k', 8, 0.5);
    title('ffmpeg sintel missrate vs budget utilization (maxfreq time)');
    figure;
    plot_missrate_vs_normbudget(aseries_minfreq_time, 'k', 8, 0.5);
    title('ffmpeg sintel missrate vs budget utilization (minfreq time)');
    figure;
    plot_missrate_vs_normbudget(aseries_oscilfreq_time, 'k', 8, 0.5);
    title('ffmpeg sintel missrate vs budget utilization (oscilfreq time)');

    figure;
    plot_missrate_vs_normbudget(aseries_maxfreq_VIC, 'k', 8, 0.5);
    title('ffmpeg sintel missrate vs budget utilization (maxfreq VIC)');
    figure;
    plot_missrate_vs_normbudget(aseries_minfreq_VIC, 'k', 8, 0.5);
    title('ffmpeg sintel missrate vs budget utilization (minfreq VIC)');
    figure;
    plot_missrate_vs_normbudget(aseries_oscilfreq_VIC, 'k', 8, 0.5);
    title('ffmpeg sintel missrate vs budget utilization (oscilfreq VIC)');    

