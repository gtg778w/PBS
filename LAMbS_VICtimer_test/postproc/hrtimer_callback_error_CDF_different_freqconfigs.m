
    datadir = '../data/hrtimer/';
    
    test_length =   10000;
    interval =  5000000;
    frequency_configs = {'1200000', '1900000', '2200000', '2301000', 'cycle'};
    plot_fmts = {'r', 'g', 'b', 'm', 'k'};
    
    fig2_h = figure();
    hold on;
    for frequency_config_i = 1:length(frequency_configs)
        
        frequency_config = frequency_configs{frequency_config_i};
        plot_fmt = plot_fmts{frequency_config_i};
        
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

