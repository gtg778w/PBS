#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
/*
memory mapping stuff
*/

#include "Allocator.h"
#include "LMSVS.h"

LMSVS_t *perf_modeladapter_p;
LMSVS_t *powr_modeladapter_p;

void    *LAMbS_models_pages = NULL;

volatile uint64_t    *perf_model_coeffs_u64 = NULL;
volatile uint64_t    *perf_model_coeffs_inv_u64 = NULL;
volatile uint64_t    *operation_mode_schedule_u64 = NULL;

double  *perf_model_coeffs_double = NULL;
double  *power_model_coeffs_double = NULL;
double  *mostat_double = NULL;

int     max_perf_coeff_moi;
double  max_perf_coeff;

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
    perf_model_coeffs_double = (double*)calloc(mo_count*3, sizeof(double));
    if(NULL == perf_model_coeffs_double)
    {
        perror( "pbsAllocator_modeladapters_init: calloc failed for "
                "perf_model_coeffs_double");
        goto error2;
    }
    power_model_coeffs_double= &(perf_model_coeffs_double[mo_count]);
    mostat_double = &(power_model_coeffs_double[mo_count]);

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
    perf_model_coeffs_u64       = (uint64_t*)LAMbS_models_pages;
    perf_model_coeffs_inv_u64   = &(perf_model_coeffs_u64[mo_count]);
    operation_mode_schedule_u64 = &(perf_model_coeffs_inv_u64[mo_count]);
    
    return 0;
    
error3:
    free(perf_model_coeffs_double);
    perf_model_coeffs_double = NULL;
    power_model_coeffs_double = NULL;
    mostat_double = NULL;
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
        perf_model_coeffs_inv_u64   = NULL;
        operation_mode_schedule_u64 = NULL;
    }

    if(NULL != perf_model_coeffs_double)
    {
        free(perf_model_coeffs_double);
        perf_model_coeffs_double    = NULL;
        power_model_coeffs_double   = NULL;
        mostat_double = NULL;
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

void pbsAllocator_modeladapters_adapt(double *est_icount_p, double *est_energy_p)
{
    int moi;
    int mocount = loaddata_list_header->mo_count;
    double energy_count;
    double instruction_count;
    
    /*Copy and scale the previous FP coefficients into the coeffs array*/
    /*Copy the u64 mostat array into the double mostat array*/
    for(moi = 0; moi < mocount; moi++)
    {
        perf_model_coeffs_double[moi] = ((double)perf_model_coeffs_u64[moi]) * 
                        (1.0 / (double)((uint64_t)1 << LAMbS_MODELS_FIXEDPOINT_SHIFT));

        /*Update the power model based on its previous value*/                                        
        /*power_model_coeffs_double[moi] = power_model_coeffs_double[moi]*/
                        
        mostat_double[moi] = (double)loaddata_list_header->mostat_last_sp[moi];
    }
    
    /*predict the instruction count*/
    *est_icount_p = LMSVS_predict(  perf_modeladapter_p, 
                                    mostat_double, 
                                    perf_model_coeffs_double);
    
    /*predict the energy consumption*/
    *est_energy_p = LMSVS_predict(  powr_modeladapter_p, 
                                    mostat_double, 
                                    power_model_coeffs_double);
    
    /*Copy the instruction count and energy consumed into local varriables*/
    instruction_count = (double)loaddata_list_header->icount_last_sp;
    energy_count = (double)loaddata_list_header->energy_last_sp;
    
    /*Adapt the performance model*/
    LMSVS_update(   perf_modeladapter_p,
                    mostat_double,
                    perf_model_coeffs_double,
                    instruction_count);
    
    /*Adapt the power model*/
    LMSVS_update(   powr_modeladapter_p,
                    mostat_double,
                    power_model_coeffs_double,
                    energy_count);


    /*Keep track of the MO with the highest performance coeff*/
    max_perf_coeff_moi = 0; 
    max_perf_coeff = perf_model_coeffs_double[0];
    
    /*Copy the perf coefficients and perf coefficient inverse back into the memory 
    region*/
    /*Copy the u64 mostat array into the double mostat array*/
    for(moi = 0; moi < mocount; moi++)
    {
        /*Since no MO can turn back time or produce energy, none of the model
        coefficients can be negative. Saturate below at zero*/
        perf_model_coeffs_double[moi]   = (perf_model_coeffs_double[moi] < 0)?
                                        0: perf_model_coeffs_double[moi];

        /*Keep track of the MO with the highest performance coeff*/
        if( max_perf_coeff < perf_model_coeffs_double[moi] )
        {
            max_perf_coeff_moi = moi;
            max_perf_coeff = perf_model_coeffs_double[moi];
        }

        perf_model_coeffs_u64[moi]
            = (uint64_t)(   perf_model_coeffs_double[moi] *
                            (double)((uint64_t)1 << LAMbS_MODELS_FIXEDPOINT_SHIFT) );
        
        perf_model_coeffs_inv_u64[moi]
            = (uint64_t)(   ((double)1.0 / perf_model_coeffs_double[moi]) *
                            (double)((uint64_t)1 << LAMbS_MODELS_FIXEDPOINT_SHIFT) );
    }
}

