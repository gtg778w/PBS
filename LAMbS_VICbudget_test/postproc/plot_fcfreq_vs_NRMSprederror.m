function [handle] = plot_fcfreq_vs_NRMSprederror(fcfreq_series, color, bigness, boldness)

    fcfreq = [];
    NRMSprederror = [];

    formatting = ['x', color];
    for k = 1:length(fcfreq_series)
        
        fcfreq_point = fcfreq_series{k};
        
        _fcfreq = fcfreq_point.fcfreq * ones(size(fcfreq_point.NRMSprederror_array(1:end)));
        fcfreq = [fcfreq, _fcfreq(1:end)];
        
        _NRMSprederror = fcfreq_point.NRMSprederror_array;
        NRMSprederror = [NRMSprederror, _NRMSprederror(1:end)];
        
    end
    
    handle = plot(fcfreq, NRMSprederror, formatting);
    set(handle, 'markersize', bigness);
    set(handle, 'linewidth', boldness);
end
