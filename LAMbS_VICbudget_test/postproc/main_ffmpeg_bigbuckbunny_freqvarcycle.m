    
    ffmpeg_sintel_dir = '../data/ffmpeg/dec.bigbuckbunnyfull.480p.mov';
    figure_dir = [ffmpeg_sintel_dir, '/figures'];

    freqvarcycle_ns_dir = [ffmpeg_sintel_dir, '/freqvarcycle/ns'];
    fseries_freqvarcycle_ns = get_fcfreq_series(freqvarcycle_ns_dir);

    freqvarcycle_VIC_dir = [ffmpeg_sintel_dir, '/freqvarcycle/VIC'];
    fseries_freqvarcycle_VIC = get_fcfreq_series(freqvarcycle_VIC_dir);
        
    h = figure;
    plot_fcfreq_vs_NRMSprederror(fseries_freqvarcycle_VIC, 'b');
    hold on;
    plot_fcfreq_vs_NRMSprederror(fseries_freqvarcycle_ns, 'r');
    hold off;
    xlabel('period of periodic clock-frequency switch');
    ylabel('normalized root mean squared prediction error');
    title('ffmpeg bigbuckbunny');
    filename = [figure_dir, '/fcfreq_vs_NRMSprederror.jpg'];
    print(h, filename, '-djpg')';
    
