#include <stdio.h>
#include <strings.h>
#include "MA.h"
#include "libPredictor_variance.h"

void* libPredictor_alloc_MA(void)
{
    return malloc(MA_size());
}

void libPredictor_init_MA(  void    *state)
{
    memset(state, 0, MA_size());
}

int libPredictor_update_MA( void    *state, int64_t exec_time,
                            int64_t *pu_c0, int64_t *pvar_c0,
                            int64_t *pu_cl, int64_t *pvar_cl)
{
    int ret = 0;

    /*Extract parts of the predictor state*/
    MA_t *MA_p  = (MA_t*)state;
    int32_t warmup  = MA_p->warmup;
    double  *x_buffer   = MA_p->x_buffer;
    
    /*Actual buffer length*/
    int32_t limit;
    
    double mean, variance;
    
    /*Check the amount of history data available*/
    if(warmup < MA_LENGTH)
    {
        /*increment the warmup counter*/
        warmup++;
        limit = warmup;
        
        /*If there is not enough history available, the predictor is not ready*/
        if(warmup < 2)
        {
            ret = -1;
        }
    }
    else
    {
        /*the history is filled*/
        limit = MA_LENGTH;
    }
    
    libPredictor_dvariance((double)exec_time,
                           limit, x_buffer,
                           &mean, &variance);

    /*Update the MA data structure*/
    MA_p->warmup= warmup;
    MA_p->mean  = mean;
    MA_p->variance  = variance;

    /*Set the output variables*/
    *pu_c0      = (int64_t)mean;
    *pvar_c0    = (int64_t)variance;
    *pu_cl      = (int64_t)mean;
    *pvar_cl    = (int64_t)variance;
    
    return ret;
}

int libPredictor_predict_MA(void   *state,
                            int64_t *pu_c0, int64_t *pvar_c0,
                            int64_t *pu_cl, int64_t *pvar_cl)
{
    MA_t    *MA_p   = (MA_t*)state;
    double  mean    = MA_p->mean;
    double  variance= MA_p->variance;
    int warmup = MA_p->warmup;
    
    int ret;

    /*Set the output variables*/
    *pu_c0      = (int64_t)mean;
    *pvar_c0    = (int64_t)variance;
    *pu_cl      = (int64_t)mean;
    *pvar_cl    = (int64_t)variance;

    ret = (warmup < 2)? -1 : 0;
        
    return ret;
}

void libPredictor_free_MA(  void    *state)
{
    free(state);
}

