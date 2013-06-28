
#include <linux/kernel.h>
/*
    printk
*/

#include <asm-generic/errno.h>
/*
    ENOMEM
*/

#include "LAMbS_icount.h"
#include "LAMbS_energy.h"
#include "LAMbS_mo.h"
#include "LAMbS_VIC.h"

/**********************************************************************

Measurement-related variables

***********************************************************************/

LAMbS_mostat_t *global_mostat_p;
LAMbS_icount_t global_icount;
LAMbS_energy_t global_energy;

/*********************************************************************
 * debug? Count kernel instructions and maybe other things later 
 *********************************************************************/
bool debug;


/**********************************************************************

Measurement-related functions

***********************************************************************/

int LAMbS_measurements_alloc(void)
{
    int ret;

    /*Allocate space for the mostat structure*/    
    global_mostat_p = LAMbS_mostat_alloc();
    if(NULL == global_mostat_p)
    {
        printk(KERN_INFO "LAMbS_measurements_alloc: LAMbS_mostat_alloc failed");
        ret = -ENOMEM;
        goto error0;
    }
    
    return 0;
    
error0:
    return ret;
}

void LAMbS_measurements_init(void)
{
    /*Init the instruction count*/
    LAMbS_icount_get(&global_icount);
    
    /*Init the energy measurement*/
    LAMbS_energy_get(&global_energy);
    
    /*Init the time spent in each mode of operation*/
    LAMbS_mostat_get(global_mostat_p);
}

void LAMbS_measure_delta(   u64* delta_icount_p,
                            u64* delta_energy_p,
                            u64* delta_mostat)
{
    /*Get the number of instructions executed since the last call*/
    LAMbS_icount_getDelta(&global_icount, delta_icount_p);
    
    /*Get the amount of energy consumed since the last call*/
    LAMbS_energy_getDelta(&global_energy, delta_energy_p);

    /*Get the amount of energy consumed since the last call*/
    LAMbS_mostat_getDelta(global_mostat_p, delta_mostat);
}

void LAMbS_measurements_free(void)
{
    /*Free the mostat structure*/
    LAMbS_mostat_free(global_mostat_p);
}

/**********************************************************************

(un)initialization-related functions for LAMbS

***********************************************************************/

int LAMbS_init(void)
{
    int ret;
    
    ret = LAMbS_icount_init(debug);
    if(0 != ret)
    {
        printk(KERN_INFO "LAMbS_init: LAMbS_icount_init failed");
        goto error0;
    }
    
    ret = LAMbS_energy_init();
    if(0 != ret)
    {
        printk(KERN_INFO "LAMbS_init: LAMbS_energy_init failed");
        goto error1;
    }
    
    ret = LAMbS_mo_init(0);
    if(0 != ret)
    {
        printk(KERN_INFO "LAMbS_init: LAMbS_mo_init failed");
        goto error2;
    }

    ret = LAMbS_VIC_init();
    if(0 != ret)
    {
        printk(KERN_INFO "LAMbS_init: LAMbS_VIC_init failed");
        goto error3;        
    }

    ret = LAMbS_measurements_alloc();
    if(0 != ret)
    {
        printk(KERN_INFO "LAMbS_init: LAMbS_measurements_alloc failed");
        goto error4;
    }
    
    return 0;
    
error4:
    LAMbS_VIC_uninit();
error3:
    LAMbS_mo_uninit();        
error2:
    LAMbS_energy_uninit();
error1:
    LAMbS_icount_uninit();
error0:
    return ret;
}

void LAMbS_uninit(void)
{
    LAMbS_measurements_free();

    LAMbS_VIC_uninit();

    LAMbS_mo_uninit();
    
    LAMbS_energy_uninit();
    
    LAMbS_icount_uninit();
}

