%for plotting variable width bar charts where each bar can have a different width
%x represents the boundaries of the bars. y represents the height of the bars
function h = bar_vw(x, y, varargin)

    if length(varargin) == 0
        C = 'b';
    else
        if(length(varargin) == 1)
            C = varargin{1};
        else
            error('Usage: bar_vw(x, y [, C])');
        end
    end
    
    if(length(x) ~= (length(y)+1))
        error('The length of array x (1st argument) must be 1 element longer than the length of array y');
    end
    
    patch_x = zeros([4, length(y)]);
    patch_y = patch_x;
    h = [];
    for(c = 1:length(y))
        patch_x(1, c) = x(c);
        patch_x(2, c) = x(c);
        patch_x(3, c) = x(c+1);
        patch_x(4, c) = x(c+1);

        patch_y(1, c) = 0;
        patch_y(2, c) = y(c);
        patch_y(3, c) = y(c);
        patch_y(4, c) = 0;
    end

    h = patch(patch_x, patch_y, C);

end
