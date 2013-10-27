#ifndef PBSPREDICTOR_INCLUDE
#define PBSPREDICTOR_INCLUDE

#include <stdint.h>
#include "MA.h"
#include "MABank.h"
#include "MAVSLMSBank.h"

typedef void* (*libPredictor_alloc_func_t)(void);

typedef void (*libPredictor_init_func_t)( void    *state);

typedef int (*libPredictor_update_func_t)(void    *state, int64_t exec_time,
                                    int64_t *pu_c0, int64_t *pvar_c0,
                                    int64_t *pu_cl, int64_t *pvar_cl);

typedef int (*libPredictor_predict_func_t)(void   *state,
                                    int64_t *pu_c0, int64_t *pvar_c0,
                                    int64_t *pu_cl, int64_t *pvar_cl);

typedef void (*libPredictor_free_func_t)( void    *state);

typedef struct libPredictor_s
{
    void *state;
    libPredictor_alloc_func_t     alloc;
    libPredictor_init_func_t      init;
    libPredictor_update_func_t    update;
    libPredictor_predict_func_t   predict;
    libPredictor_free_func_t      free;
} libPredictor_t;

int libPredictor_getPredictor(libPredictor_t *predictor, char *name);

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
