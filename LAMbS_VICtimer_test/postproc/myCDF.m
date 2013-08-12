
function [x, F_x] = myCDF(X)
    x = sort(X);
    N = length(X);
    F_x = (1:N)/N;
end

