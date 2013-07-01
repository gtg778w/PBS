/*
 * procfs_timer_hack.c: create a procfs file that allows a program to access the high resolution timer for the init_task_group
 *
 * Copyright (C) 2010 Safayet Ahmed
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

#include "LAMbS.h"
#include "jb_mgt.h"
#include "bw_mgt.h"
#include "pba.h"
#include "LAMbS_VIC.h"
/*
(un)init_bw_mgt
(un)init_jb_mgt
setup_sched_clock
*/

MODULE_AUTHOR("Safayet N Ahmed");
MODULE_LICENSE("Dual BSD/GPL");

void cleanup_pbs_module(void)
{
    uninit_jb_mgt();

    uninit_bw_mgt();

    LAMbS_uninit();
}

int __init init_pbs_module(void)
{
    int ret;

#ifdef CONFIG_SMP
    printk(KERN_INFO "This is an SMP kernel!\n");
#endif
#ifndef CONFIG_SCHED_HRTICK
    printk(KERN_INFO "HRTICK is disabled!\n");
#endif
#ifndef CONFIG_PREEMPT_NOTIFIERS
    printk(KERN_INFO "PREEMPT_NOTIFIERS is disabled!\n");
#endif

    setup_sched_clock();

    ret = LAMbS_init();
    if(0 != ret)
    {
        printk(KERN_INFO "init_pbs_module: LAMbS_init failed");
        goto error0;
    }
    
    ret = init_bw_mgt();
    if(0 != ret)
    {
        printk(KERN_INFO "init_pbs_module: init_bw_mgt failed");
        goto error1;
    }

    ret = init_jb_mgt();
    if(0 != ret)
    {
        printk(KERN_INFO "init_pbs_module: init_jb_mgt failed");
        goto error2;
    }

    return 0;

error2:
    uninit_bw_mgt();
error1:
    LAMbS_uninit();
error0:
    return ret;
}

module_init(init_pbs_module);
module_exit(cleanup_pbs_module);

