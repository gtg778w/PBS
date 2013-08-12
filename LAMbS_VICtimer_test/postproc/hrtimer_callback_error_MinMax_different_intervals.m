
    datadir = '../data/hrtimer/';
    
    test_length =    10000;
    intervals = {'100000', '200000', '500000', '1000000', '2000000', '5000000'};
    plot_fmts = {'r', 'o', 'g', 'b', 'm', 'k'};
    frequency_config =  '1200000';

    N = length(intervals);    
    min_ns_error_array  = zeros([1, N]);
    max_ns_error_array  = zeros([1, N]);
    
    for interval_i = 1:N
    
        interval = intervals{interval_i};
        plot_fmt = plot_fmts{interval_i};
    
        file_name = sprintf('%s/hrtimer_response_test_%i_%s_%s.csv', datadir, ...
                            test_length, interval, frequency_config);
                            
        [ns_interval, ns_error] = parse_hrtimer_test_data(file_name);
        
        min_ns_error  = min(ns_error);
        max_ns_error  = max(ns_error);

        min_ns_error_array(interval_i) = min_ns_error;
        max_ns_error_array(interval_i) = max_ns_error;

    end

    fig1_h = figure();
    bar1_h = bar(min_ns_error_array);
    set(gca, 'xticklabelmode', 'manual');
    set(gca, 'xticklabel', {'', intervals{1:end}});
    
    fig2_h = figure();
    bar2_h = bar(max_ns_error_array);
    set(gca, 'xticklabelmode', 'manual');
    set(gca, 'xticklabel', {'', intervals{1:end}});

