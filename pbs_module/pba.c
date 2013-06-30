#include "bw_mgt.h"
#include "pba.h"

#include "LAMbS_VIC.h"
/*
    LAMbS_clock
    LAMbS_VIC_get
*/

#define PBA_IS_THROTTLED(pba_struct_p) (pba_struct_p->flags & PBA_THROTTLED)
#define PBA_IS_SLEEPING(pba_struct_p)  (pba_struct_p->flags & PBA_SLEEPING)

#define THROTTLE_THREASHOLD_NS 5000


u64 pba_get_jbruntime(struct pba_struct *pba_struct_p)
{
    u64 now;

    u64 total_runtime, current_runtime = 0;

    //if the task is currently running, compute the runtime since
    //the last activation
    if(!PBA_IS_SLEEPING(pba_struct_p))
    {
        //obtain the current time
        now = LAMbS_clock();
        //compute the runtime in this activation
        current_runtime = now - pba_struct_p->jb_actv_time;
    }

    //set the total runtime to the sum of the curent runtime 
    //and accumulated previous runtime
    total_runtime = current_runtime + pba_struct_p->total_jb_runtime;

    return total_runtime;
}

void pba_firstjob(struct SRT_struct *ss)
{
    u64 now;

    struct pba_struct *pba_struct_p;    

    pba_struct_p = &(ss->pba_struct);

    //obtain the current time
    now = LAMbS_clock();

    (ss->loaddata)->current_runtime = 0;

    (ss->log).runtime = 0;
    (ss->log).runtime2 = 0;
    (ss->log).last_sp_compt_allocated   = (pba_struct_p->sp_budget);
    (ss->log).last_sp_compt_used_sofar  = 0;

    //reset the total accumulated runtime to 0
    pba_struct_p->total_jb_runtime = 0;
    pba_struct_p->total_jb_runtime2 = 0;
    
    //set now as the beginning of the new job
    pba_struct_p->jb_actv_time = now;
    //also set now as the beginning of the new job based on the second 
    //definition of job, although this is slightly incorrect, it will only
    //be used for the first job
    pba_struct_p->jb_actv_time2 = now;
    
    //reset the budget used to 0
    pba_struct_p->total_sp_runtime = 0;
    //set now as the beginning of current activation
    pba_struct_p->last_actv_time = now;

}

/*Job boundary according to the traditional definition of job*/
void pba_nextjob(struct SRT_struct *ss)
{
    u64 now, current_jb_runtime, current_sp_runtime;

    u64 total_runtime;

    struct pba_struct *pba_struct_p;

    pba_struct_p = &(ss->pba_struct);

    //obtain the current time
    now = LAMbS_clock();

    //compute the runtime in this activation
    current_jb_runtime = now - (pba_struct_p->jb_actv_time);
    current_sp_runtime = now - (pba_struct_p->last_actv_time);

    //set the total runtime to the sum of the curent runtime 
    //and accumulated previous runtime
    total_runtime = current_jb_runtime + (pba_struct_p->total_jb_runtime);

    //update the loaddata structure for the next job
    (ss->loaddata)->current_runtime = 0;
    (ss->loaddata)->queue_length = (unsigned short)(ss->queue_length);

    //write information regarding the completed job into the log
    (ss->log).runtime = total_runtime;
    (ss->log).last_sp_compt_allocated   = (pba_struct_p->sp_budget);
    (ss->log).last_sp_compt_used_sofar  = current_sp_runtime;

    //reset the total accumulated runtime to 0
    pba_struct_p->total_jb_runtime = 0;
    //set now as the beginning of the new job
    pba_struct_p->jb_actv_time = now;
}

/*Job boundary according to the second definition of job*/
void pba_nextjob2(struct SRT_struct *ss)
{
    u64 now, current_jb_runtime2;

    u64 total_runtime2;

    struct pba_struct *pba_struct_p;

    pba_struct_p = &(ss->pba_struct);

    //obtain the current time
    now = LAMbS_clock();

    //compute the runtime in this activation
    current_jb_runtime2 = now - (pba_struct_p->jb_actv_time2);
    
    //set the total runtime to the sum of the curent runtime 
    //and accumulated previous runtime
    total_runtime2 = current_jb_runtime2 + (pba_struct_p->total_jb_runtime2);

    //write information regarding the completed job into the log
    (ss->log).runtime2 = total_runtime2;
    
    //reset the total accumulated runtime to 0
    pba_struct_p->total_jb_runtime2 = 0;
    
    //set now as the beginning of the new job
    pba_struct_p->jb_actv_time2 = now;
}

////this is called for the purpose of checking if budget has expired
//this is also called to determine how much budget remains after a 
//job completes
u64 pba_get_spruntime(struct pba_struct *pba_struct_p)
{
    u64 now;

    u64 total_runtime, current_runtime = 0;

    //if the task is currently running, compute the runtime since
    //the last activation
    if(!PBA_IS_SLEEPING(pba_struct_p))
    {
        //obtain the current time
        now = LAMbS_clock();
        current_runtime = now - pba_struct_p->last_actv_time;
    }

    //set the total runtime to the sum of the curent accumulated 
    //runtime and previous runtime
    total_runtime = current_runtime + pba_struct_p->total_sp_runtime;

    return total_runtime;
}

//FIXME
u64 check_budget_remaining(struct pba_struct *pba_struct_p)
{
    struct SRT_struct *SRT_struct_p;
    s64 runtime;
    s64 remaining;

    SRT_struct_p = container_of(pba_struct_p, struct SRT_struct, pba_struct);

    //determine the amount of time remaining until the bandwidth expires
    runtime = (s64)pba_get_spruntime(pba_struct_p);
    remaining = (s64)pba_struct_p->sp_budget - runtime;

    if(remaining < THROTTLE_THREASHOLD_NS)  
    {
        remaining = 0;

        if((pba_struct_p->flags & PBA_THROTTLED) == 0)
        {
            if( SRT_struct_p->task != current)
            {
                static unsigned char flag = 1;

                if(flag)
                {
                    flag = 0;
                    printk( KERN_INFO 
                            "WARNING: (pba_struct_throttle_curr) acct error\n");
                }
            }

            pba_struct_p->flags |= PBA_THROTTLED;

            //throttle the task (by putting it to sleep)
            set_current_state(TASK_UNINTERRUPTIBLE);
            set_tsk_need_resched(SRT_struct_p->task);
        }
    }

    return remaining;
}

void pba_refresh_budget(struct SRT_struct *SRT_struct_p)
{
    struct pba_struct *pba_struct_p;
    s64 remaining;

    pba_struct_p = &(SRT_struct_p->pba_struct);

    (SRT_struct_p->loaddata)->current_runtime = pba_get_jbruntime(pba_struct_p);

    //determine the amount of time remaining until the bandwidth expires
    remaining
    = ((s64)pba_struct_p->sp_budget - (s64)pba_get_spruntime(pba_struct_p));

    if(remaining < 0)
    {
        u64 overuse = -remaining;

        if(overuse > THROTTLE_THREASHOLD_NS)
        {
            SRT_struct_p->overuse_count++;
        }

        if(overuse > SRT_struct_p->maximum_overuse)
        {
            SRT_struct_p->maximum_overuse = overuse;
        }
    }

    //accumulate the budget used
    SRT_struct_p->summary.consumed_budget = 
        SRT_struct_p->summary.consumed_budget + pba_struct_p->total_sp_runtime;
    pba_struct_p->total_sp_runtime = 0;

    if(pba_struct_p->flags & PBA_THROTTLED)
    {
        pba_struct_p->flags &= (~PBA_THROTTLED);

        //wakeup the throttled task, if it has work left to do
        if((SRT_struct_p->queue_length) > 0)
        {
            wake_up_process(SRT_struct_p->task);
        }
    }

}

enum hrtimer_restart pbs_hrtick(struct hrtimer *timer)
{
    struct pba_struct *pba_struct_p;
    s64 remaining;
    ktime_t remaining_k;

    enum hrtimer_restart ret = HRTIMER_NORESTART;

    if(allocator_state != ALLOCATOR_LOOP)
    {
        return ret;
    }

    pba_struct_p = container_of(timer, struct pba_struct, hrtimer);

    remaining = check_budget_remaining(pba_struct_p);

    if(remaining > 0)
    {
        //forward the timer by the necessary amount
        remaining = remaining - (THROTTLE_THREASHOLD_NS/2);
        remaining_k.tv64 = remaining;
        hrtimer_forward_now(timer, remaining_k);
        ret = HRTIMER_RESTART;
    }

    return ret;
}

static void inline pbs_hrtick_start(struct pba_struct *pba_struct_p)
{
    s64 remaining;
    ktime_t remaining_k;

    //make sure the scheduler is still in the right mode
    if(allocator_state != ALLOCATOR_LOOP)
    {
        return;
    }

    //check the time that this should be setup for
    remaining = check_budget_remaining(pba_struct_p);

    if(remaining > 0)
    {
        //setup the timer to go off after the necessary time
        remaining = remaining - (THROTTLE_THREASHOLD_NS/2);
        remaining_k.tv64 = remaining;
        hrtimer_start(&(pba_struct_p->hrtimer), remaining_k, HRTIMER_MODE_REL);
    }
}

void pba_schedin(   struct preempt_notifier *notifier, 
                    int cpu);

void pba_schedout(  struct preempt_notifier *notifier, 
                    struct task_struct *next);

static struct preempt_ops pba_pops = {
                            .sched_in = pba_schedin,
                            .sched_out= pba_schedout,
                         };

void pba_schedin(   struct preempt_notifier *notifier, 
                    int cpu)
{
    u64 now;

    struct SRT_struct *SRT_struct_p;
    struct pba_struct *pba_struct_p;

    now = LAMbS_clock();

    pba_struct_p = container_of(notifier, struct pba_struct, pin_notifier);
    SRT_struct_p = container_of(pba_struct_p, struct SRT_struct, pba_struct);

    //check if the task is already awake
    if((pba_struct_p->flags & PBA_SLEEPING) == 0)
    {
        //FIXME
        {
            static unsigned char flag = 1;

            if(flag)
            {
                printk(KERN_INFO "WARNING: pba_schedin called for task whose flag does not have PBA_SLEEPING set!");
                flag = 0;
            }
        }
    }

    //check if the task is throttled, then put it back to sleep again
    //since the kernel can only refresh bandwidths for and unthrottle tasks
    //on behalf of the allocator, the task will remain throttle at least until
    //the allocator runs, which can't happen before this function ends.
    if(pba_struct_p->flags & PBA_THROTTLED)
    {
        set_task_state(SRT_struct_p->task, TASK_UNINTERRUPTIBLE);
        set_tsk_need_resched(SRT_struct_p->task);
    }
    else
    {
        //set the start-time variable to the current time
        pba_struct_p->last_actv_time = now;
        pba_struct_p->jb_actv_time = now;
        pba_struct_p->jb_actv_time2 = now;

        //reset the SLEEPING flag
        pba_struct_p->flags &= (~PBA_SLEEPING);

        //start the bandwidth enforcement timer
        pbs_hrtick_start(pba_struct_p);
    }
}

void pba_schedout(  struct preempt_notifier *notifier, 
                    struct task_struct *next)
{
    struct SRT_struct *SRT_struct_p;
    struct pba_struct *pba_struct_p;

    unsigned long irq_flags;

    u64 now;
    s64 current_runtime;
    s64 current_runtime2;
    s64 budget_used;

    pba_struct_p = container_of(notifier, struct pba_struct, pin_notifier);
    SRT_struct_p = container_of(pba_struct_p, struct SRT_struct, pba_struct);

    //check if the task is already sleeping
    if(pba_struct_p->flags & PBA_SLEEPING)
    {
        //FIXME
        if((pba_struct_p->flags & PBA_THROTTLED) == 0)
        {
            static unsigned char flag = 1;

            if(flag)
            {
                printk(KERN_INFO "WARNING: pba_schedout called for task whose flag is set to PBA_SLEEPING, but not PBA_THROTTLED!");
                flag = 0;
            }
        }

        //there should be nothing to do
        return;
    }

    //cancel the bandwidth enforcement timer
    hrtimer_cancel(&(SRT_struct_p->pba_struct.hrtimer));

    //load the scheduling period activation time and job activation time
    current_runtime = -(pba_struct_p->jb_actv_time);
    current_runtime2 = -(pba_struct_p->jb_actv_time2);
    budget_used = -(pba_struct_p->last_actv_time);

    //allow the next set of operations to be performed atomically
    local_irq_save(irq_flags);

        //obtain the current time
        now = LAMbS_clock();

        //compute the runtime in this activation for the job
        current_runtime += now;
        current_runtime2 += now;
                
        //compute the runtime in this activation (maybe less than job runtime, 
        //because of job transition)
        budget_used += now;

        //set the total job runtime to the sum of the curent job runtime 
        //and previous accumulated job runtime
        pba_struct_p->total_jb_runtime += current_runtime;
        pba_struct_p->total_jb_runtime2 += current_runtime2;

        //set the total sp runtime to the sum of the runtime in this activation
        //and previous accumulated sp runtimes
        pba_struct_p->total_sp_runtime += budget_used;

        //set the SLEEPING flag
        pba_struct_p->flags |= PBA_SLEEPING;

    local_irq_restore(irq_flags);
}

void pba_init(struct SRT_struct *SRT_struct_p)
{
    struct pba_struct *pba_struct_p;

    pba_struct_p = &(SRT_struct_p->pba_struct);

    //initialize the bw enforcement timer
    hrtimer_init(   &(pba_struct_p->hrtimer), 
                    CLOCK_MONOTONIC, 
                    HRTIMER_MODE_REL);
    (pba_struct_p->hrtimer).function = pbs_hrtick;

    //initialize the preempt notifier
    preempt_notifier_init(  &(pba_struct_p->pin_notifier), 
                            &pba_pops);
    hlist_add_head( &(pba_struct_p->pin_notifier.link), 
                    &(SRT_struct_p->task->preempt_notifiers));

    printk(KERN_INFO "pba_init called %d", SRT_struct_p->task->pid);
}


void pba_uninit(struct SRT_struct *SRT_struct_p)
{
    //uninitialize the the preempt_notifier
    hlist_del(&(SRT_struct_p->pba_struct.pin_notifier.link));

    //cancel the bandwidth enforcement timer if it is active
    hrtimer_cancel(&(SRT_struct_p->pba_struct.hrtimer));

    printk(KERN_INFO "pba_uninit called %d", SRT_struct_p->task->pid);

    printk(KERN_INFO "The task overused its allocation by more than %dns," 
                    " %d times. Max overuse: %llu\n",
                        THROTTLE_THREASHOLD_NS,
                        SRT_struct_p->overuse_count,
                        (unsigned long long)SRT_struct_p->maximum_overuse);
}

