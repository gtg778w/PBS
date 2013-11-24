function [handles] = scatter_series_set(series_names, x_vals, y_vals, colors)
    
    N = length(series_names);
    
    if ~ ( N == length(x_vals))
        error('scatter_series_set: all arguments must be cell arrays of the same length');
    end
    
    if ~ ( N == length(y_vals))
        error('scatter_series_set: all arguments must be cell arrays of the same length');
    end

    if ~ ( N == length(colors))
        error('scatter_series_set: all arguments must be cell arrays of the same length');
    end
    
    %clear the plot
    handles = cell([N, 1]);
    hold on;
    for index = 1:N
        handles{index} = scatter(x_vals{index}, y_vals{index}, [], colors{index});
        printf('series name: %s, color: (%f, %f, %f)\n',    series_names{index}, ...
                                                            colors{index}(1), ...
                                                            colors{index}(2), ...
                                                            colors{index}(3));
    end
    hold off;
end
