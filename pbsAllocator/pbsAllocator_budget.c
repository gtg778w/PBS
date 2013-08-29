#include <stdint.h>

#include <math.h>

#include "pbsAllocator.h"

enum pbs_budget_type    budget_type = PBS_BUDGET_ns;
double                  *presaturation_budget_array;

double                  maximum_available_CPU_time;
double                  maximum_available_CPU_budget;

void compute_max_CPU_budget(void)
{
    if(PBS_BUDGET_ns == budget_type)
    {
        maximum_available_CPU_budget = maximum_available_CPU_time;
    }
    else
    {
        maximum_available_CPU_budget = maximum_available_CPU_time * max_perf_coeff;
    }
}

void compute_budget(SRT_loaddata_t *loaddata, double *budget)
{
    
    int64_t queue_length    = loaddata->queue_length;
    int64_t current_runtime = loaddata->current_runtime;
    
    double  mean0   = loaddata->u_c0;
    double  meanl   = loaddata->u_cl;
    double  var0    = loaddata->var_c0;
    double  varl    = loaddata->var_cl;
    
    double  total_var   = 0;
    double  total_mean  = 0;
    
    double  std;
    
    double  estimated_load;
    double  sp_till_deadline = loaddata->sp_till_deadline;
    
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
        
        std = sqrt(total_var);
        /*It is assumed that the predictor in the SRT task already multiplied all 
        variance values by squared alpha*/
        estimated_load = total_mean + std;
    }    

    *budget = (estimated_load / sp_till_deadline);
}

