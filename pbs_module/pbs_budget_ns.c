#include "bw_mgt.h"
#include "jb_mgt.h"

#include "pbs_budget.h"

#include "LAMbS_VIC.h"
/*
    LAMbS_clock
    LAMbS_VIC_get
*/

#define PBS_BUDGET_IS_THROTTLED(budget_struct_p)   \
        (budget_struct_p->flags & PBS_BUDGET_THROTTLED)
#define PBS_BUDGET_IS_SLEEPING(budget_struct_p)    \
        (budget_struct_p->flags & PBS_BUDGET_SLEEPING)

#define THROTTLE_THREASHOLD 5000

static u64 inline get_jbruntime(struct pbs_budget_struct *budget_struct_p)
{
    u64 now;

    u64 total_runtime, current_runtime = 0;

    //if the task is currently running, compute the runtime since
    //the last activation
    if(!PBS_BUDGET_IS_SLEEPING(budget_struct_p))
    {
        //obtain the current time
        now = LAMbS_clock();
        //compute the runtime in this activation
        current_runtime = now - budget_struct_p->jb_actv_time;
    }

    //set the total runtime to the sum of the curent runtime 
    //and accumulated previous runtime
    total_runtime = current_runtime + budget_struct_p->total_jb_runtime;

    return total_runtime;
}

void pbs_budget_firstjob(struct SRT_struct *ss)
{
    u64 now, now_VIC;

    struct pbs_budget_struct *budget_struct_p;    

    budget_struct_p = &(ss->budget_struct);

    //obtain the current time and VIC
    now_VIC = LAMbS_VIC_get(&now);
    
    (ss->loaddata)->current_runtime = 0;

    (ss->log).runtime   = 0;
    (ss->log).runtime2  = 0;
    (ss->log).last_sp_compt_allocated   = (budget_struct_p->sp_budget);
    (ss->log).last_sp_compt_used_sofar  = 0;

    (ss->log).runVIC    = 0;
    (ss->log).runVIC2   = 0;    

    //reset the total accumulated runtime to 0
    budget_struct_p->total_jb_runtime   = 0;
    budget_struct_p->total_jb_runtime2  = 0;
    
    budget_struct_p->total_jb_runVIC    = 0;
    budget_struct_p->total_jb_runVIC2   = 0;
    
    //set now as the beginning of the new job
    budget_struct_p->jb_actv_time   = now;
    budget_struct_p->jb_actv_VIC    = now_VIC;
    //also set now as the beginning of the new job based on the second 
    //definition of job, although this is slightly incorrect, it will only
    //be used for the first job
    budget_struct_p->jb_actv_time2  = now;
    budget_struct_p->jb_actv_VIC2   = now_VIC;
    
    //reset the budget used to 0
    budget_struct_p->total_sp_runtime   = 0;
    budget_struct_p->total_sp_runVIC    = 0;
    
    //set now as the beginning of current activation
    budget_struct_p->last_actv_time     = now;
    budget_struct_p->last_actv_VIC      = now_VIC;
    
}

/*Job boundary according to the traditional definition of job:
    sleep to sleep*/
void pbs_budget_jobboundary1(struct SRT_struct *ss)
{
    u64 now, now_VIC;
    u64 current_jb_runtime, current_sp_runtime;
    u64 current_jb_runVIC, current_sp_runVIC;
    
    u64 total_runtime;
    u64 total_runVIC;

    struct pbs_budget_struct *budget_struct_p;

    budget_struct_p = &(ss->budget_struct);

    //obtain the current time and VIC
    now_VIC = LAMbS_VIC_get(&now);
    
    //compute the runtime in this activation
    current_jb_runtime = now - (budget_struct_p->jb_actv_time);
    current_sp_runtime = now - (budget_struct_p->last_actv_time);

    //compute the runVIC in this activation
    current_jb_runVIC = now_VIC - (budget_struct_p->jb_actv_VIC);
    current_sp_runVIC = now_VIC - (budget_struct_p->last_actv_VIC);

    //set the total runtime to the sum of the curent runtime 
    //and accumulated previous runtime
    total_runtime = current_jb_runtime + (budget_struct_p->total_jb_runtime);

    //set the total runVIC to the sum of the curent runtime 
    //and accumulated previous runVIC
    total_runVIC  = current_jb_runVIC + (budget_struct_p->total_jb_runVIC);

    //update the loaddata structure for the next job
    (ss->loaddata)->current_runtime = 0;
    (ss->loaddata)->queue_length = (unsigned short)(ss->queue_length);

    //write information regarding the completed job into the log
    (ss->log).runtime = total_runtime;
    (ss->log).runVIC  = total_runVIC;
    (ss->log).last_sp_compt_allocated   = (budget_struct_p->sp_budget);
    (ss->log).last_sp_compt_used_sofar  = current_sp_runtime;

    //reset the total accumulated runtime and runVIC to 0
    budget_struct_p->total_jb_runtime   = 0;
    budget_struct_p->total_jb_runVIC    = 0;
    
    //set now and now_VIC as the beginning of the new job
    budget_struct_p->jb_actv_time   = now;
    budget_struct_p->jb_actv_VIC    = now_VIC;
}

/*Job boundary according to the second definition of job:
    Prediction to Prediction*/
void pbs_budget_jobboundary2(struct SRT_struct *ss)
{
    u64 now, current_jb_runtime2;

    u64 total_runtime2;

    u64 now_VIC, current_jb_runVIC2;

    u64 total_runVIC2;
    
    struct pbs_budget_struct *budget_struct_p;

    budget_struct_p = &(ss->budget_struct);

    //obtain the current time
    now_VIC = LAMbS_VIC_get(&now);
    
    //compute the runtime in this activation
    current_jb_runtime2 = now - (budget_struct_p->jb_actv_time2);
    current_jb_runVIC2  = now_VIC - (budget_struct_p->jb_actv_VIC2);
        
    //set the total runtime to the sum of the curent runtime 
    //and accumulated previous runtime
    total_runtime2  = current_jb_runtime2 + (budget_struct_p->total_jb_runtime2);
    total_runVIC2   = current_jb_runVIC2 + (budget_struct_p->total_jb_runVIC2);

    //write information regarding the completed job into the log
    (ss->log).runtime2  = total_runtime2;
    (ss->log).runVIC2   = total_runVIC2;
    
    //reset the total accumulated runtime to 0
    budget_struct_p->total_jb_runtime2  = 0;
    budget_struct_p->total_jb_runVIC2   = 0;
    
    //set now as the beginning of the new job
    budget_struct_p->jb_actv_time2  = now;
    budget_struct_p->jb_actv_VIC2   = now_VIC;
}

////this is called for the purpose of checking if budget has expired
//this is also called to determine how much budget remains after a 
//job completes
static inline u64 _get_spruntime(struct pbs_budget_struct *budget_struct_p)
{
    u64 now;

    u64 total_runtime, current_runtime = 0;

    //if the task is currently running, compute the runtime since
    //the last activation
    if(!PBS_BUDGET_IS_SLEEPING(budget_struct_p))
    {
        //obtain the current time
        now = LAMbS_clock();
        current_runtime = now - budget_struct_p->last_actv_time;
    }

    //set the total runtime to the sum of the curent accumulated 
    //runtime and previous runtime
    total_runtime = current_runtime + budget_struct_p->total_sp_runtime;

    return total_runtime;
}

//FIXME
static u64 check_budget_remaining(struct pbs_budget_struct *budget_struct_p)
{
    struct SRT_struct   *SRT_struct_p;
    s64 runtime;
    s64 remaining;
    s64 overuse;

    SRT_struct_p = container_of(budget_struct_p, struct SRT_struct, budget_struct);

    //determine the amount of time remaining until the bandwidth expires
    runtime = (s64)_get_spruntime(budget_struct_p);
    remaining = (s64)budget_struct_p->sp_budget - runtime;

    if(remaining < THROTTLE_THREASHOLD)  
    {
        /*Compute overuse statistics*/
        if(remaining < 0)
        {
            overuse = -remaining;

            if(overuse > THROTTLE_THREASHOLD)
            {
                SRT_struct_p->overuse_count++;
            }

            if(overuse > SRT_struct_p->maximum_overuse)
            {
                SRT_struct_p->maximum_overuse = overuse;
            }
            
        }
        
        /*  If the task is not already throttled, throttle it:
                - set the flag
                - put it to sleep
        */
        if((budget_struct_p->flags & PBS_BUDGET_THROTTLED) == 0)
        {
            if( SRT_struct_p->task != current)
            {
                static unsigned char flag = 1;

                if(flag)
                {
                    flag = 0;
                    printk( KERN_INFO 
                            "check_budget_remaining: WARNING accounting error\n");
                }
            }

            budget_struct_p->flags |= PBS_BUDGET_THROTTLED;

            //throttle the task (by putting it to sleep)
            set_current_state(TASK_UNINTERRUPTIBLE);
            set_tsk_need_resched(SRT_struct_p->task);
        }
        
        /*Set remaining to zero for the return value*/
        remaining = 0;
    }

    return remaining;
}

void pbs_budget_refresh(struct SRT_struct *SRT_struct_p)
{
    struct pbs_budget_struct *budget_struct_p;

    budget_struct_p = &(SRT_struct_p->budget_struct);

    (SRT_struct_p->loaddata)->current_runtime = get_jbruntime(budget_struct_p);

    //accumulate the budget used
    SRT_struct_p->summary.consumed_budget = 
        SRT_struct_p->summary.consumed_budget + budget_struct_p->total_sp_runtime;
    budget_struct_p->total_sp_runtime = 0;
    
    SRT_struct_p->summary.consumed_VIC = 
        SRT_struct_p->summary.consumed_VIC + budget_struct_p->total_sp_runVIC;
    budget_struct_p->total_sp_runVIC = 0;

    if(budget_struct_p->flags & PBS_BUDGET_THROTTLED)
    {
        budget_struct_p->flags &= (~PBS_BUDGET_THROTTLED);

        //wakeup the throttled task, if it has work left to do
        if((SRT_struct_p->queue_length) > 0)
        {
            wake_up_process(SRT_struct_p->task);
        }
    }

}

static enum hrtimer_restart _budget_enforcement_timer_callback(struct hrtimer *timer)
{
    struct pbs_budget_struct    *budget_struct_p;
    s64 remaining;
    ktime_t remaining_k;

    enum hrtimer_restart ret = HRTIMER_NORESTART;

    if(allocator_state != ALLOCATOR_LOOP)
    {
        return ret;
    }

    budget_struct_p = container_of( timer, 
                                    struct pbs_budget_struct, 
                                    budget_enforcement_timer);

    remaining = check_budget_remaining(budget_struct_p);

    if(remaining > 0)
    {
        //forward the timer by the necessary amount
        remaining = remaining - (THROTTLE_THREASHOLD/2);
        remaining_k.tv64 = remaining;
        hrtimer_forward_now(timer, remaining_k);
        ret = HRTIMER_RESTART;
    }

    return ret;
}

static void inline _budget_enforcement_timer_start(
                                            struct pbs_budget_struct *budget_struct_p)
{
    s64 remaining;
    ktime_t remaining_k;

    //make sure the scheduler is still in the right mode
    if(allocator_state != ALLOCATOR_LOOP)
    {
        return;
    }

    //check the time that this should be setup for
    remaining = check_budget_remaining(budget_struct_p);

    if(remaining > 0)
    {
        //setup the timer to go off after the necessary time
        remaining = remaining - (THROTTLE_THREASHOLD/2);
        remaining_k.tv64 = remaining;
        hrtimer_start(  &(budget_struct_p->budget_enforcement_timer), 
                        remaining_k, 
                        HRTIMER_MODE_REL);
    }
}

static void pbs_budget_schedin( struct preempt_notifier *notifier, 
                                int cpu);

static void pbs_budget_schedout(struct preempt_notifier *notifier, 
                                struct task_struct *next);

static struct preempt_ops pbs_budget_pops = {
                            .sched_in = pbs_budget_schedin,
                            .sched_out= pbs_budget_schedout,
                         };

static void pbs_budget_schedin( struct preempt_notifier *notifier, 
                                int cpu)
{
    u64 now;
    u64 now_VIC;

    struct SRT_struct           *SRT_struct_p;
    struct pbs_budget_struct    *budget_struct_p;

    now_VIC = LAMbS_VIC_get(&now);

    budget_struct_p = container_of(notifier, struct pbs_budget_struct, pin_notifier);
    SRT_struct_p = container_of(budget_struct_p, struct SRT_struct, budget_struct);

    //check if the task is already awake
    if((budget_struct_p->flags & PBS_BUDGET_SLEEPING) == 0)
    {
        //FIXME
        {
            static unsigned char flag = 1;

            if(flag)
            {
                printk(KERN_INFO "WARNING: pbs_budget_schedin called for task whose "
                                "flag does not have PBS_BUDGET_SLEEPING set!");
                flag = 0;
            }
        }
    }

    //check if the task is throttled, then put it back to sleep again
    //since the kernel can only refresh bandwidths for and unthrottle tasks
    //on behalf of the allocator, the task will remain throttle at least until
    //the allocator runs, which can't happen before this function ends.
    if(budget_struct_p->flags & PBS_BUDGET_THROTTLED)
    {
        set_task_state(SRT_struct_p->task, TASK_UNINTERRUPTIBLE);
        set_tsk_need_resched(SRT_struct_p->task);
    }
    else
    {
        //set the start-time variable to the current time
        budget_struct_p->last_actv_time = now;
        budget_struct_p->jb_actv_time   = now;
        budget_struct_p->jb_actv_time2  = now;

        //set the start-VIC variables to the current VIC
        budget_struct_p->last_actv_VIC  = now_VIC;
        budget_struct_p->jb_actv_VIC    = now_VIC;
        budget_struct_p->jb_actv_VIC2   = now_VIC;

        //reset the SLEEPING flag
        budget_struct_p->flags &= (~PBS_BUDGET_SLEEPING);

        //start the budget enforcement timer
        _budget_enforcement_timer_start(budget_struct_p);
    }
}

static void pbs_budget_schedout(struct preempt_notifier *notifier, 
                                struct task_struct *next)
{
    struct SRT_struct           *SRT_struct_p;
    struct pbs_budget_struct    *budget_struct_p;

    unsigned long irq_flags;

    u64 now;
    s64 current_runtime;
    s64 current_runtime2;
    s64 budget_used;

    u64 now_VIC;
    s64 current_runVIC;
    s64 current_runVIC2;
    s64 VICbudget_used;
    
    budget_struct_p = container_of(notifier, struct pbs_budget_struct, pin_notifier);
    SRT_struct_p = container_of(budget_struct_p, struct SRT_struct, budget_struct);

    //check if the task is already sleeping
    if(budget_struct_p->flags & PBS_BUDGET_SLEEPING)
    {
        //FIXME
        if((budget_struct_p->flags & PBS_BUDGET_THROTTLED) == 0)
        {
            static unsigned char flag = 1;

            if(flag)
            {
                printk(KERN_INFO "WARNING: pbs_budget_schedout called for task whose "
                                "flag is set to PBS_BUDGET_SLEEPING, but not "
                                "PBS_BUDGET_THROTTLED!");
                flag = 0;
            }
        }

        //there should be nothing to do
        return;
    }

    //cancel the bandwidth enforcement timer
    hrtimer_cancel(&(SRT_struct_p->budget_struct.budget_enforcement_timer));

    //load the scheduling period activation time and job activation time
    current_runtime = -(budget_struct_p->jb_actv_time);
    current_runtime2 = -(budget_struct_p->jb_actv_time2);
    budget_used = -(budget_struct_p->last_actv_time);

    //load the scheduling period activation VIC and job activation VIC
    current_runVIC = -(budget_struct_p->jb_actv_VIC);
    current_runVIC2 = -(budget_struct_p->jb_actv_VIC2);
    VICbudget_used = -(budget_struct_p->last_actv_VIC);    

    //allow the next set of operations to be performed atomically
    local_irq_save(irq_flags);

        //obtain the current time
        now_VIC = LAMbS_VIC_get(&now);
        
        //compute the runtime in this activation for the job
        current_runtime += now;
        current_runtime2 += now;
        
        //compute the runtime in this activation (maybe less than job runtime, 
        //because of job transition)
        budget_used += now;

        //set the total job runtime to the sum of the curent job runtime 
        //and previous accumulated job runtime
        budget_struct_p->total_jb_runtime   += current_runtime;
        budget_struct_p->total_jb_runtime2  += current_runtime2;

        //set the total sp runtime to the sum of the runtime in this activation
        //and previous accumulated sp runtimes
        budget_struct_p->total_sp_runtime   += budget_used;

        //compute the runVIC in this activation for the job
        current_runVIC += now_VIC;
        current_runVIC2 += now_VIC;
        
        //compute the runtime in this activation (maybe less than job runtime, 
        //because of job transition)
        VICbudget_used += now_VIC;

        //set the total job runtime to the sum of the curent job runtime 
        //and previous accumulated job runtime
        budget_struct_p->total_jb_runVIC    += current_runVIC;
        budget_struct_p->total_jb_runVIC2   += current_runVIC2;

        //set the total sp runtime to the sum of the runtime in this activation
        //and previous accumulated sp runtimes
        budget_struct_p->total_sp_runVIC    += VICbudget_used;

        //set the SLEEPING flag
        budget_struct_p->flags |= PBS_BUDGET_SLEEPING;

    local_irq_restore(irq_flags);
}

void pbs_budget_init(struct SRT_struct *SRT_struct_p)
{
    struct pbs_budget_struct    *budget_struct_p;

    budget_struct_p = &(SRT_struct_p->budget_struct);

    //initialize the bw enforcement timer
    hrtimer_init(   &(budget_struct_p->budget_enforcement_timer), 
                    CLOCK_MONOTONIC, 
                    HRTIMER_MODE_REL);
    (budget_struct_p->budget_enforcement_timer).function = 
                                                _budget_enforcement_timer_callback;

    //initialize the preempt notifier
    preempt_notifier_init(  &(budget_struct_p->pin_notifier), 
                            &pbs_budget_pops);
    hlist_add_head( &(budget_struct_p->pin_notifier.link), 
                    &(SRT_struct_p->task->preempt_notifiers));

    printk(KERN_INFO "pbs_budget_init called %d", SRT_struct_p->task->pid);
}


void pbs_budget_uninit(struct SRT_struct *SRT_struct_p)
{
    //uninitialize the the preempt_notifier
    hlist_del(&(SRT_struct_p->budget_struct.pin_notifier.link));

    //cancel the bandwidth enforcement timer if it is active
    hrtimer_cancel(&(SRT_struct_p->budget_struct.budget_enforcement_timer));

    printk(KERN_INFO "pbs_budget_uninit called %d", SRT_struct_p->task->pid);

    printk(KERN_INFO "The task overused its allocation by more than %dns," 
                    " %d times. Max overuse: %llu\n",
                        THROTTLE_THREASHOLD,
                        SRT_struct_p->overuse_count,
                        (unsigned long long)SRT_struct_p->maximum_overuse);
}

