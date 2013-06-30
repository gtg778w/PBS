#include <linux/gfp.h>
/*
    GFP_KERNEL
*/

#include <linux/slab.h>
/*
    kmalloc
    kfree
*/

#include "LAMbS_VIC.h"

#include "LAMbS_molookup.h"
/*
    LAMbS_mo_count;
*/
#include "LAMbS_mostat.h"
#include "LAMbS_models.h"

/*********************************************************
*Code and varriables related to the monotonic clock used 
by everything
*********************************************************/

extern unsigned long long sched_clock(void) __attribute__ ((weak));

int setup_sched_clock(void)
{
    if(NULL == sched_clock)
    {
        printk(KERN_INFO "setup_sched_clock: The sched_clock symbol needed by the "
                        "pbs_module is not expotred by the kernel.");
        goto error0;
    }

    return 0;
error0:
    return -1;
}


/*********************************************************

**********************************************************/

static u64              last_global_VIC;
static LAMbS_mostat_t   *VIC_mostat_p;
static u64              *_VIC_mostat_delta;

int LAMbS_VIC_init(void)
{
    int ret;

    VIC_mostat_p = LAMbS_mostat_alloc();
    if(NULL == VIC_mostat_p)
    {
        printk(KERN_INFO "LAMbS_VIC_init: LAMbS_mostat_alloc failed for VIC_mostat_p");
        ret = -ENOMEM;
        goto error0;
    }
    
    _VIC_mostat_delta = kmalloc((sizeof(u64) * LAMbS_mo_count), GFP_KERNEL);
    if(NULL == _VIC_mostat_delta)
    {
        printk(KERN_INFO "LAMbS_VIC_init: kmalloc failed for _VIC_mostat_delta");
        ret = -ENOMEM;
        goto error1;        
    }
    
    /*It is assumed that during initialization, no synchronization is necessary*/
    
    /*Init the time spent in each mode of operation*/
    LAMbS_mostat_get(VIC_mostat_p);
        
    last_global_VIC = 0;
        
    return 0;
error1:
    LAMbS_mostat_free(VIC_mostat_p);
    VIC_mostat_p = NULL;    
error0:
    return ret;
}

u64 LAMbS_VIC_get(void)
{
    unsigned long irq_flags;
    u64 VIC_delta;
    int moi;
    
    VIC_delta = 0;
    
    /*Saving and disabling interrupts around critical section*/
    local_irq_save(irq_flags);
    
        /*Get the time spent in each mode of operation since the last call*/
        LAMbS_mostat_getDelta(VIC_mostat_p, _VIC_mostat_delta);
        
        /*Do a dot product between _VIC_mostat_delta and instruction_retirement_rate*/
        for(moi = 0; moi < LAMbS_mo_count; moi++)
        {
            /*This multiplication operation is expensive, so only do it if necessary*/
            /*In most cases, there should only be non-zero delta_mostat values in one or 
            two modes of operation*/
            if(0 != _VIC_mostat_delta[moi])
            {
                /*The instruction retirement rate is stored as a fixed point value with
                LAMbS_MODELS_FIXEDPOINT_SHIFT fractional binary digits. A careful
                multiplication operation is nedded to prevent overflow*/
                VIC_delta = VIC_delta + 
                            LAMBS_models_multiply_shift(_VIC_mostat_delta[moi], 
                                                        instruction_retirement_rate[moi], 
                                                        LAMbS_MODELS_FIXEDPOINT_SHIFT);
            }
        }

        /*Accumulate the change in VIC*/
        last_global_VIC = last_global_VIC + VIC_delta;
    
    /*Restore previous interrupt state after critical section*/
    local_irq_restore(irq_flags);
    
    return last_global_VIC;
}

void LAMbS_VIC_uninit(void)
{
    kfree(_VIC_mostat_delta);
    _VIC_mostat_delta = NULL;
    
    LAMbS_mostat_free(VIC_mostat_p);
    VIC_mostat_p = NULL;
}

