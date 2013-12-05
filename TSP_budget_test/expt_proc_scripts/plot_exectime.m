function [] = plot_exectime(filename, rng)

    raw_mat = csvread(filename);
    
    period = raw_mat(1, 2);
    
    %Skip the first two rows
    raw_mat = raw_mat(3:end,:);
    exec_time = raw_mat(:, 1);
    abs_release_time = raw_mat(:, 4);
    
    abs_release_time = (abs_release_time - abs_release_time(1))/period;
    plot(abs_release_time(rng(1):rng(2)), exec_time(rng(1):rng(2)), 'k');
end
