#include <stdlib.h>
#include <string.h>
#include "MAVSLMSBank.h"

/*The forgetting factor for the exponential running average of error values
1 - 2^(-5) = 1 - 0.03125 = 0.96875*/
#define MAVSLMSBank_gamma (0.96875)


/*Various constant parameters used by the VSLMS filter*/
#define VSLMSBank_EPS (1)

#define VSLMSBank_MAX_STEPSIZE (1.5)
#define VSLMSBank_MIN_STEPSIZE (0.0125)
#define VSLMSBank_alpha (2.0)
#define VSLMSBank_1overalpha (1/VSLMSBank_alpha)

/*The m0 and m1 thresholds should not
be more than 127 so that the sign counter
can be stored in 1 byte of storage*/
#define VSLMSBank_m0 (2)
#define VSLMSBank_m1 (3)

/*
    Have a bank of LMS filters of length 0, 2, 4, 8, 16, and 32:
*/
static const int32_t VSLMSBank_lengths[VSLMSBank_FCOUNT] = {2, 4, 8, 16, 32};

static const int32_t MABank_lengths[MABank_FCOUNT]  = {0, 2, 4, 8, 16, 32, 64, 128};
static const double  MABank_weights[MABank_FCOUNT] = {  (0.0), (1/2.0), (1/4.0), 
                                                        (1/8.0), (1/16.0), (1/32.0), 
                                                        (1/64.0), (1/128.0) };

void* libPredictor_alloc_MAVSLMSBank(   void)
{
    return malloc(MAVSLMSBank_size());
}

static void inline VSLMSBank_init(MAVSLMSBank_t *MAVSLMSBank_p)
{
    int32_t fid, i; 

    double *double_buffer_address;
    int8_t *byte_buffer_address;
    int32_t buffer_length;
    
    /*Set up ALL the buffers consecutively after the 
    "buffer" field of the MAVSLMSBank struct
    
    buffer:             128 * double
    
    fid = VSLMSBank_FCOUNT-1;
    
    coefficients_p[fid]:  VSLMSBank_lengths[fid] * double
    step_size_p[fid]:     VSLMSBank_lengths[fid] * double
    fid--;
    ...    
    
    fid--; //fid = 0
    coefficients_p[fid]:  VSLMSBank_lengths[fid] * double
    step_size_p[fid]:     VSLMSBank_lengths[fid] * double
    
    fid = VSLMSBank_FCOUNT-1;
    
    previous_sign_p[fid]: VSLMSBank_lengths[fid] * int8_t
    schange_count_p[fid]: VSLMSBank_lengths[fid] * int8_t
    ...
    fid--; //fid = 0
    previous_sign_p[fid]: VSLMSBank_lengths[fid] * int8_t
    previous_sign_p[fid]: VSLMSBank_lengths[fid] * int8_t
    */
        
    /*Setup the buffers for the longest filter right after the data buffer*/
    double_buffer_address = &(MAVSLMSBank_p->buffer[128]);
    buffer_length = 0;

    for(fid = (VSLMSBank_FCOUNT-1); fid >= 0; fid--)
    {
        /*Assign coefficient buffers and step-size buffers for filter fid*/
        MAVSLMSBank_p->coefficients_p[fid] = &(double_buffer_address[buffer_length]);
        double_buffer_address = MAVSLMSBank_p->coefficients_p[fid];

        /*The buffer length for filter fid*/
        buffer_length = VSLMSBank_lengths[fid];
                        
        MAVSLMSBank_p->step_size_p[fid] = &(double_buffer_address[buffer_length]);
        double_buffer_address = MAVSLMSBank_p->step_size_p[fid];
        
        /*Initialize the step size buffers*/
        for(i = 0; i < buffer_length; i++)
        {
            (MAVSLMSBank_p->step_size_p[fid])[i] = VSLMSBank_MAX_STEPSIZE;
        }
    }

    /*Setup the byte buffers right after the double buffers*/
    byte_buffer_address = (int8_t*)&(double_buffer_address[buffer_length]);
    buffer_length = 0;
    
    for(fid = (VSLMSBank_FCOUNT-1); fid >= 0; fid--)
    {
        /*Assign previous_sign buffers and schange_count buffers for filter fid*/
        MAVSLMSBank_p->previous_sign_p[fid] = &(byte_buffer_address[buffer_length]);
        byte_buffer_address = MAVSLMSBank_p->previous_sign_p[fid];
        
        /*The buffer length for filter fid*/
        buffer_length = VSLMSBank_lengths[fid];
                        
        MAVSLMSBank_p->schange_count_p[fid] = &(byte_buffer_address[buffer_length]);
        byte_buffer_address = MAVSLMSBank_p->schange_count_p[fid];
    }

    /*Initialize the variance of all the filters to DBL_MAX*/
    for(fid = (VSLMSBank_FCOUNT-1); fid >= 0; fid--)
    {
        MAVSLMSBank_p->vslms_error_var[fid] = DBL_MAX;
    }
}

static void inline MABank_init(MAVSLMSBank_t *MAVSLMSBank_p)
{
    int32_t fid;
    
    /*Initialize the variance of all the filters to DBL_MAX*/
    for(fid = (MABank_FCOUNT-1); fid >= 0; fid--)
    {
        MAVSLMSBank_p->ma_error_var[fid] = DBL_MAX;
    }
}

void libPredictor_init_MAVSLMSBank( void    *state)
{
    MAVSLMSBank_t *MAVSLMSBank_p = (MAVSLMSBank_t*)state;
    
    /*zero out everything, including the buffers*/
    bzero(MAVSLMSBank_p, MAVSLMSBank_size());

    /*initialize structures related to the VSLMS filters*/
    VSLMSBank_init(MAVSLMSBank_p);
    
    /*initialize structures related to the MA filters*/
    MABank_init(MAVSLMSBank_p);
}

static void inline VSLMSBank_update_stepsize(   int8_t *previous_sign_p,
                                                int8_t *schange_count_p,
                                                double *step_size_p,
                                                double gradient)
{
    int8_t gradient_newsign;
    int8_t gradient_prevsign;
    int8_t gradient_sign_changed;
    int8_t schange_count;

    double step_size;
        
    /*Determine the sign of the gradient*/
    gradient_newsign    = (gradient < 0)? -1 : 0;
    gradient_newsign    = (gradient > 0)? 1 : gradient_newsign;
    gradient_prevsign   = *previous_sign_p;
    *previous_sign_p    = gradient_newsign;
    
    /*gradient_sign_changed should be 0 in case of no sign change or nonzero otherwise*/
    gradient_sign_changed = gradient_prevsign - gradient_newsign;
    
    /*positive schange count means sign changes. negative schange count means same signs*/
    schange_count   = *schange_count_p;
    step_size       = *step_size_p;
    
    /*Check is m0 or m1 consecutive sign changes or consecutive same signs occured 
    respectively. If so, then increase or decrase respectively, the step-size by a
    factor of VSLMSBank_alpha*/
    if(gradient_sign_changed == 0)
    {
        /*No sign change has occured*/
        schange_count = (schange_count > 0)? -1 : schange_count-1;
        
        /*Check if m1 consecutive identical signs occured*/
        if(schange_count <= -VSLMSBank_m1)
        {
            schange_count = -VSLMSBank_m1;
            /*Pick the minimum of (step-size * alpha) and VSLMSBank_MAX_STEPSIZE*/
            step_size = (step_size < VSLMSBank_MAX_STEPSIZE)? 
                        (step_size * VSLMSBank_alpha):
                        VSLMSBank_MAX_STEPSIZE;
        }
    }
    else
    {
        /*Sign change occured*/
        schange_count = (schange_count < 0)? 1 : schange_count+1;

        /*Check if m0 consecutive sign changes have occured*/
        if(schange_count >= VSLMSBank_m0)
        {
            schange_count = VSLMSBank_m0;
            /*Pick the maximum of (step-size * 1/alpha) and VSLMSBank_MIN_STEPSIZE*/
            step_size = (step_size > VSLMSBank_MIN_STEPSIZE)? 
                        (step_size * VSLMSBank_1overalpha):
                        VSLMSBank_MIN_STEPSIZE;
        }
    }
    *schange_count_p = schange_count;
    *step_size_p = step_size;
}

static void inline VSLMSBank_update(MAVSLMSBank_t *MAVSLMSBank_p, double observed)
{
    int32_t fid;
    int32_t i_den, i;

    double  error;
    
    double  *error_mean = MAVSLMSBank_p->vslms_error_mean;
    double  error_diff;
    double  *error_var = MAVSLMSBank_p->vslms_error_var;
    
    double  min_var;
    int32_t min_var_f;
    
    double  den;
    double  next_indep_var;    
    
    double  *coefficients;
    double  *step_size;
    int8_t  *previous_sign;
    int8_t  *schange_count;
    double  gradient;
    
    /*Initially set the minimum varriance to the maximum possible value of a double,
    and set the minimum varriance filter to the invalid value -1*/
    min_var = DBL_MAX;
    min_var_f = 0;
    
    /*initialize the variables used in computing the normalizing denominator.
    Note that the normalizing the denominator for a larger-length filter
    is equal to the denominator for a smaller-length filter plus additional
    sum-of-square terms*/
    i_den = 0;
    den = 1.0;
    
    for(fid = 0; fid < VSLMSBank_FCOUNT; fid++)
    {
        /*Check if the system has warmed up given the filter length*/
        if(MAVSLMSBank_p->warmup >= VSLMSBank_lengths[fid])
        {
            
            /*Compute the error*/
            error = observed - MAVSLMSBank_p->vslms_prediction[fid];
            
            /*Compute the vediation in the error from the mean*/
            error_diff = error - error_mean[fid];
            
            /*Is this the first computation of the error varriance?*/
            if(DBL_MAX == error_var[fid])
            {
                error_mean[fid] = error;
                error_var[fid]  = error_diff * error_diff;
            }
            else
            {
                /*Update the error statistics*/
                error_mean[fid] = MAVSLMSBank_gamma * error_mean[fid]
                                        + (1 - MAVSLMSBank_gamma) * error;
                error_var[fid]  = MAVSLMSBank_gamma * error_var[fid]
                                        + (1 - MAVSLMSBank_gamma) * error_diff * error_diff;
            }
            
            /*Check if this is the minimum variance filter*/
            min_var_f = (min_var > error_var[fid])? fid : min_var_f;
            min_var   = (min_var > error_var[fid])? error_var[fid] : min_var;
            
            /*Accumulate the additional necessary terms in the normalizing denominator*/
            for(; i_den < VSLMSBank_lengths[fid]; i_den++)
            {
                next_indep_var = MAVSLMSBank_p->buffer[i_den];
                den = den + next_indep_var * next_indep_var;
            }
            
            /*Get the relevant buffers for the current filter*/
            coefficients    = MAVSLMSBank_p->coefficients_p[fid];
            step_size       = MAVSLMSBank_p->step_size_p[fid];
            previous_sign   = MAVSLMSBank_p->previous_sign_p[fid];
            schange_count   = MAVSLMSBank_p->schange_count_p[fid];

            /*Update the step size and adapt the weights of the filter*/
            for(i = 0; i < VSLMSBank_lengths[fid]; i++)
            {
                /* update the step-size */
                next_indep_var = MAVSLMSBank_p->buffer[i];
                gradient = -error * next_indep_var;
                VSLMSBank_update_stepsize(  &(previous_sign[i]),
                                            &(schange_count[i]),
                                            &(step_size[i]),
                                            gradient);

                /*update the weights*/
                coefficients[i] = coefficients[i] + 
                                    ((step_size[i])*error*next_indep_var)/den;

            }            

        }/*if(MAVSLMSBank_p->warmup >= VSLMSBank_lengths[fid])*/
    }/*for(fid = 0; fid < VSLMSBank_FCOUNT; fid++)*/

    MAVSLMSBank_p->vslms_minvar_f = min_var_f;
}

/*The MABank_shift_predict function should be called after the VSLMSBank_update function*/
static void inline MABank_shift_predict(MAVSLMSBank_t *MAVSLMSBank_p, double observed)
{
    int32_t fid;
    int32_t i;
    
    double  *prediction = MAVSLMSBank_p->ma_prediction;
    double  *error_var  = MAVSLMSBank_p->ma_error_var;
    double  *buffer     = MAVSLMSBank_p->buffer;
    
    double  next_indep_var;
    double  prev_indep_var;

    double  error;
    double  error_diff;

    double  filter_acc;
    
    /*Initially set the minimum varriance to the maximum possible value of a double,
    and set the minimum varriance filter to the invalid value -1*/
    double  min_var = DBL_MAX;
    int32_t min_var_f = 0;
    
    /*perform the convolution for the longest filter AND update the history by shifting
    the new observed value into the registers*/
    next_indep_var = observed;
    filter_acc = 0;
    i = 0;
    for(fid = 0; fid < MABank_FCOUNT; fid++)
    {
        /*Only update the variance if the system has warmedup*/
        if(MAVSLMSBank_p->warmup >= MABank_lengths[fid])
        {

            /*Compute the error and error statistics for filter fid*/
            error = observed - prediction[fid];
            
            /*For the moving average filters, it is assumed that the moving average is the 
            predicted value. Therefore, the prediction error is deviation from the moving
            average, and the error has zero mean. As a result, the mean-squared error value 
            is also the error standard deviation*/
            error_diff = error;
            
            if(DBL_MAX == error_var[fid])
            {
                error_var[fid]  = error_diff * error_diff;
            }
            else
            {
                error_var[fid]  = MAVSLMSBank_gamma * error_var[fid] + 
                                        (1 - MAVSLMSBank_gamma) * error_diff * error_diff;
            }
            
            /*Determine if this is the minimum variance filter*/
            min_var_f   =   (error_var[fid] < min_var)?
                            fid             : min_var_f;
            min_var     =   (error_var[fid] < min_var)?  
                            error_var[fid]  : min_var;
        }
        
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

    /*Store the id of the new minimum variance filter (in case it changed)*/                        
    MAVSLMSBank_p->ma_minvar_f = min_var_f;
}

/*The VSLMSBank_predict function should be called after the MABank_shift_predict function*/
static void inline VSLMS_predict(MAVSLMSBank_t *MAVSLMSBank_p)
{
    int32_t fid;
    int32_t i;

    double *coefficients;
    double filter_acc;

    /*Perform the filtering operation for the LMS filters,
    the buffer has been already updated so no more shifting necessary*/
    for(fid = 0; fid < VSLMSBank_FCOUNT; fid++)
    {
        coefficients = MAVSLMSBank_p->coefficients_p[fid];
        filter_acc = 0.0;

        for(i = 0; i < VSLMSBank_lengths[fid]; i++)
        {
            /*Filter MAC operation*/
            filter_acc = filter_acc + (MAVSLMSBank_p->buffer[i] * coefficients[i]);
        }
       
        /*Store the filtered value*/
        MAVSLMSBank_p->vslms_prediction[fid] = filter_acc;
    }
}

/*
Loop through each filter
    for each filter, extract the appropriate buffers and update the coefficients
*/
int libPredictor_update_MAVSLMSBank(void    *state, int64_t exec_time,
                                    int64_t *pu_c0, int64_t *pvar_c0,
                                    int64_t *pu_cl, int64_t *pvar_cl)
{
    int ret;
    
    MAVSLMSBank_t *MAVSLMSBank_p = (MAVSLMSBank_t*)state;
    double observed = (double)exec_time;
    
    double ma_minvar;
    double vslms_minvar;
    
    double xhat_ma;
    double xhat_vslms;
    
    MAVSLMSBank_ftype_t minvar_ftype;
    
    /*Update the VSLMS error statistics, step sizes, andfilter coefficients*/
    VSLMSBank_update(MAVSLMSBank_p, observed);
    
    /*Shoft the newly observed value into the buffer and computed the 
    predicted value for the MA filters*/
    MABank_shift_predict(MAVSLMSBank_p, observed);
    
    /*Computed the predicted value for the VSLMS filters*/
    VSLMS_predict(MAVSLMSBank_p);

    /*Once the observed value is shifted into the buffer, check if the filter is still 
    warming up and increment the warmup counter*/
    MAVSLMSBank_p->warmup = 
                        (MAVSLMSBank_p->warmup <= 128)?
                        (MAVSLMSBank_p->warmup+1) : MAVSLMSBank_p->warmup;
    
    /*Use the minimum-error-varriance filter output for the final x_hat prediction value*/
    ma_minvar   = MAVSLMSBank_p->ma_error_var[MAVSLMSBank_p->ma_minvar_f];
    vslms_minvar= MAVSLMSBank_p->vslms_error_var[MAVSLMSBank_p->vslms_minvar_f];
    
    xhat_vslms  = MAVSLMSBank_p->vslms_prediction[MAVSLMSBank_p->vslms_minvar_f]
                    + MAVSLMSBank_p->vslms_error_mean[MAVSLMSBank_p->vslms_minvar_f];
    xhat_ma     = MAVSLMSBank_p->ma_prediction[MAVSLMSBank_p->ma_minvar_f];
    
    minvar_ftype =  ((vslms_minvar < ma_minvar) && (xhat_vslms > 0))?
                    MAVSLMSBank_VSLMS:
                    MAVSLMSBank_MA;
    
    MAVSLMSBank_p->minvar_ftype = minvar_ftype;
    MAVSLMSBank_p->x_hat =  (minvar_ftype == MAVSLMSBank_MA)?  
                            xhat_ma : 
                            xhat_vslms;
    
    *pu_c0  = (int64_t) MAVSLMSBank_p->x_hat;
    *pvar_c0= (int64_t) ((ma_minvar <= vslms_minvar)? ma_minvar : vslms_minvar);
    
    *pu_cl  = (int64_t) xhat_ma;
    *pvar_cl= (int64_t) ma_minvar;
    
    ret = (MAVSLMSBank_p->warmup < 3)? -1 : 0;
    
    return ret;
}

int libPredictor_predict_MAVSLMSBank(   void    *state,
                                        int64_t *pu_c0, int64_t *pvar_c0,
                                        int64_t *pu_cl, int64_t *pvar_cl)
{
    int ret;
    
    MAVSLMSBank_t *MAVSLMSBank_p = (MAVSLMSBank_t*)state;

    double ma_minvar;
    double vslms_minvar;
    
    double xhat_ma;
    
    /*Use the minimum-error-varriance filter output for the final x_hat prediction value*/
    ma_minvar   = MAVSLMSBank_p->ma_error_var[MAVSLMSBank_p->ma_minvar_f];
    vslms_minvar= MAVSLMSBank_p->vslms_error_var[MAVSLMSBank_p->vslms_minvar_f];
    
    xhat_ma     = MAVSLMSBank_p->ma_prediction[MAVSLMSBank_p->ma_minvar_f];
    
    *pu_c0  = (int64_t) MAVSLMSBank_p->x_hat;
    *pvar_c0= (int64_t) ((ma_minvar <= vslms_minvar)? ma_minvar : vslms_minvar);
    
    *pu_cl  = (int64_t) xhat_ma;
    *pvar_cl= (int64_t) ma_minvar;
    
    ret = (MAVSLMSBank_p->warmup < 3)? -1 : 0;
    
    return ret;
}

void libPredictor_free_MAVSLMSBank( void    *state)
{
    free(state);
}

