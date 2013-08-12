function [VIC_interval, VIC_error, ns_error] = parse_VICtimer_test_data(filename)

    raw_data = csvread(filename);
    
    VIC_start   = raw_data(:,1);
    VIC_target  = raw_data(:,2);
    VIC_interval= VIC_target - VIC_start;
    VIC_callback= raw_data(:,3);
    VIC_error   = raw_data(:,4);
    ns_target   = raw_data(:,5);
    ns_callback = raw_data(:,6);
    ns_error    = raw_data(:,7);
    transition  = raw_data(:,8);
    
end
