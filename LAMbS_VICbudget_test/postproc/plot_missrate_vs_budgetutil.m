function [handle] = plot_missrate_vs_budgetutil(alpha_series, color)

    missrate = [];
    budgetutil = [];

    formatting = ['x', color];
    for k = 1:length(alpha_series)
        
        alpha_point = alpha_series{k};
        
        _missrate   = alpha_point.missrate_array;
        missrate = [missrate, _missrate];
        
        _budgetutil = alpha_point.budgetutil_array;
        budgetutil = [budgetutil, _budgetutil];
        
    end
    
    handle = plot(missrate, budgetutil, formatting);
    
end
