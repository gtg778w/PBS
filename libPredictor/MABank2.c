#include <stdio.h>
#include <strings.h>
#include <stdint.h>
#include "libPredictor_variance.h"
#include "MABank2.h"

void* libPredictor_alloc_MABank2(   void)
{
    return malloc(MABank2_size());
}

void libPredictor_init_MABank2( void    *state)
{
    int32_t fid;
    
    int32_t x_buffer_length;
    
    int32_t prev_buffer_length;
    float   *prev_buffer_addr;
    
    /*Cast the state in void* to the propper type*/
    MABank2_t *MABank2_p  = (MABank2_t*)state;
    
    /*Zero-out the structure*/
    memset(state, 0, MABank2_size());

    /*Get the X buffer length*/
    x_buffer_length = MABank2_lengths[MABank2_FCOUNT-1];
    
    /*Set the pointers to all the error buffers
        - the first error buffer begins where the x buffer leaves off
        - each successive error buffer begins where the previous error buffer leaves off
    */
    MABank2_p->error_buffer[0] = (float*)&(MABank2_p->x_buffer[x_buffer_length]);
    for( fid = 1; fid < MABank2_FCOUNT; fid++)
    {
        prev_buffer_addr    = MABank2_p->error_buffer[(fid-1)];
        prev_buffer_length  = MABank2_lengths[(fid-1)];
        
        MABank2_p->error_buffer[fid] = &(prev_buffer_addr[prev_buffer_length]);
    }
    
}

int libPredictor_update_MABank2(void    *state, int64_t exec_time,
                                int64_t *pu_c0, int64_t *pvar_c0,
                                int64_t *pu_cl, int64_t *pvar_cl)
{
    int ret = 0;

    MABank2_t *MABank2_p  = (MABank2_t*)state;
    int warmup = MABank2_p->warmup;
    int32_t limit;

    double  observed   = (double)exec_time;

    int32_t fid;
    int32_t i;
    double  n;

    /*Previous prediction buffer and a variable to compute the prediction errors*/
    int32_t filter_length;
    double  *prediction     = MABank2_p->prediction;
    double  error;

    /*arrays pointing to the error buffers and error statistics*/    
    float   **error_buffers = MABank2_p->error_buffer;
    double  *error_means    = MABank2_p->error_mean;
    double  *error_vars     = MABank2_p->error_var;

    /*The index of the minimum error variance filter*/
    int32_t min_var_f = -1;
    double  minimum_variance =  DBL_MAX;

    /*The array of previous observations*/
    double  *x_buffer = MABank2_p->x_buffer;
    
    /*The accumulator used in the averaging operation*/
    double  filter_acc;
    
    /*varibales for shifting new observations into the observation buffer*/    
    double  next_indep_var;
    double  prev_indep_var;

    double  u, var;

    /*check the warmup variable. increment if less than the length of the 
    largest moving average*/
    if(warmup < MABank2_lengths[MABank2_FCOUNT-1])
    {
        warmup++;
        
        if(warmup < 2)
        {
            /*Should only produce an output if there has been a total of
              2 or more observations*/
            ret = -1;
        }
    }
    
    /*Compute the error and error statistics for the different moving averages*/
    for(fid = 0; fid < MABank2_FCOUNT; fid++)
    {
        /*Get the last prediction error*/
        error = (float)(observed - prediction[fid]);
        
        /*Check how much of the moving window for the particular error is full*/
        filter_length = MABank2_lengths[fid];
        limit = (warmup < filter_length)? 
                warmup : filter_length;
                
        /*Get the error mean and error variance*/
        libPredictor_fvariance( error,
                                limit,  error_buffers[fid],
                                &(error_means[fid]), &(error_vars[fid]));
        
        if(error_vars[fid] < minimum_variance)
        {
            min_var_f =         fid;
            minimum_variance =  error_vars[fid];
        }
    }
        
    /*
        - iterate over the data and dynamically compute the averages of different 
          lengths.
        - these averages make up the next set of predictions
    */
    filter_acc = 0;
    next_indep_var = observed;
    i = 0;
    n = 0.0;
    for(fid = 0; fid < MABank2_FCOUNT; fid++)
    {
        /*keep summing and shifting until the next largest filter length*/
        for(; i < MABank2_lengths[fid]; i++, n+=1.0)
        {
            filter_acc = filter_acc + next_indep_var;

            /*Shift the buffer up 1 by swapping next_indep_var and buffer[i].
            Therefore, the contents of observed is inserted into buffer[0], and the
            contents of buffer[i] are shifted to buffer[i+1] for i = 0 to the longest 
            length
            */
            prev_indep_var  = next_indep_var;
            next_indep_var  = x_buffer[i];
            x_buffer[i]     = prev_indep_var;
        }
        
        /*Compute the average over the sum*/
        prediction[fid] = filter_acc/n;
    }
        
    /*Check if the predictor has enough history to give some valid output*/
    MABank2_p->warmup    = warmup;
    if( ret == 0 )
    {
        MABank2_p->min_var_f = min_var_f;
        
        /*The final output is the predictor output and the mean error for that 
          predictor */
        u   = prediction[min_var_f] + error_means[min_var_f];
        var = error_vars[min_var_f];
        
        /*Set the output variables*/
        *pu_c0      = (int64_t)u;
        *pvar_c0    = (int64_t)var;
        *pu_cl      = (int64_t)u;
        *pvar_cl    = (int64_t)var;
    }

    return ret;
}

int libPredictor_predict_MABank2(   void   *state,
                                    int64_t *pu_c0, int64_t *pvar_c0,
                                    int64_t *pu_cl, int64_t *pvar_cl)
{
    int ret;

    MABank2_t *MABank2_p  = (MABank2_t*)state;
    int warmup = MABank2_p->warmup;

    double  *prediction = MABank2_p->prediction;
    double  *error_means= MABank2_p->error_mean;
    double  *error_vars = MABank2_p->error_var;

    int32_t min_var_f   = MABank2_p->min_var_f;
    
    double  u, var;
    
    if(warmup < 2)
    {
        /*There hasn't been sufficient observations to prodice a valid output*/
        ret = -1;
    }
    else
    {
        /*Should have a valid output*/
        ret = 0;
        
        /*The final output is the predictor output and the mean error for that 
          predictor */
        u   = prediction[min_var_f] + error_means[min_var_f];
        var = error_vars[min_var_f];
        
        /*Set the output variables*/
        *pu_c0      = (int64_t)u;
        *pvar_c0    = (int64_t)var;
        *pu_cl      = (int64_t)u;
        *pvar_cl    = (int64_t)var;        
    }
        
    return ret;
}

void libPredictor_free_MABank2( void    *state)
{
    free(state);
}

