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

#include <linux/smp.h>
/*
    smp_processor_id
*/ 

#include <linux/slab.h>
/*
    kmem_cache_* objects and functions
*/

#include "LAMbS_mostat.h"

struct _mostat_s
{
    u64 last_transition_time;
    int last_moi;
    /*The following field is a variable length array*/
    u64 stat[LAMbS_mo_MAXCOUNT];
};

/*An internal mostat structure. 
For multicore, there should be a per-cpu variable*/
static struct _mostat_s _mostat;

void LAMbS_mostat_transition_dummy(int old_moi, int new_moi){}

void LAMbS_mostat_transition(int old_moi, int new_moi)
{
    unsigned long irq_flags;
    u64 now;
    u64 time_since_last_transition;
    
    /*Saving and disabling interrupts around critical section*/
    local_irq_save(irq_flags);    
    
        /*Compute time spent in the previous state*/
        now = sched_clock();
        time_since_last_transition = now - _mostat.last_transition_time;
        _mostat.last_transition_time= now;
        
        /*Accumulate the time spent in the previous state*/
        _mostat.stat[old_moi] += time_since_last_transition;
        
        /*Update the current state*/
        _mostat.last_moi = new_moi;
        
    /*Saving and disabling interrupts around critical section*/
    local_irq_save(irq_flags);
}

void (*LAMbS_mostat_transition_p)(int old_moi, int new_moi) = 
    LAMbS_mostat_transition_dummy;

/*This macro should be called with interrupts disabled to prevent the value of 
mostat.last_moi from chaning between the time it is read in the caller and the time
it is checked in LAMbS_mostat_transition */
#define _LAMbS_mostat_update() LAMbS_mostat_transition_p(   _mostat.last_moi, \
                                                            _mostat.last_moi)

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
    int cpu;
    int moi;
    int mo;
    u64 now;
    
    unsigned long irq_flags;
    
    /*zero out the time spent in each mo*/
    for(moi = 0; moi < LAMbS_mo_MAXCOUNT; moi++)
    {
        _mostat.stat[moi] = 0;
    }
    
    /*Saving and disabling interrupts around critical section*/
    local_irq_save(irq_flags);
    
        /*Get the index of the current mode of operation on this CPU*/
        cpu = smp_processor_id();
        mo = cpufreq_quick_get(cpu);
        moi= LAMbS_molookup(mo);
        /*Check that a valid moi was returned*/
        if(moi >= 0)
        {
            now = sched_clock();
            _mostat.last_transition_time = now;
            _mostat.last_moi = moi;
            
            /*Setup the propper mostat callback function for
            transition in modes of operation*/
            LAMbS_mostat_transition_p = LAMbS_mostat_transition;
        }else
        {
            ret = -1;
            goto error0;
        }
    
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
        goto error1;
    }

    return 0;

error1:
    LAMbS_mostat_transition_p = LAMbS_mostat_transition_dummy;
error0:
    return ret;
}

void LAMbS_mostat_uninit(void)
{
    /*Free the mostat slab cache*/
    kmem_cache_destroy(LAMbS_mostat_slab_cache);
    
    /*Turn-off the mostat transition callback*/
    LAMbS_mostat_transition_p = LAMbS_mostat_transition_dummy;
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

void LAMbS_mostat_free(LAMbS_mostat_t* mostat)
{
    kmem_cache_free(LAMbS_mostat_slab_cache, mostat);
}

void LAMbS_mostat_get(LAMbS_mostat_t* mostat)
{
    unsigned long irq_flags;
    int moi;
    
    /*disable interrupts for critical section*/
    local_irq_save(irq_flags);
    
        /*update _mostat*/
        _LAMbS_mostat_update();
     
        /*copy the state table*/   
        for(moi = 0; moi < LAMbS_mo_count; moi++)
        {
            mostat->stat[moi] = _mostat.stat[moi];
        }
    
    /*restore interrupt flags for the end of the critical section*/
    local_irq_restore(irq_flags);
}

void LAMbS_mostat_getElapsed(LAMbS_mostat_t* mostat, u64 *elapsed)
{
    unsigned long irq_flags;
    int moi;
    
    /*disable interrupts for critical section*/
    local_irq_save(irq_flags);
    
        /*update _mostat*/
        _LAMbS_mostat_update();
        
        /*compute the change in the stat table and update the stat table*/
        for(moi = 0; moi < LAMbS_mo_count; moi++)
        {
            elapsed[moi] = _mostat.stat[moi] - mostat->stat[moi];
            mostat->stat[moi] = _mostat.stat[moi];
        }
    
    /*restore interrupt flags for the end of the critical section*/
    local_irq_restore(irq_flags);
}

