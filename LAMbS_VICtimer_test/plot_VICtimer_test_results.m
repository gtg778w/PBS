function [] = plot_VICtimer_test_results(results_name)

    csv_filename = sprintf('%s.csv', results_name);
    
    raw_data = csvread(csv_filename);
    
    target_VIC = raw_data(:,2);
    callback_VIC = raw_data(:,3);
    VIC_error = target_VIC - callback_VIC;
    [x_boundaries, fbar] = discretized_pdf(VIC_error(1:end), 100)
    figure(1);
    bar_vw(x_boundaries, fbar);
    axis([-10000, 10000, -0.0005, 0.002]);
    
    target_ns = raw_data(:,5);
    callback_ns = raw_data(:,6);
    ns_error = target_ns - callback_ns;
    [x_boundaries, fbar] = discretized_pdf(ns_error(1:end), 100)
    figure(2);
    bar_vw(x_boundaries, fbar);
    axis([-10000, 100, -0.0005, 0.002]);
    
    LAMbS_ns_threshold = 3000;
    LAMbS_FP_gain = 2 ^ 48;
    
    inst_retrate_inv = raw_data(:,8) / LAMbS_FP_gain;
    
    VIC_target_error = VIC_error - (ns_error + LAMbS_ns_threshold).*inst_retrate_inv;
    [x_boundaries, fbar] = discretized_pdf(VIC_target_error(1:end), 100)
    figure(3);
    bar_vw(x_boundaries, fbar);
    axis([-10000, 10000, -0.0005, 0.002]);
    
end
