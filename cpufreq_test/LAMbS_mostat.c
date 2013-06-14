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

#include "LAMbS_molookup.h"

#include "LAMbS_mostat.h"

/*A separate mostat_s structure is defined per cpu*/
struct mostat_s mostat;

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
        time_since_last_transition = now - mostat.last_transition_time;
        mostat.last_transition_time= now;
        
        /*Accumulate the time spent in the previous state*/
        mostat.stat[old_moi] += time_since_last_transition;
        
        /*Update the current state*/
        mostat.last_moi = new_moi;
        
    /*Saving and disabling interrupts around critical section*/
    local_irq_save(irq_flags);
}

void (*LAMbS_mostat_transition_p)(int old_moi, int new_moi) = 
    LAMbS_mostat_transition_dummy;

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
        mostat.stat[moi] = 0;
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
            mostat.last_transition_time = now;
            mostat.last_moi = moi;
            
            /*Setup the propper mostat callback function for
            transition in modes of operation*/
            LAMbS_mostat_transition_p = LAMbS_mostat_transition;
        }else
        {
            ret = -1;
        }
    
    /*Restoring interrupts after critical section*/
    local_irq_restore(irq_flags);
    
    return ret;
}

void LAMbS_mostat_free(void)
{
    LAMbS_mostat_transition_p = LAMbS_mostat_transition_dummy;
}

