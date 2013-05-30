#ifndef MAVSLMSBank_INCLUDE
#define MAVSLMSBank_INCLUDE

#include <stdlib.h>
#include <stdint.h>
#include <float.h>

/*Enums and macros for distinguishing between the MA filter and VSLMS filters*/
typedef enum 
{
    MAVSLMSBank_VSLMS,
    MAVSLMSBank_MA
} MAVSLMSBank_ftype_t;

#define VSLMSBank_FCOUNT (5)
#define MABank_FCOUNT (8)

typedef struct MAVSLMSBank_s
{
    int32_t warmup;
    
    int32_t ma_minvar_f;
    int32_t vslms_minvar_f;
    MAVSLMSBank_ftype_t minvar_ftype;

    double  x_hat;
        
    double  ma_prediction[MABank_FCOUNT];
    double  ma_error_var[MABank_FCOUNT];
    
    double  vslms_prediction[VSLMSBank_FCOUNT];
    double  vslms_error_mean[VSLMSBank_FCOUNT];
    double  vslms_error_var[VSLMSBank_FCOUNT];
        
    /*positive values indiciate consecutive sign changes*/
    /*negative values indiciate consecutive fixed sign values*/
    double  *step_size_p[VSLMSBank_FCOUNT];
    int8_t  *previous_sign_p[VSLMSBank_FCOUNT];
    int8_t  *schange_count_p[VSLMSBank_FCOUNT];

    /*to allow this to be a variable length structure
      to handle a variable number of taps*/
    double  *coefficients_p[VSLMSBank_FCOUNT];
    double  buffer[1];
} MAVSLMSBank_t;

/*
    data buffer: 128
    step_size, coefficients: 32 + 16 + 8 + 4 + 2
    previous_sign, schange_count:  32 + 16 + 8 + 4 + 2
*/
#define MAVSLMSBank_size()  ( sizeof(MAVSLMSBank_t) + \
                              (sizeof(double)*(128)) + \
                              (sizeof(double)*(32 + 16 + 8 + 4 + 2)*(2)) + \
                              (sizeof(int8_t)*(32 + 16 + 8 + 4 + 2)*(2)))

void* pbsSRT_alloc_MAVSLMSBank( void);
void pbsSRT_init_MAVSLMSBank(   void    *state);
int pbsSRT_update_MAVSLMSBank(  void    *state, int64_t exec_time,
                                int64_t *pu_c0, int64_t *pvar_c0,
                                int64_t *pu_cl, int64_t *pvar_cl);
int pbsSRT_predict_MAVSLMSBank( void    *state,
                                int64_t *pu_c0, int64_t *pvar_c0,
                                int64_t *pu_cl, int64_t *pvar_cl);
void pbsSRT_free_MAVSLMSBank(   void    *state);

#endif
