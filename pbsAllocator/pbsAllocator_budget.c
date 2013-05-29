#include <stdint.h>

#include <math.h>

#include "pbsAllocator.h"

void compute_budget(SRT_history_t *history, uint64_t* budget)
{
    
    int64_t queue_length = history->queue_length;
    int64_t current_runtime = history->current_runtime;
    
    int64_t mean0   = history->u_c0;
    int64_t meanl   = history->u_cl;
    int64_t var0    = history->var_c0;
    int64_t varl    = history->var_cl;
    
    int64_t total_var = 0;
    int64_t total_mean = 0;
    
    double std;
    
    int64_t estimated_load;
    int64_t sp_till_deadline = history->sp_till_deadline;
    
    if(queue_length == 0)
    {
        estimated_load = 0;
    }
    else
    {
        total_mean = mean0 - current_runtime;
        total_mean = (total_mean < 0)? 0 : total_mean;
        total_var  = var0;
        
        if(queue_length > 1)
        {
            total_mean =  total_mean + (queue_length - 1)*meanl;
            total_var  =  total_var + (queue_length - 1)*varl;
        }
        
        std = sqrt((double)total_var);
        /*It is assumed that the predictor in the SRT task already multiplied all 
        variance values by squared alpha*/
        estimated_load = total_mean + (int64_t)std;
    }    

    *budget = (estimated_load / sp_till_deadline);
}

