
    ffmpeg_sintel_dir = '../data/ffmpeg/dec.sintelfull.720p.mkv';
    figure_dir = [ffmpeg_sintel_dir, '/figures'];

    freqconstmed_ns_dir = [ffmpeg_sintel_dir, '/freqconstmed/ns'];
    aseries_freqconstmed_ns = get_alpha_series(freqconstmed_ns_dir);

    freqcycle_ns_dir = [ffmpeg_sintel_dir, '/freqcycle/ns'];
    aseries_freqcycle_ns = get_alpha_series(freqcycle_ns_dir);

    freqconstmed_VIC_dir = [ffmpeg_sintel_dir, '/freqconstmed/VIC'];
    aseries_freqconstmed_VIC = get_alpha_series(freqconstmed_VIC_dir);

    freqcycle_VIC_dir = [ffmpeg_sintel_dir, '/freqcycle/VIC'];
    aseries_freqcycle_VIC = get_alpha_series(freqcycle_VIC_dir);
    
    h = figure;
    plot_missrate_vs_budgetutil(aseries_freqconstmed_VIC, 'k', 8, 0.5);
    hold on;
    plot_missrate_vs_budgetutil(aseries_freqcycle_VIC, 'b', 8, 0.5);
    plot_missrate_vs_budgetutil(aseries_freqconstmed_ns, 'g', 8, 0.5);
    plot_missrate_vs_budgetutil(aseries_freqcycle_ns, 'r', 8, 0.5);
    hold off;
    xlabel('miss rate');
    ylabel('budget utilization');
    title('ffmpeg sintel missrate vs budget utilization');
    filename = [figure_dir, '/missrate_vs_util.jpg'];
    print(h, filename, '-djpg')';
    
    h = figure;
    plot_missrate_vs_normbudget(aseries_freqconstmed_VIC, 'k', 8, 0.5);
    hold on;
    plot_missrate_vs_normbudget(aseries_freqconstmed_ns, 'g', 8, 0.5);
    hold off;
    xlabel('miss rate');
    ylabel('average CPU bandwidth');
    title('ffmpeg sintel constant CPU frequency');
    filename = [figure_dir, '/missrate_vs_normbudget_freqconst.jpg'];
    print(h, filename, '-djpg')';
    
    h = figure;
    plot_missrate_vs_normbudget(aseries_freqcycle_VIC, 'b', 8, 0.5);
    hold on;
    plot_missrate_vs_normbudget(aseries_freqcycle_ns, 'r', 8, 0.5);
    hold off;
    xlabel('miss rate');
    ylabel('average CPU bandwidth');
    title('ffmpeg sintel switching CPU frequency');
    filename = [figure_dir, '/missrate_vs_normbudget_freqcycle.jpg'];
    print(h, filename, '-djpg')';
    
