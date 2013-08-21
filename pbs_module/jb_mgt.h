#ifndef JB_MGT_INCLUDE
#define JB_MGT_INCLUDE

#include <linux/module.h>
/*
THIS_MODULE
*/

#include <linux/proc_fs.h>
/*
struct proc_dir_entry
create_proc_entry
remove_proc_entry
*/

#include <linux/kernel.h>
/* 
printk() 
*/

#include <linux/errno.h>
/* 
error codes 
*/

#include <linux/preempt.h>
/*
preempt_notifier
*/

#include <linux/slab.h>
/*
slab cache related stuff
*/

#include <linux/list.h>
/*
all linked list related stuff
*/

#include <linux/rculist.h>
/*
rcu list related stuff
*/
#include <linux/semaphore.h>
/*
semaphore related stuff
*/

#include <linux/ktime.h>
/*
ktime_t
*/

#include <asm/current.h>
/*
current
*/

#include "pbs_timing.h"
/*
SRT_timing_struct
*/

#include "pbs_mmap.h"
/*
Everything related to the loaddata structures
*/

#include "pbsSRT_cmd.h"
/*
Definitions that have to be shared with SRT tasks
e.g. job_mgt_cmd_t
*/

#include "pbs_budget.h"
/*
Code implementing budget accounting and enforcement
*/

struct SRT_struct
{
    struct SRT_timing_struct    timing_struct;
    struct pba_struct           pba_struct;
    struct SRT_job_log          log;
    struct SRT_summary_s        summary;

    SRT_loaddata_t  *loaddata;
    u64             maximum_overuse;
    u32             overuse_count;
    short           queue_length;
    unsigned short  allocation_index;

    struct task_struct  *task;

    char            state;
};

enum {SRT_OPEN, SRT_CONFIGURED, SRT_STARTED, SRT_LOOP, SRT_CLOSED};

int init_jb_mgt(void);
void uninit_jb_mgt(void);

#endif

