/*
 *
 * Copyright (C) 2013 Safayet Ahmed
 * Just test access to the cpufreq subsystem
 *
 */

#include <linux/module.h>
/*
MODULE_AUTHOR
MODULE_LICENSE
THIS_MODULE

module_init
module_exit
*/

#include <linux/moduleparam.h>
/*
module_param
*/

#include <linux/cpufreq.h>
/*
    struct cpufreq_frequency_table
    cpufreq_frequency_get_table
*/

#include <linux/cpumask.h>
/*
    for_each_online_cpu
*/

#include <linux/notifier.h>
/*
    struct notifier_block
*/

#include "LAMbS_molookup.h"

MODULE_AUTHOR("Safayet N Ahmed");
MODULE_LICENSE("Dual BSD/GPL");

static int cpufreq_test_trans_notifier( struct notifier_block *nb,
                                        unsigned long val, void *data)
{
    /*Data contains a pointer to a cpufreq_freqs structure.
    The cpufreqs structure describes the previous frequency,
    next frequency, the cpu where the transition takes place,
    and flags related to the driver*/
    struct cpufreq_freqs *freq = data;

    unsigned int cur_freq = cpufreq_quick_get(freq->cpu);
    
    printk(KERN_INFO "cpufre_test: trans notifier called for cpu %i", 
                     freq->cpu);
    
    /*val contains one of a number of values:
        CPUFREQ_PRECHANGE
        CPUFREQ_POSTCHANGE
        CPUFREQ_RESUMECHANGE
        CPUFREQ_SUSPENDCHANGE
    */
    switch(val)
    {
        case CPUFREQ_PRECHANGE:
            printk(KERN_INFO "transition event: CPUFREQ_PRECHANGE");
            break;
            
        case CPUFREQ_POSTCHANGE:
            printk(KERN_INFO "transition event: CPUFREQ_POSTCHANGE");
            break;
        
        case CPUFREQ_RESUMECHANGE:
            printk(KERN_INFO "transition event: CPUFREQ_RESUMECHANGE");
            break;
            
        case CPUFREQ_SUSPENDCHANGE:
            printk(KERN_INFO "transition event: CPUFREQ_SUSPENDCHANGE");
            break;
            
        default:
            break;
    }
    
    printk(KERN_INFO "\t%ukHz -> %ukHz",
                     freq->old,
                     freq->new);
    printk(KERN_INFO "\tcurrent: %ukHz", cur_freq);
    
    return 0;
}

static struct notifier_block cpufreq_test_trans_notifier_block = 
{
    .notifier_call = cpufreq_test_trans_notifier
};
int trans_notifier_registered = 0;


void cleanup_cpufreq_test(void)
{
    if(1 == trans_notifier_registered)
    {
        cpufreq_unregister_notifier(&cpufreq_test_trans_notifier_block,
                                    CPUFREQ_TRANSITION_NOTIFIER);   
    }
}

int __init init_cpufreq_test(void)
{
    int ret = 0;

#ifdef CONFIG_SMP
    printk(KERN_INFO "This is an SMP kernel!\n");
#endif
#ifndef CONFIG_PREEMPT_NOTIFIERS
    printk(KERN_INFO "PREEMPT_NOTIFIERS is disabled!\n");
#endif

    ret = LAMbS_molookup_init();
    if(0 != ret)
    {
        printk(KERN_INFO "init_cpufreq_test: LAMbS_molookup_init failed!");
    }
    else
    {
        ret = LAMbS_molookup_test();
        if(ret == -1)
        {
            printk(KERN_INFO "init_cpufreq_test: LAMbS_molookup_test failed!");
        }
        LAMbS_molookup_free();
    }

    ret = cpufreq_register_notifier(&cpufreq_test_trans_notifier_block,
                                    CPUFREQ_TRANSITION_NOTIFIER);
    if(0 != ret)
    {
        trans_notifier_registered = 0;
        printk(KERN_INFO "\tcpufreq_register_notifier failed!\n");
    }
    else
    {
        trans_notifier_registered = 1;
    }
    
    return ret;
}

module_init(init_cpufreq_test);
module_exit(cleanup_cpufreq_test);

