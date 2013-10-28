#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <float.h>

#define MABank2_FCOUNT (7)
static const int32_t MABank2_lengths[MABank2_FCOUNT] = {2, 4, 8, 16, 32, 64, 128};
/* 2 + 4 + 8 + 16 + 32 + 64 + 128 = 254*/
#define MABank2_ERRORBUF_LEN (254)

typedef struct MABank2_s
{
    /*The amount of history available*/
    int32_t warmup; /*4*/
    
    /*The index of the minimum variance filter*/
    int32_t min_var_f; /*4*/
    
    /*The value prdicted by each filter*/
    double  prediction[MABank2_FCOUNT]; /*56*/
    
    /*The error buffer and error statistics associated with each filter*/
    double  error_mean[MABank2_FCOUNT]; /*56*/
    double  error_var[MABank2_FCOUNT];  /*56*/
    float   *error_buffer[MABank2_FCOUNT]; /*56*/
    
    /*to allow this to be a variable length structure
      to handle a variable number of taps*/
    double  x_buffer[1]; /*1024*/
} MABank2_t; /*1016*/

/*
    data buffer: 32
    step_size, coefficients: 32 + 16 + 8 + 4 + 2
    previous_sign, schange_count:  32 + 16 + 8 + 4 + 2
*/
#define MABank2_size()    (   sizeof(MABank2_t) + \
                             (sizeof(double)*(MABank2_lengths[MABank2_FCOUNT-1])) + \
                             (sizeof(float)*MABank2_ERRORBUF_LEN) )

void*   libPredictor_alloc_MABank2( void);
void    libPredictor_init_MABank2(  void    *state);
int libPredictor_update_MABank2(void    *state, int64_t exec_time,
                                int64_t *pu_c0, int64_t *pvar_c0,
                                int64_t *pu_cl, int64_t *pvar_cl);
int libPredictor_predict_MABank2(   void    *state,
                                    int64_t *pu_c0, int64_t *pvar_c0,
                                    int64_t *pu_cl, int64_t *pvar_cl);
void libPredictor_free_MABank2(     void    *state);

