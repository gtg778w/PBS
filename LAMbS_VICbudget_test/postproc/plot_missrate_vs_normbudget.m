function [handle] = plot_missrate_vs_normbudget(alpha_series, color, bigness, boldness)

    missrate = [];
    normbudget = [];

    formatting = ['x', color];
    for k = 1:length(alpha_series)
        
        alpha_point = alpha_series{k};
        
        _missrate = alpha_point.missrate_array;
        missrate = [missrate, _missrate];
        
        _normbudget = alpha_point.normbudget_array;
        normbudget = [normbudget, _normbudget];
        
    end
    
    handle = plot(missrate, normbudget, formatting);
    set(handle, 'markersize', bigness);
    set(handle, 'linewidth', boldness);
end
