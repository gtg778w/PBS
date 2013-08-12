
    datadir = '../data/VICtimer/';
    
    test_length =    10000;
    intervals = [100000, 200000, 500000, 1000000, 2000000, 5000000];
    plot_fmts = {'r', 'o', 'g', 'b', 'm', 'k'};
    frequency_config =  '1200000';
    
    fig1_h = figure();
    hold on
    fig2_h = figure();
    hold on;
    for interval_i = 1:length(intervals)
    
        interval = intervals(interval_i);
        plot_fmt = plot_fmts{interval_i};
    
        file_name = sprintf('%s/VICtimer_response_test_%i_%i_%s.csv', datadir, ...
                            test_length, interval, frequency_config);
                            
        [VIC_interval, VIC_error, ns_error] = ...
            parse_VICtimer_test_data(file_name);
        
        %VIC_error   = VIC_error ./ VIC_interval;
        
        N = length(VIC_error);
        i_1p = round(N * 1 / 100);
        i_99p = round(N * 99 / 100);
        
        [Ve, F_Ve] = myCDF(VIC_error);
        Ve = Ve(i_1p:i_99p);
        F_Ve = F_Ve(i_1p:i_99p);
        figure(fig1_h);
        plot(Ve, F_Ve, plot_fmt);
        
        [Ne, F_Ne] = myCDF(ns_error);
        Ne = Ne(i_1p:i_99p);
        F_Ne = F_Ne(i_1p:i_99p);
        figure(fig2_h);
        plot(Ne, F_Ne, plot_fmt);
    end
    
    figure(fig1_h);
    hold off;
    
    figure(fig2_h);
    hold off;

