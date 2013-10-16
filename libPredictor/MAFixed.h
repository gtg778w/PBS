#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <float.h>

#define MA_LENGTH (20)

typedef struct MA_s
{
    int32_t warmup;
    
    double  x_hat;
    double  prediction;
    double  error_mean;
    double  error_var;
    double  buffer[MA_LENGTH];
} MA_t;

/*
    data buffer: 32
    step_size, coefficients: 32 + 16 + 8 + 4 + 2
    previous_sign, schange_count:  32 + 16 + 8 + 4 + 2
*/
#define MA_size()    (   sizeof(MA_t) + \
                             (sizeof(double)*(MA_lengths[MA_FCOUNT-1])))

void* pbsSRT_alloc_MA(  void);
void pbsSRT_init_MA(    void    *state);
int pbsSRT_update_MA(   void    *state, int64_t exec_time,
                        int64_t *pu_c0, int64_t *pvar_c0,
                        int64_t *pu_cl, int64_t *pvar_cl);
int pbsSRT_predict_MA(  void    *state,
                        int64_t *pu_c0, int64_t *pvar_c0,
                        int64_t *pu_cl, int64_t *pvar_cl);
void pbsSRT_free_MA(    void    *state);

