#include <linux/irqflags.h>
/*
local_irq_save
local_irq_restore
*/

#include <linux/sched.h>
/*
sched_clock
*/

#include <linux/cpufreq.h>
/*
    cpufreq_quick_get
*/

#include <linux/slab.h>
/*
    kmem_cache_* objects and functions
*/

#include <linux/module.h>
/*
EXPORT_SYMBOL
*/

#include "LAMbS_mostat.h"

#include "LAMbS_mo.h"

/*An internal mostat structure with a statically defined number of MOs. 
For multicore, there should be a per-cpu variable*/
struct _mostat_s
{
    u64 time_stamp;
    /*The following field is a variable length array*/
    u64 stat[LAMbS_mo_MAXCOUNT];
};

struct _mostat_s _mostat;

/*It is assumed that the following function is called with interrupts disabled.*/
void LAMbS_mostat_motrans_callback( s32 old_moi)
{
    u64 now;
    u64 time_since_last_transition;
        
    /*Compute time spent in the previous state*/
    now = sched_clock();
    time_since_last_transition = now - _mostat.time_stamp;
    _mostat.time_stamp = now;
    
    /*Accumulate the time spent in the previous state*/
    _mostat.stat[old_moi] += time_since_last_transition;
}

/*This macro should be called with interrupts disabled to prevent the value of 
LAMbS_current_moi from chaning between the time it is read in the caller and the time
it is checked in LAMbS_mostat_transition */
#define _LAMbS_mostat_update() LAMbS_mostat_motrans_callback(LAMbS_current_moi)

/*Look-aside cache of mostat variables. 
If the number of MO is different per cpu, this should 
definitely be a per-cpu variable. Otherwise, use of the SLAB_HWCACHE_ALIGN option during
the allocation of the kmem_cache object should ensure good performance, even under SMP 
configuration*/
struct  kmem_cache *LAMbS_mostat_slab_cache = NULL;

/*It is assumed that the molookup mechanism and the frequency transition notifiers are 
setup before this function is called*/
int LAMbS_mostat_init(void)
{
    int ret = 0;
    s32 moi;
    
    unsigned long irq_flags;
    
    /*Saving and disabling interrupts around critical section*/
    local_irq_save(irq_flags);

        /*zero out the time spent in each mo*/
        for(moi = 0; moi < LAMbS_mo_MAXCOUNT; moi++)
        {
            _mostat.stat[moi] = 0;
        }

        _mostat.time_stamp = sched_clock();

    /*Restoring interrupts after critical section*/
    local_irq_restore(irq_flags);    
    
    /*Initialize the mostat slabcache*/
    LAMbS_mostat_slab_cache = kmem_cache_create(    "LAMbS_mostat_t", 
                                                    LAMbS_mostat_size(), 
                                                    0, SLAB_HWCACHE_ALIGN, 
                                                    NULL);
    if(NULL == LAMbS_mostat_slab_cache)
    {
        ret = -ENOMEM;
        goto error0;
    }

    return 0;

error0:
    return ret;
}

void LAMbS_mostat_uninit(void)
{
    /*Free the mostat slab cache*/
    kmem_cache_destroy(LAMbS_mostat_slab_cache);
}

LAMbS_mostat_t* LAMbS_mostat_alloc(void)
{
    LAMbS_mostat_t* ret = kmem_cache_alloc(LAMbS_mostat_slab_cache, GFP_KERNEL);
    if(NULL == ret)
    {
        printk(KERN_INFO "LAMbS_mostat_alloc: kmem_cache_alloc failed");
    }
    
    return ret;
}

EXPORT_SYMBOL(LAMbS_mostat_alloc);

void LAMbS_mostat_free(LAMbS_mostat_t* mostat)
{
    kmem_cache_free(LAMbS_mostat_slab_cache, mostat);
}

EXPORT_SYMBOL(LAMbS_mostat_free);

void LAMbS_mostat_get(LAMbS_mostat_t* mostat)
{
    unsigned long irq_flags;
    int moi;
    
    /*disable interrupts for critical section*/
    local_irq_save(irq_flags);
    
        /*update _mostat*/
        _LAMbS_mostat_update();
     
        /*Copy the time stamp*/
        mostat->time_stamp = _mostat.time_stamp;
        /*Copy the state table*/   
        for(moi = 0; moi < LAMbS_mo_struct.count; moi++)
        {
            mostat->stat[moi] = _mostat.stat[moi];
        }
    
    /*restore interrupt flags for the end of the critical section*/
    local_irq_restore(irq_flags);
}

EXPORT_SYMBOL(LAMbS_mostat_get);

void LAMbS_mostat_getDelta(LAMbS_mostat_t* mostat, u64 *delta_mostat)
{
    unsigned long irq_flags;
    s32 moi;
    
    /*disable interrupts for critical section*/
    local_irq_save(irq_flags);
    
        /*update _mostat*/
        _LAMbS_mostat_update();
        
        /*Copy the time stamp*/
        mostat->time_stamp = _mostat.time_stamp;
        /*Compute the change in the stat table and update the stat table*/
        for(moi = 0; moi < LAMbS_mo_struct.count; moi++)
        {
            delta_mostat[moi] = _mostat.stat[moi] - mostat->stat[moi];
            mostat->stat[moi] = _mostat.stat[moi];
        }
    
    /*restore interrupt flags for the end of the critical section*/
    local_irq_restore(irq_flags);
}

EXPORT_SYMBOL(LAMbS_mostat_getDelta);

