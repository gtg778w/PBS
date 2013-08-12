
    datadir = '../data/VICtimer/';
    
    test_length =    10000;
    intervals = {'100000', '200000', '500000', '1000000', '2000000', '5000000'};
    plot_fmts = {'r', 'o', 'g', 'b', 'm', 'k'};
    frequency_config =  '1200000';
    
    N = length(frequency_configs);
    min_VIC_error_array = zeros([1, N]);
    max_VIC_error_array = zeros([1, N]);
    
    min_ns_error_array  = zeros([1, N]);
    max_ns_error_array  = zeros([1, N]);
    
    for interval_i = 1:length(intervals)
    
        interval = intervals{interval_i};
        plot_fmt = plot_fmts{interval_i};
    
        file_name = sprintf('%s/VICtimer_response_test_%i_%s_%s.csv', datadir, ...
                            test_length, interval, frequency_config);
                            
        [VIC_interval, VIC_error, ns_error] = ...
            parse_VICtimer_test_data(file_name);
        
        min_VIC_error = min(VIC_error);
        max_VIC_error = max(VIC_error);

        min_ns_error  = min(ns_error);
        max_ns_error  = max(ns_error);
        
        min_VIC_error_array(interval_i) = min_VIC_error;
        max_VIC_error_array(interval_i) = max_VIC_error;
        
        min_ns_error_array(interval_i) = min_ns_error;
        max_ns_error_array(interval_i) = max_ns_error;
        
    end
    
    fig1_h = figure();
    bar1_h = bar(min_VIC_error_array);
    set(gca, 'xticklabelmode', 'manual');
    set(gca, 'xticklabel', {'', intervals{1:end}});
    
    fig2_h = figure();
    bar2_h = bar(max_VIC_error_array);
    set(gca, 'xticklabelmode', 'manual');
    set(gca, 'xticklabel', {'', intervals{1:end}});
    
    fig3_h = figure();
    bar3_h = bar(min_ns_error_array);
    set(gca, 'xticklabelmode', 'manual');
    set(gca, 'xticklabel', {'', intervals{1:end}});
    
    fig4_h = figure();
    bar4_h = bar(max_ns_error_array);
    set(gca, 'xticklabelmode', 'manual');
    set(gca, 'xticklabel', {'', intervals{1:end}});
    
