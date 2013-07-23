
#include <linux/kernel.h>

#include <linux/module.h>
/*
EXPORT_SYMBOL
*/

#include <linux/cpufreq.h>
/*
    cpufreq_register_notifier
    cpufreq_unregister_notifier    
*/

#include <linux/notifier.h>
/*
    struct notifier_block
*/

#include <linux/smp.h>
/*
    smp_processor_id
*/ 

#include "LAMbS_mo.h"

s32 LAMbS_current_moi;
EXPORT_SYMBOL(LAMbS_current_moi);

static struct list_head motrans_notifier_chain;

/*Insert a LAMbS_motrans_notifier_s object into the notifier chain*/
void LAMbS_motrans_register_notifier(struct LAMbS_motrans_notifier_s *notifier)
{
    unsigned long irq_flags;
    
    local_irq_save(irq_flags);

    list_add(&(notifier->chain_node), &(motrans_notifier_chain));

    local_irq_restore(irq_flags);
}

/*Remove a LAMbS_motrans_notifier_s object from the notifier chain*/
void LAMbS_motrans_unregister_notifier(struct LAMbS_motrans_notifier_s *notifier)
{
    unsigned long irq_flags;
    
    local_irq_save(irq_flags);

    list_del(&(notifier->chain_node));

    local_irq_restore(irq_flags);
}

static int _freqchange_notifier(  struct notifier_block *nb,
                                  unsigned long val, void *data)
{
    int ret = 0;
    
    /*Data contains a pointer to a cpufreq_freqs structure.
    The cpufreqs structure describes the previous frequency,
    next frequency, the cpu where the transition takes place,
    and flags related to the driver*/
    struct cpufreq_freqs *freq = data;
    u32 old_mo, new_mo;
    s32 old_moi, new_moi;    

    struct list_head *list_node_next;
    struct list_head *temp_node;

    struct LAMbS_motrans_notifier_s *motrans_notifier_next;

    unsigned long irq_flags;
        
    /*Check what type of event this notification is for*/
    switch(val)
    {
        case CPUFREQ_PRECHANGE:
            /*Ignored*/
            break;
            
        case CPUFREQ_POSTCHANGE:
            /*Pick the POSTCHANGE event since it is closer to the actual change in MO*/
            
            /*Determine the index of the previous and current modes of operation*/
            old_mo  = freq->old;
            new_mo  = freq->new;
            old_moi = LAMbS_molookup(old_mo);
            new_moi = LAMbS_molookup(new_mo);

            /*Enter critical section*/
            local_irq_save(irq_flags);
            
                /*Update the current mo and instruction retirement rate*/
                LAMbS_current_moi = new_moi;
                
                /*Call motrans_notifiers registered in the chain and call the 
                corresponding callbacks*/
                list_for_each_safe( list_node_next, 
                                    temp_node, 
                                    &(motrans_notifier_chain))
                {
                    /*Compute the pointer to the motrans notifier using the pointer to the
                    list head*/
                    motrans_notifier_next = container_of(   list_node_next, 
                                                            struct LAMbS_motrans_notifier_s, 
                                                            chain_node);
                                                        
                    /*Call the callback function*/
                    motrans_notifier_next->callback(    motrans_notifier_next,
                                                        old_moi,
                                                        new_moi);
                }

            /*Exit critical section*/
            local_irq_restore(irq_flags);

            break;
        
        case CPUFREQ_RESUMECHANGE:
            /*Ignored*/
            break;
            
        case CPUFREQ_SUSPENDCHANGE:
            /*Ignored*/
            break;
            
        default:
            printk(KERN_INFO "LAMbS_motrans_notifier: unrecognized CPUFREQ_CHANGE event");
            ret = -1;
            break;
    }
    
    return ret;
}

static struct notifier_block _freqchange_notifier_block = 
{
    .notifier_call = _freqchange_notifier
};

static int LAMbS_mo_setup = 0;

int LAMbS_mo_init(int verbose)
{
    int ret;
    
    unsigned long irq_flags;
    int cpu, mo;
    
    /*Check that LAMbS_mo has not already been setup*/
    if(0 != LAMbS_mo_setup)
    {
        printk(KERN_INFO "LAMbS_mo_init: has been called previously");
        ret = -1;
        goto error0;
    }
    
    /*Setup the mo lookup table*/
    ret = LAMbS_molookup_init();
    if(0 != ret)
    {
        printk(KERN_INFO "LAMbS_mo_init: LAMbS_molookup_init failed!");
        goto error0;
    }

    /*Test the mo lookup table*/
    ret = LAMbS_molookup_test(verbose);
    if(ret == -1)
    {
        printk(KERN_INFO "LAMbS_mo_init: LAMbS_molookup_test failed!");
        goto error1;
    }

    /*Initialize the linked list head, motransition_notifier_chain*/
    INIT_LIST_HEAD(&motrans_notifier_chain);

    /*Setup the frequency change notifier*/
    ret = cpufreq_register_notifier(    &_freqchange_notifier_block,
                                        CPUFREQ_TRANSITION_NOTIFIER);
    if(0 != ret)
    {
        printk(KERN_INFO "LAMbS_mo_init: cpufreq_register_notifier failed!\n");
        goto error1;
    }

    /*Get the current MOI and set the last_moi global varriable*/
    /*Saving and disabling interrupts around critical section*/
    local_irq_save(irq_flags);
    
        /*Get the index of the current mode of operation on this CPU*/
        cpu = smp_processor_id();
        mo = cpufreq_quick_get(cpu);
        LAMbS_current_moi = LAMbS_molookup(mo);

    /*Restoring interrupts after critical section*/
    local_irq_restore(irq_flags);    

    /*Check that a valid moi was returned by the molookup*/
    if(LAMbS_current_moi < 0)
    {
        printk(KERN_INFO "LAMbS_mo_init: starting moi: %i", LAMbS_current_moi);
        ret = -1;
        goto error2;
    }
    
    /*Setup the mostat mechanism*/
    ret = LAMbS_mostat_init();
    if(ret == -1)
    {
        printk(KERN_INFO "LAMbS_mo_init: LAMbS_mostat_init failed!");
        goto error2;
    }
        
    LAMbS_mo_setup = 1;
    
    return 0;
    
error2:
    LAMbS_current_moi = 0;
    /*Remove the MO(frequency) change notifier*/
    cpufreq_unregister_notifier(    & _freqchange_notifier_block,
                                    CPUFREQ_TRANSITION_NOTIFIER);
error1:
    /*cleanup the MO lookup table*/
    LAMbS_molookup_uninit();
error0:
    return ret;
}

void LAMbS_mo_uninit(void)
{
    if(0 != LAMbS_mo_setup)
    {
        LAMbS_mo_setup = 0;
        
        /*Cleanup the mostat mechanism*/
        LAMbS_mostat_uninit();

            /*Remove the MO(frequency) change notifier*/
            cpufreq_unregister_notifier(    & _freqchange_notifier_block,
                                            CPUFREQ_TRANSITION_NOTIFIER);
        
        /*Clean up the MO lookup table*/
        LAMbS_molookup_uninit();
    }
}

