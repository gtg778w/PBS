% Takes in an array x and optionally some number of bins N. 
% If N is not specified, it is assumed to be 10. N should 
% be no larger than the length of x minus 1. x should have 
% minimum length 11.
%
% The data is sorted and seperated into N bins whose boundaries  
% represent 0th percentile, through (i*100/N)th percentiles for 
% i = 1:N up to the 100th percentile.
%
% For each bin, the height of the bin is computed as (1/N) 
% divided by the width of the bin.
%
% This is a form of probability density function where the 
% horizontal axis is discretized, such that in regions with
% a lot of points is represented with a higher horizontal
% resolution and regions with few points are represented with a
% coarser horizontal resolution.
%
% The height of the bars represent the average probability density
% over the width of the bar, since the area of each bar multiplied
% by the size of the data set would equal the actual number of
% points that fall with in the region represented by the bar
%
function [x_boundaries, fbar] = discretized_pdf(x, varargin)

    [r, c] = size(x);
    if(c > 1)
        x = x(1:end);
    end
    
    x_count = length(x);
    if(x_count < 10)
        error('argument 1 should be an array of at least length 11');
    end

    if length(varargin) == 0
        N = 10;
    else
        N = varargin{1};
    end

    if x_count < (N+1)
        error('argument 1 should be an array of at least length (N+1), the second argument');
    end

    sorted_x = sort(x);
    y_inc = x_count/N;
    
    y   = [1, (y_inc:y_inc:x_count)];
    rounded_y   = round(y);
     
    x_boundaries= sorted_x(rounded_y);
    [r, c] = size(x_boundaries);
    if(c > 1)
        x_boundaries = x_boundaries';
    end

    x_start     = x_boundaries(1:end-1);
    x_end       = x_boundaries(2:end);
    
    for i = 1:N
        fbar_num(i) = sum((x>=x_start(i)) & (x<x_end(i)));
    end
    fbar_den = (x_end - x_start);

    [r, c] = size(fbar_num);
    if(c > 1)
        fbar_num = fbar_num';
    end

    fbar = fbar_num(1:end) ./ fbar_den(1:end);
    fbar(fbar_den == 0) = 0;
    
    fbar = fbar/x_count;
end

