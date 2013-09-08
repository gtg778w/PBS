%{

Compute the mean, p1th and p2th percentile over the following:

    allocated_budget
    consumed_budget
    total_misses
    
Compute the mean, p1th and p2th percentile of the following over the repetitions, 
seperately for each job:

    cpuusage_ns
    cpuusage_VIC

%}

function [summ_SRT_record] = summarizeResults_SRT_run_full_wVIC(    SRT_records, ...
                                                                    p1, p2, discard)
        
    %count the number of records
    record_count = length(SRT_records);
    
    %set up arrays for some of the remaining varriables
    arr_allocated_budget    = zeros([1, record_count]);
    arr_consumed_budget_ns  = zeros([1, record_count]);
    arr_consumed_budget_VIC = zeros([1, record_count]);
    arr_total_misses        = zeros([1, record_count]);

    %count the number of jobs, for which execution time and VIC is available    
    counted_jobs =  length(SRT_records(1).cpuusage_ns);
    
    if(counted_jobs <= discard)
        error([ 'summarizeResults_SRT_run_full_wVIC: The third argument, "discard" is' ...
                'too high given the number of jobs recorded.']);
    end
    
    rel_release_time =  SRT_records(1).releaseTime(discard+1:end) - ...
                        SRT_records(1).releaseTime((discard+1));
    std_rel_release_time = rel_release_time;
    
    arr_cpuusage_ns  =  zeros([[counted_jobs - discard], record_count]);
    arr_cpuusage_VIC =  zeros([[counted_jobs - discard], record_count]);

    %loop over the record
    for r = 1:record_count
                
        rel_release_time =  SRT_records(r).releaseTime(discard+1:end) - ...
                            SRT_records(r).releaseTime(discard+1);
        comp_release_time = rel_release_time ~= std_rel_release_time;
        if(sum(comp_release_time) ~= 0)
            error('Varriable "rel_release_time" of record %i did not match the value in the first record', 
                  r);
        end

        arr_allocated_budget(r) =   SRT_records(r).allocated_budget;
        arr_consumed_budget_ns(r)   =   SRT_records(r).consumed_budget_ns;
        arr_consumed_budget_VIC(r)  =   SRT_records(r).consumed_budget_VIC;
        arr_total_misses(r)     =   SRT_records(r).total_misses;

        arr_cpuusage_ns(:, r)   =   SRT_records(r).cpuusage_ns(discard+1:end);
        arr_cpuusage_VIC(:,r)   =   SRT_records(r).cpuusage_VIC(discard+1:end);
        
    end
        
    summ_SRT_record.rel_release_time = std_rel_release_time;
    
    summ_SRT_record.p1 = p1;
    summ_SRT_record.p1 = p2;

    summ_SRT_record.avg_allocated_budget    =   mean(arr_allocated_budget, 2);
    summ_SRT_record.avg_consumed_budget_ns  =   mean(arr_consumed_budget_ns, 2);
    summ_SRT_record.avg_consumed_budget_VIC =   mean(arr_consumed_budget_VIC, 2);
    summ_SRT_record.avg_total_misses=           mean(arr_total_misses, 2);
    summ_SRT_record.avg_cpuusage_ns =           mean(arr_cpuusage_ns, 2);
    summ_SRT_record.avg_cpuusage_VIC=           mean(arr_cpuusage_VIC, 2);
    
    summ_SRT_record.p1_allocated_budget    =    prctile(arr_allocated_budget, p1, 2);
    summ_SRT_record.p1_consumed_budget_ns  =    prctile(arr_consumed_budget_ns, p1, 2);
    summ_SRT_record.p1_consumed_budget_VIC =    prctile(arr_consumed_budget_VIC, p1, 2);
    summ_SRT_record.p1_total_misses=            prctile(arr_total_misses, p1, 2);
    summ_SRT_record.p1_cpuusage_ns =            prctile(arr_cpuusage_ns, p1, 2);
    summ_SRT_record.p1_cpuusage_VIC=            prctile(arr_cpuusage_VIC, p1, 2);
    
    summ_SRT_record.p2_allocated_budget    =    prctile(arr_allocated_budget, p2, 2);
    summ_SRT_record.p2_consumed_budget_ns  =    prctile(arr_consumed_budget_ns, p2, 2);
    summ_SRT_record.p2_consumed_budget_VIC =    prctile(arr_consumed_budget_VIC, p2, 2);
    summ_SRT_record.p2_total_misses=            prctile(arr_total_misses, p2, 2);
    summ_SRT_record.p2_cpuusage_ns =            prctile(arr_cpuusage_ns, p2, 2);
    summ_SRT_record.p2_cpuusage_VIC=            prctile(arr_cpuusage_VIC, p2, 2);
    
end

