function [] = plot_norm_C_vs_VIC(   summ_SRT_record, std_C, std_VIC, ...
                                    plot_name, filedir, ...
                                    Caxis_vals, VICaxis_vals)


    norm_p1_C = summ_SRT_record.p1_cpuusage_ns ./ std_C;
    norm_avg_C= summ_SRT_record.avg_cpuusage_ns ./ std_C;
    norm_p2_C = summ_SRT_record.p2_cpuusage_ns ./ std_C;
    
    norm_p1_VIC =   summ_SRT_record.p1_cpuusage_VIC ./ std_VIC;
    norm_avg_VIC =  summ_SRT_record.avg_cpuusage_VIC ./ std_VIC;
    norm_p2_VIC =   summ_SRT_record.p2_cpuusage_VIC ./ std_VIC;

    t = summ_SRT_record.rel_release_time;
    
    plot_title = sprintf('Plot of normalized C and VIC against t for %s', plot_name);
    
    fig_h = figure();
        
    subplot(2, 1, 1);
    hold on
    %plot(t, norm_p1_C);
    plot(t, norm_avg_C);
    %plot(t, norm_p2_C);
    xlabel('Time (ns)');
    ylabel('Normalized Execution Time');
    axis(Caxis_vals);
    
    title_h = title(plot_title);
    
    subplot(2, 1, 2);
    %plot(t, norm_p1_VIC);
    plot(t, norm_avg_VIC);
    %plot(t, norm_p2_VIC);
    xlabel('Time (ns)');
    ylabel('Normalized VIC');
    axis(VICaxis_vals);
    
    figurename = sprintf('%s/%s.jpg', filedir, plot_name);
    print (fig_h, figurename, '-djpg');
end
