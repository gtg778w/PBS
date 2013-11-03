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
    
    double  mean0   = (double)loaddata->u_c0;
    double  meanl   = (double)loaddata->u_cl;
    double  var0    = (double)loaddata->var_c0;
    double  varl    = (double)loaddata->var_cl;
    double  alpha   = ((double)loaddata->alpha_fp) / ((double)((uint32_t)1 << 16));
    double  std0;
    
    double  c0_load;
    double  total_var;
    double  total_mean;
    double  total_std;
    
    double  estimated_load;
    double  sp_till_deadline = loaddata->sp_till_deadline;
    
    if(queue_length == 0)
    {
        estimated_load = 0;
    }
    else
    {
        std0       = sqrt(var0);
        /*
            To understand this comment, look at the chapter on the TSP in my dissertation:
            
            Let c0_load be defined as (c_{cond}[k] - c_{min}[k])
            
            c_{trans}[k] = mean0 - std0;
            
            if( c_{trans}[k] > c_{min}[k])
            {
                c0_load = mean0 - c_{min}[k];
            }
            else
            {
                c0_load = std0;
            }
            
            or equivalently
            
            c0_load = mean0 - c_{min}[k];
            if(c0_load < std0)
            {
                c0_load = std0;
            }
        */
        c0_load = mean0 - current_runtime;
        c0_load = (c0_load < std0)? std0 : c0_load;
        
        total_mean = c0_load;
        total_var  = var0;
        
        if(queue_length > 1)
        {
            total_mean += (queue_length - 1)*meanl;
            total_var  += (queue_length - 1)*varl;
        }
        total_std = sqrt(total_var);
        
        /*It is assumed that the predictor in the SRT task already multiplied all 
        variance values by squared alpha*/
        estimated_load = total_mean + alpha*total_std;
    }    

    *budget = (estimated_load / sp_till_deadline);
}

