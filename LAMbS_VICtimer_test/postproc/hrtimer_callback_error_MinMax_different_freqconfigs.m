
    datadir = '../data/hrtimer/';
    
    test_length =   10000;
    interval =  5000000;
    frequency_configs = {'1200000', '1900000', '2200000', '2301000', 'cycle'};
    plot_fmts = {'r', 'g', 'b', 'm', 'k'};

    N = length(frequency_configs);    
    min_ns_error_array  = zeros([1, N]);
    max_ns_error_array  = zeros([1, N]);
    
    for frequency_config_i = 1:length(frequency_configs)
        
        frequency_config = frequency_configs{frequency_config_i};
        plot_fmt = plot_fmts{frequency_config_i};
        
        file_name = sprintf('%s/hrtimer_response_test_%i_%i_%s.csv', datadir, ...
                            test_length, interval, frequency_config);
                            
        [ns_interval, ns_error] = parse_hrtimer_test_data(file_name);
        
        min_ns_error  = min(ns_error);
        max_ns_error  = max(ns_error);

        min_ns_error_array(frequency_config_i) = min_ns_error;
        max_ns_error_array(frequency_config_i) = max_ns_error;

    end

    fig1_h = figure();
    bar1_h = bar(min_ns_error_array);
    set(gca, 'xticklabelmode', 'manual');
    set(gca, 'xticklabel', {'', frequency_configs{1:end}});
    
    fig2_h = figure();
    bar2_h = bar(max_ns_error_array);
    set(gca, 'xticklabelmode', 'manual');
    set(gca, 'xticklabel', {'', frequency_configs{1:end}});

