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

MODULE_AUTHOR("Safayet N Ahmed");
MODULE_LICENSE("Dual BSD/GPL");

void cleanup_cpufreq_test(void)
{
}

int __init init_cpufreq_test(void)
{
    int returnable = 0;
    int cpu;
    int f;

    unsigned int cur_freq;

    struct cpufreq_frequency_table *freq_table;

#ifdef CONFIG_SMP
    printk(KERN_INFO "This is an SMP kernel!\n");
#endif
#ifndef CONFIG_PREEMPT_NOTIFIERS
    printk(KERN_INFO "PREEMPT_NOTIFIERS is disabled!\n");
#endif

    
    for_each_online_cpu(cpu) 
    {
        printk(KERN_INFO "cpufreq_test: cpu %i", cpu);
        printk(KERN_INFO "\tAvailable frequencies:");
        freq_table = cpufreq_frequency_get_table(cpu);
        if(NULL == freq_table)
        {
            printk(KERN_INFO "\t NULL");
        }
        else
        {
            for (f = 0; freq_table[f].frequency != CPUFREQ_TABLE_END; f++) 
            {
                printk(KERN_INFO "\t\t%i", freq_table[f].frequency);
            }
        }
        
        cur_freq = cpufreq_quick_get(cpu);
        printk(KERN_INFO "\tcurrent CPU frequency:");
        printk(KERN_INFO "\t\t%u", cur_freq);
    }
    
    return returnable;
}

module_init(init_cpufreq_test);
module_exit(cleanup_cpufreq_test);

