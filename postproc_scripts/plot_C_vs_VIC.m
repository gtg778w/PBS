function [] = plot_C_vs_VIC(result_struct, initial_skip, plot_name, filedir, ...
                            Caxis_vals, VICaxis_vals)

    initial_skip = initial_skip + 1;
    
    C   = result_struct.C1(initial_skip:end);
    VIC = result_struct.VIC1(initial_skip:end);
    t   = result_struct.abs_release_time(initial_skip:end);
    
    plot_title = sprintf('Plot of C and VIC against t for %s', plot_name);
    
    fig_h = figure();
        
    subplot(2, 1, 1);
    plot(t, C);
    xlabel('time (ns)');
    ylabel('Execution Time (ns)');
    axis(Caxis_vals);
    
    title_h = title(plot_title);
    
    subplot(2, 1, 2);
    plot(t, VIC);
    xlabel('time (ns)');
    ylabel('Virtual Instruction Count');
    axis(VICaxis_vals);
    
    figurename = sprintf('%s/%s.jpg', filedir, plot_name);
    print (fig_h, figurename, '-djpg');
end
