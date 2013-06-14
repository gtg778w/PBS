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

#include "LAMbS_mo.h"

MODULE_AUTHOR("Safayet N Ahmed");
MODULE_LICENSE("Dual BSD/GPL");

void cleanup_cpufreq_test(void)
{
    LAMbS_mo_free();
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

    ret = LAMbS_mo_init(1);
    if(0 != ret)
    {
        printk(KERN_INFO "init_cpufreq_test: LAMbS_mo_init failed!");
        goto error0;
    }
    
    return 0;

error0:
    return -1;
}

module_init(init_cpufreq_test);
module_exit(cleanup_cpufreq_test);

