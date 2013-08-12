function [ns_interval, ns_error] = parse_hrtimer_test_data(filename)

    raw_data = csvread(filename);
    
    ns_start    = raw_data(:,1);
    ns_target   = raw_data(:,2);
    ns_callback = raw_data(:,3);
    ns_interval = raw_data(:,4);
    ns_error    = raw_data(:,5);
    
end
