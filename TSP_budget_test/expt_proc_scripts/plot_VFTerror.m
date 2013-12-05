function [] = plot_VFTerror(filename, rng)

    raw_mat = csvread(filename);
    
    period = raw_mat(1, 2);
    reservation_period = raw_mat(1, 3);
    
    %Skip the first two rows
    raw_mat = raw_mat(3:end,:);
    
    abs_release_time = raw_mat(:, 4);
    abs_LFT = raw_mat(:, 5);
    LFT = abs_LFT - abs_release_time;
    budget_allocated = raw_mat(:, 6);
    budget_used = raw_mat(:, 7);
    budget_remaining = budget_allocated - budget_used;
    VFT = LFT .- ((budget_remaining ./ budget_allocated) * reservation_period);
    VFT_error = (VFT - period)/period;
    abs_release_time = (abs_release_time - abs_release_time(1))/period;
    
    plot(abs_release_time(rng(1):rng(2)), VFT_error(rng(1):rng(2)), 'k');
end
