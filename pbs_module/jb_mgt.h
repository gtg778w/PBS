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
Everything related to the history structures
*/

struct pba_struct
{
	//bandwidth enforcement timer
	struct hrtimer	hrtimer;

    struct preempt_notifier pin_notifier;

	//keep track of the last task activation time
	u64	last_actv_time;

	//total runtime in the current scheduling period
	u64	total_sp_runtime;
	//reset at sp beginning
	//update tsr on sched out
	//update last_actv_time on sched_in and sp beginning
	//tsr = tsr + (now - last_actv_time)

	//keep track of the last job activation time
	u64	jb_actv_time;

	//total runtime of the current job
	u64 total_jb_runtime; 
	//reset at jb beginning
	//update tjr on sched_out
	//update jb_actv_time on sched in and jb beginning
	//tjr = tjr + (now - total_jb_runtime)

    //for the second definition of job, jobs begin just before the prediction operation
    //unlike the original definition of job, where jobs begin at task-period boundaries
    
    //keep track of the last job activation time for the second definition of job
	u64	jb_actv_time2;

	//total runtime of the current job for the second definition of job
	u64	total_jb_runtime2;
	//reset on read operation
	//update tjr2 on sched_out
	//update jb_actv_time2 on sched in and read operation
	//tjr2 = tjr2 + (now - total_jb_runtime2)
	
    //the budget allocated in a scheduling period
    u64 sp_budget;

    u16 flags;

    //FIXME
    u16 switch_count;
    u16 throttle_count;
};

//the following are bits in the flags field 
//of the above data structure
#define PBA_SLEEPING (0x1)
#define PBA_THROTTLED (0x2)

struct SRT_job_log
{
	u64	abs_releaseTime;
	u64	abs_LFT;
	u32	runtime;
	u32 runtime2;

    u32	last_sp_compt_allocated;
	u32	last_sp_compt_used_sofar;

    //FIXME
    u32 throttle_count;
    u32 switch_count;
    unsigned char miss;
};

struct SRT_struct
{
	struct SRT_timing_struct	timing_struct;
	struct pba_struct           pba_struct;
	struct SRT_job_log 			log;

	SRT_history_t	*history;
	u64				maximum_overuse;
	u32				overuse_count;
	short	        queue_length;
	unsigned short	allocation_index;

	struct task_struct	*task;

	char			state;
	char			init_mask;
};

enum {SRT_OPEN, SRT_CONFIGURED, SRT_STARTED, SRT_LOOP, SRT_CLOSED};

void init2_jb_mgt(void);
int init_jb_mgt(void);
int uninit_jb_mgt(void);

#endif

