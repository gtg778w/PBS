#include <stdint.h>
#include <stdlib.h>
#include "libpredictor.h"

void* libPredictor_alloc_template(void)
{
    /*No need to store any state, but need to return a non-NULL address to the state.
    This address will later be freed by free.*/
    return malloc(1);
}

void libPredictor_init_template(  void    *state)
{
}

int libPredictor_update_template( void    *state, int64_t exec_time,
                            int64_t *pu_c0, int64_t *pvar_c0,
                            int64_t *pu_cl, int64_t *pvar_cl)
{
    return -1;
}

int libPredictor_predict_template(void   *state,
                            int64_t *pu_c0, int64_t *pvar_c0,
                            int64_t *pu_cl, int64_t *pvar_cl)
{
    return -1;
}

void libPredictor_free_template(  void    *state){}

