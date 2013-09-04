function [VFT_error_plot] = plot_VFT_error(data, figtitle)

    VFT_error_plot.figure1 = figure;
    VFT_error_plot.plot1 = plot(data.releaseTime, data.cpuusage_ns);
    VFT_error_plot.xlabel1 = xlabel('job release time (ns)');
    VFT_error_plot.ylabel1 = ylabel('job CPU usage (ns)');
    VFT_error_plot.title1 = title(figtitle);
    
    VFT_error_plot.figure2  = figure;
    VFT_error_plot.plot2    = plot(data.releaseTime, data.cpuusage_VIC);
    VFT_error_plot.xlabel2  = xlabel('job release time (ns)');
    VFT_error_plot.ylabel2  = ylabel('job CPU usage (VIC)');
    VFT_error_plot.title2   = title(figtitle);
        
    VFT_error_plot.figure3  = figure;
    VFT_error_plot.plot3    = plot(data.releaseTime, data.norm_VFT_error);
    VFT_error_plot.xlabel3  = xlabel('job release time (ns)');
    VFT_error_plot.ylabel3  = ylabel('normalized VFT error (task periods)');
    VFT_error_plot.title3   = title(figtitle);
    
end
