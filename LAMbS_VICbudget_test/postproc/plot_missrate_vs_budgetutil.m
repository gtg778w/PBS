function [handle] = plot_missrate_vs_budgetutil(alpha_series, formatting)

    missrate = [];
    budgetutil = [];

    for k = 1:length(alpha_series)
        
        alpha_point = alpha_series{k};
        
        missrate(k) = alpha_point.missrate_mean;
        
        budgetutil(k) = alpha_point.budgetutil_mean;
        
    end
    
    handle = plot(missrate, budgetutil, formatting);
    
end
