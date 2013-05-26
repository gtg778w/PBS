#include <stdio.h>
#include <strings.h>
#include "MABank.h"

void* pbsSRT_alloc_MABank(void)
{
    return malloc(MABank_size());
}

void pbsSRT_init_MABank(void    *state)
{
    memset(state, 0, MABank_size());
}

int pbsSRT_update_MABank(   void    *state, int64_t exec_time,
                            int64_t *pu_c0, int64_t *pvar_c0,
                            int64_t *pu_cl, int64_t *pvar_cl)
{
    MABank_t *MABank_p  = (MABank_t*)state;
    double   observed   = (double)exec_time;
    int32_t fid;
	int32_t i;

	double  *prediction = MABank_p->prediction;
	double  *error_mean = MABank_p->error_mean;
	double  *error_var  = MABank_p->error_var;
	double  *buffer     = MABank_p->buffer;

	double  next_indep_var;
	double  prev_indep_var;

    double  error;
    double  error_diff;

	double  filter_acc;

    int32_t min_var_f;
    double  min_var;
    double  min_var_pred;

    int warmup = MABank_p->warmup;
    int ret = 0;
    
    /*Compute the error and error statistics for the zero filter*/
    error = observed;
    switch(warmup)
    {
        case 0:
            ret = -1;
            MABank_p->warmup = warmup + 1;
            error_mean[0] = error;
            error_var[0] = (int64_t)((~(uint64_t)0) >> 1);
            break;
            
        case 1:
            ret = -1;
            MABank_p->warmup = warmup + 1;
            error_mean[0]   = MABank_gamma * error_mean[0] + (1 - MABank_gamma) * error;
            error_diff      = error - error_mean[0];
            error_var[0]    = error_diff * error_diff;
            break;
            
        default:            
            error_mean[0]   =   MABank_gamma * error_mean[0] + (1 - MABank_gamma) * error;
            error_diff      =   error - error_mean[0];
            error_var[0]    =   MABank_gamma * error_var[0] + 
                                (1 - MABank_gamma) * error_diff * error_diff;
            break;
    }
    
    /*Initially set the zero filter as the minimum var filter*/
    min_var = error_var[0];
    min_var_f = 0;
    
    /*perform the convolution for the longest filter AND update the history by shifting
    the new observed value into the registers*/
    next_indep_var = observed;
    filter_acc = 0;
    i = 0;
    for(fid = 0; fid < MABank_FCOUNT; fid++)
    {
        /*Compute the error and error statistics for filter fid*/
        error = observed - prediction[fid];
        switch(warmup)
        {
            case 0:
                error_mean[(fid+1)] = error;
                error_var[(fid+1)] = (int64_t)((~(uint64_t)0) >> 1);
                break;
                
            case 1:
                error_mean[(fid+1)]=    MABank_gamma * error_mean[(fid+1)] + 
                                        (1 - MABank_gamma) * error;
                error_diff  =   error - error_mean[(fid+1)];
                error_var[(fid+1)] =    error_diff * error_diff;
                break;
                
            default:
                error_mean[(fid+1)]=    MABank_gamma * error_mean[(fid+1)] + 
                                        (1 - MABank_gamma) * error;
                error_diff  =   error - error_mean[(fid+1)];
                error_var[(fid+1)] =    MABank_gamma * error_var[(fid+1)] + 
                                        (1 - MABank_gamma) * error_diff * error_diff;
                break;
        }
        
        /*Determine if this is the minimum variance filter*/
        min_var_f   =   (error_var[(fid+1)] < min_var)?  
                        (fid+1)             :   min_var_f;
        min_var     =   (error_var[(fid+1)] < min_var)?  
                        error_var[(fid+1)]  : min_var;

        /*Accumulate additional terms to compute the running average over a longer 
        history*/
        for(; i < MABank_lengths[fid]; i++)
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
        prediction[fid] = filter_acc * MABank_weights[fid]; 
    }
        
    /*Store the final x_hat prediction value*/
    if(min_var_f == 0)
    {
        min_var_pred = 0;
    }
    else
    {
        min_var_pred = prediction[(min_var_f-1)];
    }

    MABank_p->min_var_f = min_var_f;
    MABank_p->x_hat = min_var_pred + error_mean[min_var_f];
    
    *pu_c0      = (int64_t)MABank_p->x_hat;
    *pvar_c0    = (int64_t)error_var[min_var_f];
    *pu_cl      = (int64_t)MABank_p->x_hat;
    *pvar_cl    = (int64_t)error_var[min_var_f];
    
    return ret;
}

int pbsSRT_predict_MABank(  void   *state,
                            int64_t *pu_c0, int64_t *pvar_c0,
                            int64_t *pu_cl, int64_t *pvar_cl)
{
    MABank_t *MABank_p  = (MABank_t*)state;

	double  *error_var  = MABank_p->error_var;
    int min_var_f = MABank_p->min_var_f;

    int warmup = MABank_p->warmup;
    	    
    int ret;

    *pu_c0      = (int64_t)MABank_p->x_hat;
    *pvar_c0    = (int64_t)error_var[min_var_f];
    *pu_cl      = (int64_t)MABank_p->x_hat;
    *pvar_cl    = (int64_t)error_var[min_var_f];

    ret = (warmup < 2)? -1 : 0;
        
    return ret;
}

void pbsSRT_free_MABank(  void    *state)
{
    free(state);
}

