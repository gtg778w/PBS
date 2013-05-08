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

#include "jb_mgt.h"
#include "bw_mgt.h"
#include "pba.h"
/*
(un)init_bw_mgt
(un)init_jb_mgt
*/

MODULE_AUTHOR("Safayet N Ahmed");
MODULE_LICENSE("Dual BSD/GPL");

void cleanup_pbs_module(void)
{
	int returnable = 0;

	returnable = uninit_bw_mgt();
	if(returnable < 0)
		goto error;

	returnable = uninit_jb_mgt();

error:
	0;
	/*FIX ME*/
	/*Report the errors!*/
}

int __init init_pbs_module(void)
{
	int returnable = 0;

#ifdef CONFIG_SMP
	printk(KERN_INFO "This is an SMP kernel!\n");
#endif
#ifndef CONFIG_SCHED_HRTICK
	printk(KERN_INFO "HRTICK is disabled!\n");
#endif
#ifndef CONFIG_PREEMPT_NOTIFIERS
	printk(KERN_INFO "PREEMPT_NOTIFIERS is disabled!\n");
#endif

    returnable = setup_sched_clock();
    if(returnable < 0)
        goto error;

	returnable = init_bw_mgt();
	if(returnable < 0)
		goto error;

	returnable = init_jb_mgt();
	if(returnable < 0)
	{
		uninit_bw_mgt();
	}

error:
	return returnable;
}

module_init(init_pbs_module);
module_exit(cleanup_pbs_module);

