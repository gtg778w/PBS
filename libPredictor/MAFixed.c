#include <stdio.h>
#include <strings.h>
#include "MA.h"

/*The forgetting factor for the exponential running average of error values
1 - 2^(-5) = 1 - 0.03125 = 0.96875*/
#define MA_gamma (0.96875)

void* pbsSRT_alloc_MA(void)
{
    return malloc(MA_size());
}

void pbsSRT_init_MA(void    *state)
{
    memset(state, 0, MA_size());
}

int pbsSRT_update_MA(   void    *state, int64_t exec_time,
                        int64_t *pu_c0, int64_t *pvar_c0,
                        int64_t *pu_cl, int64_t *pvar_cl)
{
    MA_t *MA_p  = (MA_t*)state;
    double   observed   = (double)exec_time;
    int32_t i;

    double  prediction = MA_p->prediction;
    double  error_mean = MA_p->error_mean;
    double  error_var  = MA_p->error_var;
    double  *buffer     = MA_p->buffer;

    double  next_indep_var;
    double  prev_indep_var;

    double  error;
    double  error_diff;

    double  filter_acc;

    int32_t min_var_f;
    double  min_var;
    double  min_var_pred;

    int warmup = MA_p->warmup;
    int ret = 0;
    
    /*Compute the error and error statistics*/
    error = observed;
    switch(warmup)
    {
        case 0:
            ret = -1;
            MA_p->warmup    = warmup + 1;
            MA_p->error_mean= error;
            MA_p->error_var = (int64_t)((~(uint64_t)0) >> 1);
            break;
            
        case 1:
            ret = -1;
            MA_p->warmup    = warmup + 1;
            MA_p->error_mean= MA_gamma * MA_p->error_mean + (1 - MA_gamma) * error;
            error_diff      = error - MA_p->error_mean;
            MA_p->error_var = error_diff * error_diff;
            break;
            
        default:            
            MA_p->error_mean=   MA_gamma * MA_p->error_mean + (1 - MA_gamma) * error;
            error_diff      =   error - MA_p->error_mean;
            MA_p->error_var =   MA_gamma * MA_p->error_var + 
                                (1 - MA_gamma) * error_diff * error_diff;
            break;
    }
    
    /*perform the convolution for the longest filter AND update the history by shifting
    the new observed value into the registers*/
    next_indep_var = observed;
    filter_acc = 0;
    /*Accumulate additional terms to compute the running average over a longer 
    history*/
    for(i = 0; i < MA_LENGTH; i++)
    {
        filter_acc = filter_acc + next_indep_var;

        /*Shift the buffer up 1 by swapping next_indep_var and buffer[i].
        Therefore, the contents of observed is inserted into buffer[0], and the
        contents of buffer[i] are shifted to buffer[i+1]
        */
        prev_indep_var  = next_indep_var;
        next_indep_var  = buffer[i];
        buffer[i]       = prev_indep_var;
    }
    
    /*Compute the average over the sum*/
    prediction = filter_acc * (1/(double)MA_LENGTH);
    
    MA_p->x_hat = prediction + MA_p->error_mean;
    
    *pu_c0      = (int64_t)MA_p->x_hat;
    *pvar_c0    = (int64_t)MA_p->error_var;
    *pu_cl      = (int64_t)MA_p->x_hat;
    *pvar_cl    = (int64_t)MA_p->error_var;
    
    return ret;
}

int pbsSRT_predict_MA(  void   *state,
                            int64_t *pu_c0, int64_t *pvar_c0,
                            int64_t *pu_cl, int64_t *pvar_cl)
{
    MA_t *MA_p  = (MA_t*)state;

    double  *error_var  = MA_p->error_var;
    int min_var_f = MA_p->min_var_f;

    int warmup = MA_p->warmup;
    
    int ret;

    *pu_c0      = (int64_t)MA_p->x_hat;
    *pvar_c0    = (int64_t)MA_p->error_var;
    *pu_cl      = (int64_t)MA_p->x_hat;
    *pvar_cl    = (int64_t)MA_p->error_var;

    ret = (warmup < 2)? -1 : 0;
        
    return ret;
}

void pbsSRT_free_MA(  void    *state)
{
    free(state);
}

