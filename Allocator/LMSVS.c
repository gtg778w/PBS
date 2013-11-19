#include <string.h>
#include "LMSVS.h"

void LMSVS_init(LMSVS_t *LMSVS_p, int32_t taps_in)
{
    uint32_t i;
    
    /*zero out everything, including the buffers*/
    bzero(LMSVS_p, LMSVS_size(taps_in));

    /*LMSVS_p->prediction = 0.0;*/
    LMSVS_p->taps = taps_in;

    /*Set up ALL the buffers consecutively after the coefficients array
    
    coefficients:   stages * LMSVSVL_TPS * double
    buffer:         stages * LMSVSVL_TPS * double
    step_size:      stages * LMSVSVL_TPS * double
    
    previous_sign:  stages * LMSVSVL_TPS * int8_t
    schange_count:  stages * LMSVSVL_TPS * int8_t
    */

    /*Set up the buffers consecutively after the coefficients array*/
    LMSVS_p->previous_sign  = (int8_t*)&(LMSVS_p->step_size[taps_in]);
    
    LMSVS_p->schange_count  = &(LMSVS_p->previous_sign[taps_in]);
    
    for(i = 0; i < taps_in; i++)
    {
        LMSVS_p->step_size[i] = LMSVS_MAX_STEPSIZE;
    }
}

static void inline LMSVS_update_stepsize(  int8_t *previous_sign_p,
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
    factor of LMSVS_alpha*/
    if(gradient_sign_changed == 0)
    {
        /*No sign change has occured*/
        schange_count = (schange_count > 0)? -1 : schange_count-1;
    
        /*Check if m1 consecutive identical signs occured*/
        if(schange_count <= -LMSVS_m1)
        {
            schange_count = -LMSVS_m1;
            /*Pick the minimum of (step-size * alpha) and LMSVS_MAX_STEPSIZE*/
            step_size = (step_size < LMSVS_MAX_STEPSIZE)?
                        (step_size * LMSVS_alpha):
                        LMSVS_MAX_STEPSIZE;
        }
    }
    else
    {
        /*Sign change occured*/
        schange_count = (schange_count < 0)? 1 : schange_count+1;

        /*Check if m0 consecutive sign changes have occured*/
        if(schange_count >= LMSVS_m0)
        {
            schange_count = LMSVS_m0;
            /*Pick the maximum of (step-size * 1/alpha) and LMSVS_MIN_STEPSIZE*/
            step_size = (step_size > LMSVS_MIN_STEPSIZE)?
                        (step_size * LMSVS_1overalpha):
                        LMSVS_MIN_STEPSIZE;
        }
    }
    *schange_count_p = schange_count;
    *step_size_p = step_size;
}

double LMSVS_predict(LMSVS_t *LMSVS_p, double *x_hat, double *coeffs)
{
    int32_t i;
    int32_t taps = LMSVS_p->taps;

    double y_hat = 0;
    
    for(i = 0; i < taps; i++)
    {
        y_hat = y_hat+ x_hat[i] * coeffs[i];
    }
    
    return y_hat;
}

void LMSVS_update(LMSVS_t *LMSVS_p, double *x_hat, double *coeffs, double observed)
{
    int32_t i;
    int32_t taps = LMSVS_p->taps;
    
    double den;
    
    double next_indep_var;
    
    /* compute the error*/
    double y_hat;
    double error;

    double gradient;

    /*compute the denominator and y_hat*/
    y_hat = 0.0;
    den = LMSVS_EPS;
    for(i = 0; i < taps; i++)
    {
        next_indep_var = x_hat[i];
        
        y_hat = y_hat + (next_indep_var * coeffs[i]);
        
        den = den + next_indep_var * next_indep_var;
    }

    /*compute the error*/
    error = observed - y_hat;
    
    /* update coefficients and step sizes */
    for(i = 0; i < taps; i++)
    {
        /* update step-size[i] */
        next_indep_var = x_hat[i];
        gradient = -error * next_indep_var;
        LMSVS_update_stepsize( &(LMSVS_p->previous_sign[i]),
                                &(LMSVS_p->schange_count[i]),
                                &(LMSVS_p->step_size[i]),
                                gradient);

        /* compute coefficient[i] */
        coeffs[i] += ((LMSVS_p->step_size[i])*error*next_indep_var)/den;
    }
}

