#include <linux/kernel.h>
#include <linux/hrtimer.h>

#include "LAMbS_VIC.h"
#include "LAMbS_VICtimer.h"

#include "pbs_budget_VIC_helper.h"
#include "pbs_budget.h"
#include "jb_mgt.h"
#include "bw_mgt.h"

/*Get the runtime of the currently running job of the task*/
u64 pbs_budget_VIC_get_jbusage1(struct pbs_budget_struct *budget_struct_p,
                                u64 now_VIC,    int is_sleeping)
{
    u64 total_VIC, current_VIC;

    struct pbs_budget_VIC_struct *budget_VIC_struct_p;
    
    budget_VIC_struct_p = &(budget_struct_p->budget_VIC_struct);

    /*  if the task is currently running, compute the runtime since
        the last activation of this job*/
    current_VIC =   (!is_sleeping) ? 
                    (now_VIC - budget_VIC_struct_p->jb_actv_VIC) : 
                    0;

    /*  set the total runtime to the sum of the curent accumulated 
        runtime and previous runtime */
    total_VIC = current_VIC + budget_VIC_struct_p->total_jb_VIC;

    return total_VIC;
}

u64 pbs_budget_VIC_get_jbusage2(struct pbs_budget_struct *budget_struct_p,
                                u64 now_VIC,    int is_sleeping)
{
    u64 total_VIC, current_VIC;

    struct pbs_budget_VIC_struct *budget_VIC_struct_p;
    
    budget_VIC_struct_p = &(budget_struct_p->budget_VIC_struct);

    /*  if the task is currently running, compute the runtime since
        the last activation of this job*/
    current_VIC =   (!is_sleeping) ? 
                    (now_VIC - budget_VIC_struct_p->jb_actv_VIC2) : 
                    0;

    /*  set the total runtime to the sum of the curent accumulated 
        runtime and previous runtime */
    total_VIC = current_VIC + budget_VIC_struct_p->total_jb_VIC2;

    return total_VIC;
}


/*Get the runtime of the of the task so far in the current reservation period*/
u64 pbs_budget_VIC_get_rpusage( struct pbs_budget_struct *budget_struct_p,
                                u64 now_VIC,    int is_sleeping)
{
    u64 total_VIC, current_VIC;

    struct pbs_budget_VIC_struct *budget_VIC_struct_p;
    
    budget_VIC_struct_p = &(budget_struct_p->budget_VIC_struct);

    /*  if the task is currently running, compute the runtime since
        the last activation of this task in this reservation period*/
    current_VIC =   (!is_sleeping) ? 
                    (now_VIC - budget_VIC_struct_p->last_actv_VIC) :
                    0;

    /*  set the total runtime to the sum of the curent accumulated 
        runtime and previous runtime */
    total_VIC = current_VIC + budget_VIC_struct_p->total_rp_VIC;

    return total_VIC;
}

void pbs_budget_VIC_firstjob(   struct pbs_budget_struct *budget_struct_p,
                                u64 now_VIC)
{
    struct pbs_budget_VIC_struct *budget_VIC_struct_p;    

    budget_VIC_struct_p = &(budget_struct_p->budget_VIC_struct);
    
    //set now_VIC as the beginning of the new job
    budget_VIC_struct_p->jb_actv_VIC    = now_VIC;
    
    //also set now_VIC as the beginning of the new job based on the second 
    //definition of job, although this is slightly incorrect, it will only
    //be used for the first job
    budget_VIC_struct_p->jb_actv_VIC2   = now_VIC;

    //reset the total accumulated VIC to 0
    budget_VIC_struct_p->total_jb_VIC   = 0;
    budget_VIC_struct_p->total_jb_VIC2  = 0;
    
    //set now_VIC as the beginning of current activation
    budget_VIC_struct_p->last_actv_VIC  = now_VIC;
    
    //reset the budget used to 0
    budget_VIC_struct_p->total_rp_VIC   = 0;    
}

/*  Job boundary according to the traditional definition of job:
    sleep to sleep*/
void pbs_budget_VIC_jobboundary1(struct pbs_budget_struct *budget_struct_p,
                                u64 now_VIC)
{
    struct pbs_budget_VIC_struct *budget_VIC_struct_p;

    budget_VIC_struct_p = &(budget_struct_p->budget_VIC_struct);

    /*  set now_VIC and now_VIC as the type1 beginning of the new job */
    budget_VIC_struct_p->jb_actv_VIC    = now_VIC;

    /*  reset the total accumulated job type1 VIC to 0 */
    budget_VIC_struct_p->total_jb_VIC   = 0;    
}

/*  Job boundary according to the second definition of job:
    Prediction to Prediction*/
void pbs_budget_VIC_jobboundary2(   struct pbs_budget_struct *budget_struct_p,
                                    u64 now_VIC)
{
    struct pbs_budget_VIC_struct *budget_VIC_struct_p;

    budget_VIC_struct_p = &(budget_struct_p->budget_VIC_struct);

    //reset the total accumulated VIC to 0
    budget_VIC_struct_p->total_jb_VIC2  = 0;
    
    //set now_VIC as the beginning of the new job
    budget_VIC_struct_p->jb_actv_VIC2   = now_VIC;
}

#define BUDGET_VIC_THROTTLE_THREASHOLD  (LAMbS_VICtimer_threshold_VIC)

static u64 check_budget_VIC_remaining(  struct pbs_budget_VIC_struct *budget_VIC_struct_p,
                                        u64 *now_VIC_p)
{
    struct pbs_budget_struct *budget_struct_p;
    struct SRT_struct   *SRT_struct_p;
    
    u64 now_VIC;
    
    s64 VIC;
    s64 remaining;
    s64 overuse;

    /*Climb super struct pointers upto the SRT struct*/
    budget_struct_p = container_of( budget_VIC_struct_p, 
                                    struct pbs_budget_struct, 
                                    budget_VIC_struct);
                                    
    SRT_struct_p    = container_of( budget_struct_p, 
                                    struct SRT_struct, 
                                    budget_struct);

    /*  determine the amount of VIC remaining until the bandwidth expires  */
    
    /*  check_budget_remaining is always called in the context of the SRT task itself */
    /*  The following branch should never be taken  */
    if( SRT_struct_p->task != current)
    {
        printk( KERN_INFO 
                "check_budget_remaining: WARNING called in the context of a task "
                "other than the checked task\n");
    }

    now_VIC = LAMbS_VIC_get(NULL);
    *now_VIC_p = now_VIC;
    
    VIC =   pbs_budget_VIC_get_rpusage( budget_struct_p, 
                                        now_VIC,  
                                        (SRT_struct_p->task != current));
    
    remaining = (s64)budget_struct_p->sp_budget - VIC;

    if( remaining < BUDGET_VIC_THROTTLE_THREASHOLD)
    {
        /*Compute overuse statistics*/
        if(remaining < 0)
        {
            overuse = -remaining;

            if(overuse > BUDGET_VIC_THROTTLE_THREASHOLD)
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

static enum LAMbS_VICtimer_restart budget_VIC_BET_callback(
                                            struct LAMbS_VICtimer_s* timer,
                                            u64 now_VIC)
{
    struct pbs_budget_VIC_struct *budget_VIC_struct_p;
    s64 remaining;

    enum LAMbS_VICtimer_restart ret = LAMbS_VICTIMER_NORESTART;

    if( ALLOCATOR_LOOP != allocator_state)
    {
        return ret;
    }

    budget_VIC_struct_p = container_of( timer, 
                                        struct pbs_budget_VIC_struct, 
                                        budget_enforcement_timer);

    remaining = check_budget_VIC_remaining(budget_VIC_struct_p, &now_VIC);
    
    if(remaining > 0)
    {
        /*  forward the timer to the desired VIC target */
        timer->target_VIC = now_VIC + remaining;
        ret = LAMbS_VICTIMER_RESTART;
    }

    return ret;
}

void pbs_budget_VIC_BET_start(  struct pbs_budget_struct *budget_struct_p)
{
    struct pbs_budget_VIC_struct *budget_VIC_struct_p;
    
    s64 remaining;
    u64 now_VIC;

    budget_VIC_struct_p = &(budget_struct_p->budget_VIC_struct);

    /*  check the time that this should be setup for */
    remaining = check_budget_VIC_remaining(budget_VIC_struct_p, &now_VIC);

    if(remaining > 0)
    {
        /*  forward the timer by the necessary amount */
        LAMbS_VICtimer_start(   &(budget_VIC_struct_p->budget_enforcement_timer),
                                (now_VIC + remaining),
                                LAMbS_VICTIMER_ABS);

        min_timer_interval = (remaining < min_timer_interval)? 
                             remaining : min_timer_interval;
        max_timer_interval = (remaining > max_timer_interval)? 
                             remaining : max_timer_interval;
    }
}

void pbs_budget_VIC_schedin(struct pbs_budget_struct *budget_struct_p,
                            u64 now_VIC)
{
    struct pbs_budget_VIC_struct *budget_VIC_struct_p;
    
    budget_VIC_struct_p = &(budget_struct_p->budget_VIC_struct);
    
    /*  set the start-VIC variable to the current VIC */
    budget_VIC_struct_p->last_actv_VIC = now_VIC;
    budget_VIC_struct_p->jb_actv_VIC   = now_VIC;
    budget_VIC_struct_p->jb_actv_VIC2  = now_VIC;
}

void pbs_budget_VIC_schedout(struct pbs_budget_struct *budget_struct_p,
                             u64 now_VIC)
{
    struct pbs_budget_VIC_struct *budget_VIC_struct_p;
    
    s64 current_VIC;
    s64 current_VIC2;
    s64 budget_used;

    budget_VIC_struct_p = &(budget_struct_p->budget_VIC_struct);

    /*  compute the job VIC in this activation for the job */
    current_VIC =   now_VIC - budget_VIC_struct_p->jb_actv_VIC;
    current_VIC2=   now_VIC - budget_VIC_struct_p->jb_actv_VIC2;
    
    /*  compute the budget used in this activation (maybe more than job VIC, 
        because of job transition) */
    budget_used     =   now_VIC - budget_VIC_struct_p->last_actv_VIC;

    /*  set the total job VIC to the sum of the curent job VIC 
        and previous accumulated job VIC */
    budget_VIC_struct_p->total_jb_VIC    += current_VIC;
    budget_VIC_struct_p->total_jb_VIC2   += current_VIC2;

    /*  set the total sp VIC to the sum of the VIC in this activation
        and previous accumulated sp VICs */
    budget_VIC_struct_p->total_rp_VIC    += budget_used;
}

void pbs_budget_VIC_BET_cancel( struct pbs_budget_struct *budget_struct_p)
{
    struct pbs_budget_VIC_struct *budget_VIC_struct_p;
    
    budget_VIC_struct_p = &(budget_struct_p->budget_VIC_struct);

    /*  cancel the budget enforcement timer */
    LAMbS_VICtimer_cancel(&(budget_VIC_struct_p->budget_enforcement_timer));
}

void pbs_budget_VIC_init(struct pbs_budget_struct *budget_struct_p)
{
    struct pbs_budget_VIC_struct *budget_VIC_struct_p;
    
    budget_VIC_struct_p = &(budget_struct_p->budget_VIC_struct);

    /*Initialize the budget_enforcement_timer including the callback function*/
    LAMbS_VICtimer_init(&(budget_VIC_struct_p->budget_enforcement_timer));
                    
    (budget_VIC_struct_p->budget_enforcement_timer).function = budget_VIC_BET_callback;
}

void pbs_budget_VIC_uninit(  struct pbs_budget_struct *budget_struct_p)
{
    struct pbs_budget_VIC_struct *budget_VIC_struct_p;
    
    budget_VIC_struct_p = &(budget_struct_p->budget_VIC_struct);
    
    /*Cancel the budget_enforcement_timer*/
    LAMbS_VICtimer_cancel(&(budget_VIC_struct_p->budget_enforcement_timer));
}

