#ifndef PBSPREDICTOR_INCLUDE
#define PBSPREDICTOR_INCLUDE

#include <stdint.h>
#include "MABank.h"

typedef void* (*pbsSRT_alloc_func_t)(void);

typedef void (*pbsSRT_init_func_t)( void    *state);

typedef int (*pbsSRT_update_func_t)(void    *state, int64_t exec_time,
                                    int64_t *pu_c0, int64_t *pvar_c0,
                                    int64_t *pu_cl, int64_t *pvar_cl);

typedef int (*pbsSRT_predict_func_t)(void   *state,
                                    int64_t *pu_c0, int64_t *pvar_c0,
                                    int64_t *pu_cl, int64_t *pvar_cl);

typedef void (*pbsSRT_free_func_t)( void    *state);

typedef struct pbsSRT_predictor_s
{
    void *state;
    pbsSRT_alloc_func_t     alloc;
    pbsSRT_init_func_t      init;
    pbsSRT_update_func_t    update;
    pbsSRT_predict_func_t   predict;
    pbsSRT_free_func_t      free;
} pbsSRT_predictor_t;

int pbsSRT_getPredictor(pbsSRT_predictor_t *predictor, char *name);

#define pbsSRT_freePredictor(predictor) predictor->free(predictor->state)

void*   pbsSRT_alloc_template(  void);

void    pbsSRT_init_template(   void    *state);

int     pbsSRT_update_template( void    *state, int64_t exec_time,
                                int64_t *pu_c0, int64_t *pvar_c0,
                                int64_t *pu_cl, int64_t *pvar_cl);
                                
int     pbsSRT_predict_template(void    *state,
                                int64_t *pu_c0, int64_t *pvar_c0,
                                int64_t *pu_cl, int64_t *pvar_cl);
                                
void    pbsSRT_free_template(   void    *state);

#endif
