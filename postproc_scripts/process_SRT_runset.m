function [  missrate_alpha_plot, ...
            missrate_budget_plot, ...
            budget_utilization_plot] = process_SRT_runset(matfile_path)

    load('-v7', matfile_path, 'SRT_records');

    alpha_mabank = [];
    alpha_hybrid = [];

    miss_rate_mabank = [];
    miss_rate_hybrid = [];
    
    mean_saturated_budget_allocation_mabank = [];
    mean_saturated_budget_allocation_hybrid = [];
    
    budget_utilization_mabank = [];
    budget_utilization_hybrid = [];
    
    run_count = length(SRT_records);
    
    workload_name = SRT_records(1).workload_name;
    
    for r = 1:run_count
        SRT_record = SRT_records(r);
        
        if(0 == strcmp(SRT_records(r).workload_name, workload_name))
            error('process_SRT_runset is meant to process data points from the same ', ...
                    'workload. The SRT_records variable from matfile being ', ...
                    'processed contains data points from more than one workload!' );
        end
        
        alpha               = SRT_record.alpha;
        job_count           = SRT_record.job_count;
        miss_count          = SRT_record.total_misses;
        miss_rate           = miss_count/job_count;
        cumul_budget_sat    = SRT_record.cumulative_budget_sat;
        mean_budget_sat     = cumul_budget_sat/job_count;
        consumed_budget     = SRT_record.consumed_budget;
        budget_utilization  = consumed_budget/cumul_budget_sat;
        
        if( 1 == strcmp(SRT_record.predictor, 'mabank') )
        
            alpha_mabank = [alpha_mabank, alpha];
            miss_rate_mabank = [miss_rate_mabank, miss_rate];
            mean_saturated_budget_allocation_mabank = ...
                [mean_saturated_budget_allocation_mabank, mean_budget_sat];
            budget_utilization_mabank = [budget_utilization_mabank, budget_utilization];
        
        else
            if( 1 == strcmp(SRT_record.predictor, 'mavslmsbank'))
            
                alpha_hybrid = [alpha_hybrid, alpha];
                miss_rate_hybrid = [miss_rate_hybrid, miss_rate];
                mean_saturated_budget_allocation_hybrid = ...
                    [mean_saturated_budget_allocation_hybrid, mean_budget_sat];
                budget_utilization_hybrid = ...
                    [budget_utilization_hybrid, budget_utilization];
            
            end
        end
        
    end
   
    missrate_alpha_plot = figure();
    hold on
    if(length(miss_rate_mabank) > 0)
        scatter(miss_rate_mabank, alpha_mabank, 16, [0, 0, 0], 'x');
    end
    if(length(miss_rate_hybrid) > 0)
        scatter(miss_rate_hybrid, alpha_hybrid, 16, [0, 0, 1], 'o');
    end
    plot_title_string = sprintf(['Plot of miss rate vs alpha for: ' ...
                                '%s'], workload_name);
    h_t = title(plot_title_string);
    set(h_t, 'interpreter', 'none');
    xlabel('miss rate');
    ylabel('alpha');
    hold off
    plot_name = sprintf('%s.alpha_plot.jpg', matfile_path);
    print(missrate_alpha_plot, plot_name, '-djpg');
    
    missrate_budget_plot = figure();
    hold on
    if(length(miss_rate_mabank) > 0)
        scatter(miss_rate_mabank, mean_saturated_budget_allocation_mabank, ...
                16, [0, 0, 0], 'x');
    end
    if(length(miss_rate_hybrid) > 0)
        scatter(miss_rate_hybrid, mean_saturated_budget_allocation_hybrid, ...
                16, [0, 0, 1], 'o');
    end
    plot_title_string = sprintf('Plot of miss rate vs mean budget allocation for: %s', ...
                                workload_name);
    h_t = title(plot_title_string, 'interpreter', 'none');
    h_l = xlabel('miss rate');
    h_l = ylabel('mean budget (ns)');
    hold off
    plot_name = sprintf('%s.budget_plot.jpg', matfile_path);
    print(missrate_budget_plot, plot_name, '-djpg');
    
    budget_utilization_plot = figure();
    hold on
    if(length(miss_rate_mabank) > 0)
        scatter(miss_rate_mabank, budget_utilization_mabank, 16, [0, 0, 0], 'x');
    end
    if(length(miss_rate_hybrid) > 0)
        scatter(miss_rate_hybrid, budget_utilization_hybrid, 16, [0, 0, 1], 'o');
    end
    plot_title_string = sprintf('Plot of miss rate vs mean budget utilization for %s', ...
                                workload_name);
    h_t = title(plot_title_string);
    set(h_t, 'interpreter', 'none');
    xlabel('miss rate');
    ylabel('mean utilization');
    hold off
    plot_name = sprintf('%s.util_plot.jpg', matfile_path);
    print(budget_utilization_plot, plot_name, '-djpg');
end
