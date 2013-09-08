function [] = plot_C_vs_VIC(result_struct, initial_skip, plot_name, filedir)

    initial_skip = initial_skip + 1;
    
    C   = result_struct.cpuusage_ns(initial_skip:end);
    VIC = result_struct.cpuusage_VIC(initial_skip:end);
    t   =   result_struct.releaseTime(initial_skip:end) - ...
            result_struct.releaseTime(1);
    
    plot_title = sprintf('Plot of C and VIC against t for %s', plot_name);
    
    fig_h = figure();
        
    subplot(2, 1, 1);
    plot(t, C);
    xlabel('time (ns)');
    ylabel('Execution Time (ns)');
    
    title_h = title(plot_title);
    
    subplot(2, 1, 2);
    plot(t, VIC);
    xlabel('time (ns)');
    ylabel('Virtual Instruction Count');
    
    figurename = sprintf('%s/%s.jpg', filedir, plot_name);
    print (fig_h, figurename, '-djpg');
end
