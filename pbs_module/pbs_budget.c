#include "bw_mgt.h"
#include "jb_mgt.h"

#include "pbs_budget.h"

#include "LAMbS_VIC.h"
/*
    LAMbS_clock
    LAMbS_VIC_get
*/

enum pbs_budget_type {PBS_BUDGET_VIC, PBS_BUDGET_ns};

enum pbs_budget_type budget_type = PBS_BUDGET_ns;

#define PBS_BUDGET_IS_THROTTLED(budget_struct_p)   \
        (budget_struct_p->flags & PBS_BUDGET_THROTTLED)
#define PBS_BUDGET_IS_SLEEPING(budget_struct_p)    \
        (budget_struct_p->flags & PBS_BUDGET_SLEEPING)

void pbs_budget_firstjob(struct SRT_struct *ss)
{
    u64 now, now_VIC;

    struct pbs_budget_struct *budget_struct_p;    

    budget_struct_p = &(ss->budget_struct);

    /*  obtain the current time and VIC */
    now_VIC = LAMbS_VIC_get(&now);

    /*  Set this point as the beginning of the first job and first reservation period */
    pbs_budget_ns_firstjob(budget_struct_p, now);
    pbs_budget_VIC_firstjob(budget_struct_p, now_VIC);
    
    (ss->loaddata)->current_runtime = 0;

    (ss->log).runtime2  = 0;
    (ss->log).runVIC2   = 0;
    (ss->log).last_sp_budget_allocated  = (budget_struct_p->sp_budget);
    (ss->log).last_sp_budget_used_sofar = 0;
}

/*Job boundary according to the traditional definition of job:
    sleep to sleep*/
void pbs_budget_jobboundary1(struct SRT_struct *ss)
{
    u64 now, now_VIC;

    s64 current_rp_budgetusage;

    struct pbs_budget_struct *budget_struct_p;

    budget_struct_p = &(ss->budget_struct);

    /*  obtain the current time and VIC */
    now_VIC = LAMbS_VIC_get(&now);

    /*  update the loaddata structure for the next job */
    (ss->loaddata)->current_runtime = 0;
    (ss->loaddata)->queue_length = (unsigned short)(ss->queue_length);

    /*  write information regarding the completed job into the log  */
    (ss->log).last_sp_budget_allocated   = (budget_struct_p->sp_budget);
    
    /*  Determine the cpu-usage in the current reservation period*/
    if( PBS_BUDGET_VIC == budget_type)
    {
        current_rp_budgetusage = pbs_budget_VIC_get_rpusage(budget_struct_p, now_VIC, 0);
    }
    else
    {
        current_rp_budgetusage = pbs_budget_ns_get_rpusage(budget_struct_p, now, 0);
    }
    
    pbs_budget_VIC_jobboundary1(budget_struct_p, now_VIC);
    pbs_budget_ns_jobboundary1(budget_struct_p, now);
    
    /*  When logs are analyzed, the budget usage in the reservation period can be used to
        compute VFT error.*/
    (ss->log).last_sp_budget_used_sofar  = current_rp_budgetusage;
}

/*Job boundary according to the second definition of job:
    Prediction to Prediction*/
void pbs_budget_jobboundary2(struct SRT_struct *ss)
{
    u64 now, now_VIC;

    struct pbs_budget_struct *budget_struct_p;

    budget_struct_p = &(ss->budget_struct);

    /*obtain the current time and VIC*/
    now_VIC = LAMbS_VIC_get(&now);
    
    /*perform accounting related to jobboundary 2*/
    /*write information regarding the completed job into the log*/
    (ss->log).runtime2  = pbs_budget_ns_get_jbusage2( budget_struct_p, now, 0);
    (ss->log).runVIC2   = pbs_budget_VIC_get_jbusage2( budget_struct_p, now_VIC, 0);
   
    pbs_budget_ns_jobboundary2(budget_struct_p, now);
    pbs_budget_VIC_jobboundary2(budget_struct_p, now_VIC);
}

void pbs_budget_refresh(struct SRT_struct *SRT_struct_p)
{
    struct pbs_budget_struct *budget_struct_p;

    u64 now, now_VIC;

    int is_sleeping;

    budget_struct_p = &(SRT_struct_p->budget_struct);
    
    is_sleeping = PBS_BUDGET_IS_SLEEPING(budget_struct_p);
    
    /* Obtain the current time and VIC */
    now_VIC = LAMbS_VIC_get(&now);

    /*  Start the budget_enforcement_timer */
    if( PBS_BUDGET_VIC == budget_type)
    {
        (SRT_struct_p->loaddata)->current_runtime = 
                                pbs_budget_VIC_get_jbusage1(budget_struct_p,
                                                            now_VIC, is_sleeping);
    }
    else
    {
        (SRT_struct_p->loaddata)->current_runtime = 
                                pbs_budget_ns_get_jbusage1( budget_struct_p,
                                                            now, is_sleeping);
    }

    /*  Accumulate the actual time and VIC usage over the current reservation period*/
    SRT_struct_p->summary.consumed_budget += 
                                pbs_budget_ns_get_rpusage(  budget_struct_p,
                                                            now_VIC, is_sleeping);
    
    SRT_struct_p->summary.consumed_VIC += 
                                pbs_budget_VIC_get_rpusage( budget_struct_p,
                                                            now, is_sleeping);

    /*  Refresh the budgets  */
    pbs_budget_ns_refresh(budget_struct_p);
    pbs_budget_VIC_refresh(budget_struct_p);

    /*  Check if the task was previously throttled and unthrottle it*/
    if(budget_struct_p->flags & PBS_BUDGET_THROTTLED)
    {
        budget_struct_p->flags &= (~PBS_BUDGET_THROTTLED);

        /*  Only wakeup the throttled task if it has work left to do (non-zero queue 
            length) */
        if((SRT_struct_p->queue_length) > 0)
        {
            wake_up_process(SRT_struct_p->task);
        }
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
        SRT_struct_p = container_of(    budget_struct_p, 
                                        struct SRT_struct, 
                                        budget_struct);
        set_task_state(SRT_struct_p->task, TASK_UNINTERRUPTIBLE);
        set_tsk_need_resched(SRT_struct_p->task);
    }
    else
    {
        /*set the start-time and start-VIC variables to the current time and current VIC*/
        pbs_budget_ns_schedin( budget_struct_p, now);
        pbs_budget_VIC_schedin( budget_struct_p, now_VIC);

        /* unset the SLEEPING flag */
        budget_struct_p->flags &= (~PBS_BUDGET_SLEEPING);

        /* Start the budget_enforcement_timer */
        if( PBS_BUDGET_VIC == budget_type)
        {
            pbs_budget_ns_BET_start(budget_struct_p);
        }
        else
        {
            pbs_budget_VIC_BET_start(budget_struct_p);
        }
    }
}

static void pbs_budget_schedout(struct preempt_notifier *notifier, 
                                struct task_struct *next)
{
    struct pbs_budget_struct    *budget_struct_p;

    unsigned long irq_flags;

    u64 now;
    u64 now_VIC;
    
    budget_struct_p = container_of(notifier, struct pbs_budget_struct, pin_notifier);

    /* Check if the task is already sleeping */
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

        /*The task is already in the SLEEPING state, just exit*/
        return;
    }

    /* Cancel the budget_enforcement_timer */
    if( PBS_BUDGET_VIC == budget_type)
    {
        pbs_budget_VIC_BET_cancel(budget_struct_p);
    }
    else
    {
        /*The default assumption is ns-type budget.*/
        pbs_budget_ns_BET_cancel(budget_struct_p);
    }

    /*  Allow the next set of operations to be performed atomically */
    local_irq_save(irq_flags);

        /*  obtain the current time */
        now_VIC = LAMbS_VIC_get(&now);
        
        pbs_budget_ns_schedout( budget_struct_p, now);
        pbs_budget_VIC_schedout( budget_struct_p, now_VIC);

        /*  set the SLEEPING flag   */
        budget_struct_p->flags |= PBS_BUDGET_SLEEPING;

    local_irq_restore(irq_flags);
}

void pbs_budget_init(struct SRT_struct *SRT_struct_p)
{
    struct pbs_budget_struct    *budget_struct_p;

    budget_struct_p = &(SRT_struct_p->budget_struct);

    /*  Initialize the budget_enforcement_timer and anything else that needs to be 
        initialized, based on the type of budget being used*/
    if( PBS_BUDGET_VIC == budget_type)
    {
        pbs_budget_VIC_init( &(SRT_struct_p->budget_struct));
    }
    else
    {
        /*The default assumption is ns-type budget.*/
        pbs_budget_ns_init( &(SRT_struct_p->budget_struct));
    }

    /*initialize the preempt notifier*/
    preempt_notifier_init(  &(budget_struct_p->pin_notifier), 
                            &pbs_budget_pops);
    preempt_notifier_register(&(budget_struct_p->pin_notifier));

    printk(KERN_INFO "pbs_budget_init called %d", SRT_struct_p->task->pid);
}


void pbs_budget_uninit(struct SRT_struct *SRT_struct_p)
{
    /*Unhook the preemption callback function*/
    preempt_notifier_unregister(&(SRT_struct_p->budget_struct.pin_notifier));

    /* Cancel the budget_enforcement_timer based on the type of budget being used*/
    if( PBS_BUDGET_VIC == budget_type)
    {
        pbs_budget_VIC_uninit( &(SRT_struct_p->budget_struct));
    }
    else
    {
        /*The default assumption is ns-type budget.*/
        pbs_budget_ns_uninit( &(SRT_struct_p->budget_struct));
    }

    printk(KERN_INFO "pbs_budget_uninit called %d", SRT_struct_p->task->pid);

    printk(KERN_INFO "The task overused its allocation %d times. Max overuse: %llu\n",
                    SRT_struct_p->overuse_count,
                    (unsigned long long)SRT_struct_p->maximum_overuse);
}
