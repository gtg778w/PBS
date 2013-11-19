#ifndef PBSPREDICTOR_INCLUDE
#define PBSPREDICTOR_INCLUDE

#include <stdint.h>
#include "SRT.h"
#include "MA.h"
#include "MABank.h"
#include "MABank2.h"
#include "MAVSLMSBank.h"


int libPredictor_getPredictor(SRT_Predictor_t *predictor, char *name);

#define libPredictor_freePredictor(predictor) predictor->free(predictor->state)

void*   libPredictor_alloc_template(  void);

void    libPredictor_init_template(   void    *state);

int     libPredictor_update_template( void    *state, int64_t exec_time,
                                int64_t *pu_c0, int64_t *pvar_c0,
                                int64_t *pu_cl, int64_t *pvar_cl);
                                
int     libPredictor_predict_template(void    *state,
                                int64_t *pu_c0, int64_t *pvar_c0,
                                int64_t *pu_cl, int64_t *pvar_cl);
                                
void    libPredictor_free_template(   void    *state);

#endif
