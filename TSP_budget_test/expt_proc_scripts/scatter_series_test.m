
    x1 = 0:(2*pi/100):(2*pi);
    x2 = 2*x1;
    y1 = sin(x1);
    y2 = cos(x1);
    y3 = sin(x2);
    y4 = cos(x2);
    names = {'a', 'b', 'c', 'd'};
    
    x_vals = {x1, x1, x1, x1};
    y_vals = {y1, y2, y3, y4};
    sizes  = {10, 10, 10, 10};
    colors = {[1, 0, 0], [0, 0, 1], [0, 0, 0], [0.7071, 0, 0.7071]};
    styles = {'^', 's', 'x', '+'}
    
    scatter_series_set(x_vals, y_vals, sizes, colors, styles);
    
    legend(names, 'location', 'southeast');
    print(gcf, 'scatter_test.jpg', '-djpg');

