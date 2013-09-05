%{
Check that the following are the same:

    alpha
    c1
    predictor
    
    workload_name
    period
    job_count
    
    relative_release_time
    
Compute the mean, p1th and p2th percentile over the following:

    cumulative_budget
    cumulative_budget_sat
    consumed_budget
    estimated_mean_exectime
    total_misses
    
Compute the mean, p1th and p2th percentile of the following over the repetitions, 
seperately for each job:

    C1
    C2
    VIC1
    VIC2

%}

function [summ_SRT_record] = summarizeResults_SRT_run_full_wVIC(    SRT_records, ...
                                                                    p1, p2, discard)
    
    %Use the values in the first record as standard values
    std_alpha =         SRT_records(1).alpha;
    std_c1 =            SRT_records(1).c1;
    std_predictor =     SRT_records(1).predictor;
    
    std_workload_name = SRT_records(1).workload_name;
    std_period =        SRT_records(1).period;
    std_job_count =     SRT_records(1).job_count;
    
    %count the number of records
    record_count = length(SRT_records);
    
    %set up arrays for some of the remaining varriables
    arr_cumulative_budget =     zeros([1, record_count]);
    arr_cumulative_budget_sat = zeros([1, record_count]);
    arr_consumed_budget =       zeros([1, record_count]);
    arr_estimated_mean_exectime=zeros([1, record_count]);
    arr_total_misses    =       zeros([1, record_count]);

    %count the number of jobs, for which execution time and VIC is available    
    counted_jobs =  length(SRT_records(1).C1);
    
    if(counted_jobs <= discard)
        error([ 'summarizeResults_SRT_run_full_wVIC: The third argument, "discard" is' ...
                'too high given the number of jobs recorded.']);
    end
    
    rel_release_time =  SRT_records(1).abs_release_time(discard+1:end) - ...
                        SRT_records(1).abs_release_time((discard+1));
    std_rel_release_time = rel_release_time;

    std_rel_release_time = std_rel_release_time;
    
    arr_C1       =  zeros([[counted_jobs - discard], record_count]);
    arr_C2       =  zeros([[counted_jobs - discard], record_count]);
    arr_VIC1     =  zeros([[counted_jobs - discard], record_count]);
    arr_VIC2     =  zeros([[counted_jobs - discard], record_count]);

    %loop over the record
    for r = 1:record_count
        
        %Check relevant varriables against standard values
        if(SRT_records(r).alpha ~= std_alpha)
            error('Varriable "alpha" of record %i did not match the value in the first record', 
                  r);
        end

        if(SRT_records(r).c1 ~= std_c1)
            error('Varriable "c1" of record %i did not match the value in the first record', 
                  r);
        end
        
        if(strcmp(SRT_records(r).predictor, std_predictor) ~= 1)
            error('Varriable "predictor" of record %i did not match the value in the first record', 
                  r);
        end
        
        if(strcmp(SRT_records(r).workload_name, std_workload_name) ~= 1)
            error('Varriable "workload_name" of record %i did not match the value in the first record', 
                  r);
        end
        
        if(SRT_records(r).period ~= std_period)
            error('Varriable "period" of record %i did not match the value in the first record', 
                  r);
        end
        
        if(SRT_records(r).job_count ~= std_job_count)
            error('Varriable "job_count" of record %i did not match the value in the first record', 
                  r);
        end
        
        rel_release_time =  SRT_records(r).abs_release_time(discard+1:end) - ...
                            SRT_records(r).abs_release_time(discard+1);
        comp_release_time = rel_release_time ~= std_rel_release_time;
        if(sum(comp_release_time) ~= 0)
            error('Varriable "rel_release_time" of record %i did not match the value in the first record', 
                  r);
        end
        
        arr_cumulative_budget(r)    =   SRT_records(r).cumulative_budget;
        arr_cumulative_budget_sat(r)=   SRT_records(r).cumulative_budget_sat;
        arr_consumed_budget(r)      =   SRT_records(r).consumed_budget;
        arr_estimated_mean_exectime(r)= SRT_records(r).estimated_mean_exectime;
        arr_total_misses(r)         =   SRT_records(r).total_misses;

        arr_C1(:, r)    =   SRT_records(r).C1(discard+1:end);
        arr_C2(:, r)    =   SRT_records(r).C2(discard+1:end);
        arr_VIC1(:, r)  =   SRT_records(r).VIC1(discard+1:end);
        arr_VIC2(:, r)  =   SRT_records(r).VIC2(discard+1:end);
        
    end
    
    summ_SRT_record.alpha = std_alpha;
    summ_SRT_record.c1 = std_c1;
    summ_SRT_record.predictor = std_predictor;
    
    summ_SRT_record.workload_name = std_workload_name;
    summ_SRT_record.period = std_period;
    summ_SRT_record.job_count = std_job_count;
    
    summ_SRT_record.rel_release_time = std_rel_release_time;
    
    summ_SRT_record.p1 = p1;
    summ_SRT_record.p1 = p2;

    summ_SRT_record.avg_cumulative_budget =         mean(arr_cumulative_budget, 2);
    summ_SRT_record.avg_cumulative_budget_sat =     mean(arr_cumulative_budget_sat, 2);
    summ_SRT_record.avg_consumed_budget =           mean(arr_consumed_budget, 2);
    summ_SRT_record.avg_estimated_mean_exectime =   mean(arr_estimated_mean_exectime, 2);
    summ_SRT_record.avg_total_misses =              mean(arr_total_misses, 2);
    summ_SRT_record.avg_C1 =                        mean(arr_C1, 2);
    summ_SRT_record.avg_C2 =                        mean(arr_C2, 2);
    summ_SRT_record.avg_VIC1 =                      mean(arr_VIC1, 2);
    summ_SRT_record.avg_VIC2 =                      mean(arr_VIC2, 2);
    
    summ_SRT_record.p1_cumulative_budget =          prctile(arr_cumulative_budget, p1, 2);
    summ_SRT_record.p1_cumulative_budget_sat =      prctile(arr_cumulative_budget_sat, p1, 2);
    summ_SRT_record.p1_consumed_budget =            prctile(arr_consumed_budget, p1, 2);
    summ_SRT_record.p1_estimated_mean_exectime =    prctile(arr_estimated_mean_exectime, p1, 2);
    summ_SRT_record.p1_total_misses =               prctile(arr_total_misses, p1, 2);
    summ_SRT_record.p1_C1 =                         prctile(arr_C1, p1, 2);
    summ_SRT_record.p1_C2 =                         prctile(arr_C2, p1, 2);
    summ_SRT_record.p1_VIC1 =                       prctile(arr_VIC1, p1, 2);
    summ_SRT_record.p1_VIC2 =                       prctile(arr_VIC2, p1, 2);
    
    summ_SRT_record.p2_cumulative_budget =          prctile(arr_cumulative_budget, p2, 2);
    summ_SRT_record.p2_cumulative_budget_sat =      prctile(arr_cumulative_budget_sat, p2, 2);
    summ_SRT_record.p2_consumed_budget =            prctile(arr_consumed_budget, p2, 2);
    summ_SRT_record.p2_estimated_mean_exectime =    prctile(arr_estimated_mean_exectime, p2, 2);
    summ_SRT_record.p2_total_misses =               prctile(arr_total_misses, p2, 2);
    summ_SRT_record.p2_C1 =                         prctile(arr_C1, p2, 2);
    summ_SRT_record.p2_C2 =                         prctile(arr_C2, p2, 2);
    summ_SRT_record.p2_VIC1 =                       prctile(arr_VIC1, p2, 2);
    summ_SRT_record.p2_VIC2 =                       prctile(arr_VIC2, p2, 2);
end

