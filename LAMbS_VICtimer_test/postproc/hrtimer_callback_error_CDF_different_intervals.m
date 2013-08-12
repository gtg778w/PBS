
    datadir = '../data/hrtimer/';
    
    test_length =    10000;
    intervals = [100000, 200000, 500000, 1000000, 2000000, 5000000];
    plot_fmts = {'r', 'o', 'g', 'b', 'm', 'k'};
    frequency_config =  '1200000';
    
    fig1_h = figure();
    hold on
    for interval_i = 1:length(intervals)
    
        interval = intervals(interval_i);
        plot_fmt = plot_fmts{interval_i};
    
        file_name = sprintf('%s/hrtimer_response_test_%i_%i_%s.csv', datadir, ...
                            test_length, interval, frequency_config);
                            
        [ns_interval, ns_error] = parse_hrtimer_test_data(file_name);
        
        N = length(ns_error);
        i_1p = round(N * 1 / 100);
        i_99p = round(N * 99 / 100);
        
        [Ne, F_Ne] = myCDF(ns_error);
        Ne = Ne(i_1p:i_99p);
        F_Ne = F_Ne(i_1p:i_99p);
        plot(Ne, F_Ne, plot_fmt);
    end
    hold off;

