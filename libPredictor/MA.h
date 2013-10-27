#ifndef MA_INCLUDE
#define MA_INCLUDE

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <float.h>

#define MA_LENGTH (20)

typedef struct MA_s
{
    int32_t warmup;
    
    double  mean;
    double  variance;
    double  x_buffer[MA_LENGTH];
} MA_t;

#define MA_size()   ( sizeof(MA_t) )

void*   libPredictor_alloc_MA(  void);
void    libPredictor_init_MA(   void    *state);

int libPredictor_update_MA( void    *state, int64_t exec_time,
                            int64_t *pu_c0, int64_t *pvar_c0,
                            int64_t *pu_cl, int64_t *pvar_cl);
                            
int libPredictor_predict_MA(void    *state,
                            int64_t *pu_c0, int64_t *pvar_c0,
                            int64_t *pu_cl, int64_t *pvar_cl);
                            
void    libPredictor_free_MA(   void    *state);

#endif
