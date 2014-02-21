function [x_prime, ut] = get_dbydt(x, t)

    if(length(x) < 2)
        error('Need at least two points to compute estimated derivative');
    end
    
    if(length(x) ~= length(t))
        error('The length of x and the length of t must be the same')
    end
    
    dx = x(2:end) - x(1:end-1);
    dt = t(2:end) - t(1:end-1);
    ut = (t(2:end) + t(1:end-1))/2;
    ut = ut - ut(1);
    
    x_prime = dx ./ dt;
end
