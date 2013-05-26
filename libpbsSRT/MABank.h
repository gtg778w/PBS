#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <float.h>

/*The forgetting factor for the exponential running average of error values
1 - 2^(-5) = 1 - 0.03125 = 0.96875*/
#define MABank_gamma (0.96875)

/*
    Have a bank of MA filters of length 0, 2, 4, 8, 16, 32, 64
*/

#define MABank_FCOUNT (7)
static const int32_t MABank_lengths[MABank_FCOUNT] = {2, 4, 8, 16, 32, 64, 128};
static const double  MABank_weights[MABank_FCOUNT] = {  (1/2.0), (1/4.0), (1/8.0),
                                                        (1/16.0), (1/32.0), (1/64.0),
                                                        (1/128.0)};

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

void* pbsSRT_alloc_MABank(  void);
void pbsSRT_init_MABank(void    *state);
int pbsSRT_update_MABank(   void    *state, int64_t exec_time,
                            int64_t *pu_c0, int64_t *pvar_c0,
                            int64_t *pu_cl, int64_t *pvar_cl);
int pbsSRT_predict_MABank(  void    *state,
                            int64_t *pu_c0, int64_t *pvar_c0,
                            int64_t *pu_cl, int64_t *pvar_cl);
void pbsSRT_free_MABank(void    *state);

