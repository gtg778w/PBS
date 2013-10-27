#ifndef MABANK_INCLUDE
#define MABANK_INCLUDE

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <float.h>

typedef struct MABank_s
{
    int32_t warmup;
    
    int32_t min_var_f;
    double  x_hat;
    double  prediction[7];
    double  error_mean[8];
    double  error_var[8];
    
    /*to allow this to be a variable length structure
      to handle a variable number of taps*/
    double  buffer[1];
} MABank_t;

/*
    data buffer: 32
    step_size, coefficients: 32 + 16 + 8 + 4 + 2
    previous_sign, schange_count:  32 + 16 + 8 + 4 + 2
*/
#define MABank_size()    (   sizeof(MABank_t) + \
                             (sizeof(double)*(MABank_lengths[MABank_FCOUNT-1])))

void*   libPredictor_alloc_MABank(  void);
void    libPredictor_init_MABank(   void    *state);
int libPredictor_update_MABank( void    *state, int64_t exec_time,
                                int64_t *pu_c0, int64_t *pvar_c0,
                                int64_t *pu_cl, int64_t *pvar_cl);
int libPredictor_predict_MABank(void    *state,
                                int64_t *pu_c0, int64_t *pvar_c0,
                                int64_t *pu_cl, int64_t *pvar_cl);
void libPredictor_free_MABank(  void    *state);

#endif
