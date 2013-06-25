#include <stdlib.h>
#include <stdio.h>

#include "pbsAllocator.h"
#include "LMSVS.h"

LMSVS_t *perf_modeladapter_p;
LMSVS_t *powr_modeladapter_p;

int pbsAllocator_modeladapters_init(void)
{
    int mo_count = loaddata_list_header->mo_count;
    
    perf_modeladapter_p = LMSVS_malloc(mo_count);
    if(NULL == perf_modeladapter_p)
    {
        perror( "pbsAllocator_modeladapters_init: LMSVS_malloc failed for "
                "perf_modeladapter_p");
        goto error0;
    }

    LMSVS_init(perf_modeladapter_p, mo_count);

    powr_modeladapter_p = LMSVS_malloc(mo_count);
    if(NULL == powr_modeladapter_p)
    {
        perror("pbsAllocator_modeladapters_init: LMSVS_malloc failed for "
                "powr_modeladapter_p");
        goto error1;
    }

    LMSVS_init(powr_modeladapter_p, mo_count);
    
    return 0;

error2:
    free(powr_modeladapter_p);
    powr_modeladapter_p = NULL;
error1:
    free(perf_modeladapter_p);
    perf_modeladapter_p = NULL;
error0:
    return -1;
}

void pbsAllocator_modeladapters_free(void)
{
    if(NULL != powr_modeladapter_p)
    {
        free(powr_modeladapter_p);
        powr_modeladapter_p = NULL;
    }
    
    if(NULL != perf_modeladapter_p)
    {
        free(perf_modeladapter_p);
        perf_modeladapter_p = NULL;
    }
}

void pbsAllocator_modeladapters_adapt(void)
{
    
}

