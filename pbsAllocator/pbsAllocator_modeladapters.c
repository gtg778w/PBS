#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
/*
memory mapping stuff
*/

#include "pbsAllocator.h"
#include "LMSVS.h"

LMSVS_t *perf_modeladapter_p;
LMSVS_t *powr_modeladapter_p;

void    *LAMbS_models_pages = NULL;

uint64_t    *perf_model_coeffs_u64 = NULL;
uint64_t    *power_model_coeffs_u64 = NULL;
uint64_t    *operation_mode_schedule_u64 = NULL;

double  *perf_model_coeffs_double = NULL;
double  *power_model_coeffs_double = NULL;

int pbsAllocator_modeladapters_init(int proc_file)
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

    /*Allocate space for the independant variables and model coefficients*/
    perf_model_coeffs_double = (double*)calloc(mo_count*2, sizeof(double));
    if(NULL == perf_model_coeffs_double)
    {
        perror( "pbsAllocator_modeladapters_init: calloc failed for "
                "perf_model_coeffs_double");
        goto error2;
    }
    power_model_coeffs_double= &(power_model_coeffs_double[mo_count]);

    /*Setup the LAMbS_models mapping*/
    LAMbS_models_pages = mmap(  NULL, LAMbS_MODELS_SIZE, 
                                (PROT_READ | PROT_WRITE), MAP_SHARED, 
                                proc_file, (LAMbS_MODELS_OFFSET * PAGE_SIZE));
    if(LAMbS_models_pages == MAP_FAILED)
    {
        perror("pbsAllocator_modeladapters_init: Failed to map LAMbS_models_pages");
        goto error3;
    }
    
    /*Set the u64 model array pointers to corresponding addresses in the mapped pages*/
    perf_model_coeffs_u64 = (uint64_t*)LAMbS_models_pages;
    power_model_coeffs_u64= &(perf_model_coeffs_u64[mo_count]);
    operation_mode_schedule_u64=&(power_model_coeffs_u64[mo_count]);
    
    return 0;
    
error3:
    free(perf_model_coeffs_double);
    perf_model_coeffs_double = NULL;
    power_model_coeffs_double = NULL;
error2:
    free(powr_modeladapter_p);
    powr_modeladapter_p = NULL;
error1:
    free(perf_modeladapter_p);
    perf_modeladapter_p = NULL;
error0:
    return -1;
}

void pbsAllocator_modeladapters_free(int proc_file)
{
    if(NULL != LAMbS_models_pages)
    {
        if(munmap(LAMbS_models_pages, LAMbS_MODELS_SIZE) != 0)
        {
            perror( "pbsAllocator_modeladapters_free: Failed to unmap LAMbS_models "
                    "pages!\n");
        }
        
        LAMbS_models_pages      = NULL;
        perf_model_coeffs_u64   = NULL;
        power_model_coeffs_u64  = NULL;
        operation_mode_schedule_u64 = NULL;
    }

    if(NULL != perf_model_coeffs_double)
    {
        free(perf_model_coeffs_double);
        perf_model_coeffs_double= NULL;
        power_model_coeffs_double   = NULL;
    }
    
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

