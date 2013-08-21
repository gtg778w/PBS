#ifndef PBS_BUDGET_INCLUDE
#define PBS_BUDGET_INCLUDE

struct SRT_struct;

/*For each time-related variable, a VIC-based variable has been created*/
struct pba_struct
{
    //bandwidth enforcement timer
    struct hrtimer  budget_enforcement_timer;

    struct preempt_notifier pin_notifier;

    //keep track of the last task activation time
    u64 last_actv_time;
    u64 last_actv_VIC;

    //total runtime in the current scheduling period
    u64 total_sp_runtime;
    u64 total_sp_runVIC;
    
    //reset at sp beginning
    //update tsr on sched out
    //update last_actv_time on sched_in and sp beginning
    //tsr = tsr + (now - last_actv_time)

    //keep track of the last job activation time
    u64 jb_actv_time;
    u64 jb_actv_VIC;

    //total runtime of the current job
    u64 total_jb_runtime;
    u64 total_jb_runVIC;
    
    //reset at jb beginning
    //update tjr on sched_out
    //update jb_actv_time on sched in and jb beginning
    //tjr = tjr + (now - total_jb_runtime)

    //for the second definition of job, jobs begin just before the prediction operation
    //unlike the original definition of job, where jobs begin at task-period boundaries
    
    //keep track of the last job activation time for the second definition of job
    u64 jb_actv_time2;
    u64 jb_actv_VIC2;

    //total runtime of the current job for the second definition of job
    u64 total_jb_runtime2;
    u64 total_jb_runVIC2;
    
    //reset on read operation
    //update tjr2 on sched_out
    //update jb_actv_time2 on sched in and read operation
    //tjr2 = tjr2 + (now - total_jb_runtime2)
    
    //the budget allocated in a scheduling period
    u64 sp_budget;

    u16 flags;
};

//the following are bits in the flags field 
//of the above data structure
#define PBA_SLEEPING (0x1)
#define PBA_THROTTLED (0x2)

void pba_firstjob(struct SRT_struct *ss);
void pba_nextjob(struct SRT_struct *ss);
void pba_nextjob2(struct SRT_struct *ss);

void pba_refresh_budget(struct SRT_struct *SRT_struct_p);

#define pba_set_budget(SRT_struct_p, budget) \
(SRT_struct_p->pba_struct.sp_budget = budget)

void pba_init(struct SRT_struct *SRT_struct_p);
void pba_uninit(struct SRT_struct *SRT_struct_p);

#endif
