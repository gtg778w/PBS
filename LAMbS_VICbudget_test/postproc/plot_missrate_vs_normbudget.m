function [handle] = plot_missrate_vs_normbudget(alpha_series, formatting)

    missrate = [];
    normbudget = [];

    for k = 1:length(alpha_series)
        
        alpha_point = alpha_series{k};
        
        missrate(k) = alpha_point.missrate_mean;
        
        normbudget(k) = alpha_point.normbudget_mean;
        
    end
    
    handle = plot(missrate, normbudget, formatting);
    
end
