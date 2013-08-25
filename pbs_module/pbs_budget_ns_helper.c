#include <linux/kernel.h>
#include <linux/hrtimer.h>

#include "LAMbS_VIC.h"

#include "pbs_budget_ns_helper.h"
#include "pbs_budget.h"
#include "jb_mgt.h"
#include "bw_mgt.h"

/*Get the runtime of the currently running job of the task*/
u64 pbs_budget_ns_get_jbusage1( struct pbs_budget_struct *budget_struct_p,
                                u64 now,    int is_sleeping)
{
    u64 total_runtime, current_runtime;

    struct pbs_budget_ns_struct *budget_ns_struct_p;
    
    budget_ns_struct_p = &(budget_struct_p->budget_ns_struct);

    /*  if the task is currently running, compute the runtime since
        the last activation of this job*/
    current_runtime =   (!is_sleeping) ? 
                        (now - budget_ns_struct_p->jb_actv_time) : 
                        0;

    /*  set the total runtime to the sum of the curent accumulated 
        runtime and previous runtime */
    total_runtime = current_runtime + budget_ns_struct_p->total_jb_runtime;

    return total_runtime;
}

u64 pbs_budget_ns_get_jbusage2( struct pbs_budget_struct *budget_struct_p,
                                u64 now,    int is_sleeping)
{
    u64 total_runtime, current_runtime;

    struct pbs_budget_ns_struct *budget_ns_struct_p;
    
    budget_ns_struct_p = &(budget_struct_p->budget_ns_struct);

    /*  if the task is currently running, compute the runtime since
        the last activation of this job*/
    current_runtime =   (!is_sleeping) ? 
                        (now - budget_ns_struct_p->jb_actv_time2) : 
                        0;

    /*  set the total runtime to the sum of the curent accumulated 
        runtime and previous runtime */
    total_runtime = current_runtime + budget_ns_struct_p->total_jb_runtime2;

    return total_runtime;
}


/*Get the runtime of the of the task so far in the current reservation period*/
u64 pbs_budget_ns_get_rpusage(  struct pbs_budget_struct *budget_struct_p,
                                u64 now,    int is_sleeping)
{
    u64 total_runtime, current_runtime;

    struct pbs_budget_ns_struct *budget_ns_struct_p;
    
    budget_ns_struct_p = &(budget_struct_p->budget_ns_struct);

    /*  if the task is currently running, compute the runtime since
        the last activation of this task in this reservation period*/
    current_runtime =   (!is_sleeping) ? 
                        (now - budget_ns_struct_p->last_actv_time) :
                        0;

    /*  set the total runtime to the sum of the curent accumulated 
        runtime and previous runtime */
    total_runtime = current_runtime + budget_ns_struct_p->total_rp_runtime;

    return total_runtime;
}


void pbs_budget_ns_firstjob(struct pbs_budget_struct *budget_struct_p,
                            u64 now)
{
    struct pbs_budget_ns_struct *budget_ns_struct_p;    

    budget_ns_struct_p = &(budget_struct_p->budget_ns_struct);
    
    //reset the total accumulated runtime to 0
    budget_ns_struct_p->total_jb_runtime   = 0;
    budget_ns_struct_p->total_jb_runtime2  = 0;
    
    //set now as the beginning of the new job
    budget_ns_struct_p->jb_actv_time   = now;
    
    //also set now as the beginning of the new job based on the second 
    //definition of job, although this is slightly incorrect, it will only
    //be used for the first job
    budget_ns_struct_p->jb_actv_time2  = now;
    
    //reset the budget used to 0
    budget_ns_struct_p->total_rp_runtime   = 0;
    
    //set now as the beginning of current activation
    budget_ns_struct_p->last_actv_time     = now;    
}

/*Job boundary according to the traditional definition of job:
    sleep to sleep*/
void pbs_budget_ns_jobboundary1(struct pbs_budget_struct *budget_struct_p,
                                u64 now)
{
    struct pbs_budget_ns_struct *budget_ns_struct_p;

    budget_ns_struct_p = &(budget_struct_p->budget_ns_struct);

    /*  reset the total accumulated job type1 runtime to 0 */
    budget_ns_struct_p->total_jb_runtime   = 0;
    
    /*  set now and now as the type1 beginning of the new job */
    budget_ns_struct_p->jb_actv_time   = now;
}

/*Job boundary according to the second definition of job:
    Prediction to Prediction*/
void pbs_budget_ns_jobboundary2(struct pbs_budget_struct *budget_struct_p,
                                u64 now)
{
    struct pbs_budget_ns_struct *budget_ns_struct_p;

    budget_ns_struct_p = &(budget_struct_p->budget_ns_struct);

    //reset the total accumulated runtime to 0
    budget_ns_struct_p->total_jb_runtime2  = 0;
    
    //set now as the beginning of the new job
    budget_ns_struct_p->jb_actv_time2  = now;
}

#define BUDGET_NS_THROTTLE_THREASHOLD 5000

static u64 check_budget_ns_remaining(  struct pbs_budget_ns_struct *budget_ns_struct_p)
{
    struct pbs_budget_struct *budget_struct_p;
    struct SRT_struct   *SRT_struct_p;
    
    u64 now;
    
    s64 runtime;
    s64 remaining;
    s64 overuse;

    /*Climb super struct pointers upto the SRT struct*/
    budget_struct_p = container_of( budget_ns_struct_p, 
                                    struct pbs_budget_struct, 
                                    budget_ns_struct);
                                    
    SRT_struct_p = container_of(    budget_struct_p, 
                                    struct SRT_struct, 
                                    budget_struct);

    /*  get the current time */
    now = LAMbS_clock();
    
    /*  determine the amount of time remaining until the bandwidth expires  */
    
    /*  check_budget_remaining is always called in the context of the SRT task itself */
    /*  The following branch should never be taken  */
    if( SRT_struct_p->task != current)
    {
        printk( KERN_INFO 
                "check_budget_remaining: WARNING called in the context of a task "
                "other than the checked task\n");
    }

    runtime =   pbs_budget_ns_get_rpusage(  budget_struct_p, 
                                            now,  
                                            (SRT_struct_p->task != current));
    
    remaining = (s64)budget_struct_p->sp_budget - runtime;

    if(remaining < BUDGET_NS_THROTTLE_THREASHOLD)  
    {
        /*Compute overuse statistics*/
        if(remaining < 0)
        {
            overuse = -remaining;

            if(overuse > BUDGET_NS_THROTTLE_THREASHOLD)
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

static enum hrtimer_restart budget_ns_BET_callback(struct hrtimer *timer)
{
    struct pbs_budget_ns_struct *budget_ns_struct_p;
    s64 remaining;
    ktime_t remaining_k;

    enum hrtimer_restart ret = HRTIMER_NORESTART;

    if(allocator_state != ALLOCATOR_LOOP)
    {
        return ret;
    }

    budget_ns_struct_p = container_of(  timer, 
                                        struct pbs_budget_ns_struct, 
                                        budget_enforcement_timer);

    remaining = check_budget_ns_remaining(budget_ns_struct_p);

    if(remaining > 0)
    {
        //forward the timer by the necessary amount
        remaining = remaining - (BUDGET_NS_THROTTLE_THREASHOLD/2);
        remaining_k.tv64 = remaining;
        hrtimer_forward_now(timer, remaining_k);
        ret = HRTIMER_RESTART;
    }

    return ret;
}

void pbs_budget_ns_BET_start(   struct pbs_budget_struct *budget_struct_p)
{
    struct pbs_budget_ns_struct *budget_ns_struct_p;
    
    s64 remaining;
    ktime_t remaining_k;

    budget_ns_struct_p = &(budget_struct_p->budget_ns_struct);

    //check the time that this should be setup for
    remaining = check_budget_ns_remaining(budget_ns_struct_p);

    if(remaining > 0)
    {
        //setup the timer to go off after the necessary time
        remaining = remaining - (BUDGET_NS_THROTTLE_THREASHOLD/2);
        remaining_k.tv64 = remaining;
        hrtimer_start(  &(budget_ns_struct_p->budget_enforcement_timer), 
                        remaining_k, 
                        HRTIMER_MODE_REL);
    }
}


void pbs_budget_ns_schedin( struct pbs_budget_struct *budget_struct_p,
                            u64 now)
{
    struct pbs_budget_ns_struct *budget_ns_struct_p;
    
    budget_ns_struct_p = &(budget_struct_p->budget_ns_struct);
    
    /*  set the start-time variable to the current time */
    budget_ns_struct_p->last_actv_time = now;
    budget_ns_struct_p->jb_actv_time   = now;
    budget_ns_struct_p->jb_actv_time2  = now;
}


void pbs_budget_ns_schedout(struct pbs_budget_struct *budget_struct_p,
                            u64 now)
{
    struct pbs_budget_ns_struct *budget_ns_struct_p;
    
    s64 current_runtime;
    s64 current_runtime2;
    s64 budget_used;

    budget_ns_struct_p = &(budget_struct_p->budget_ns_struct);

    /*  compute the job runtime in this activation for the job */
    current_runtime =   now - budget_ns_struct_p->jb_actv_time;
    current_runtime2=   now - budget_ns_struct_p->jb_actv_time2;
    
    /*  compute the budget used in this activation (maybe more than job runtime, 
        because of job transition) */
    budget_used     =   now - budget_ns_struct_p->last_actv_time;

    /*  set the total job runtime to the sum of the curent job runtime 
        and previous accumulated job runtime */
    budget_ns_struct_p->total_jb_runtime    += current_runtime;
    budget_ns_struct_p->total_jb_runtime2   += current_runtime2;

    /*  set the total sp runtime to the sum of the runtime in this activation
        and previous accumulated sp runtimes */
    budget_ns_struct_p->total_rp_runtime    += budget_used;
}

void pbs_budget_ns_BET_cancel(  struct pbs_budget_struct *budget_struct_p)
{
    struct pbs_budget_ns_struct *budget_ns_struct_p;
    
    budget_ns_struct_p = &(budget_struct_p->budget_ns_struct);

    //cancel the bandwidth enforcement timer
    hrtimer_cancel(&(budget_ns_struct_p->budget_enforcement_timer));
}

void pbs_budget_ns_init(struct pbs_budget_struct *budget_struct_p)
{
    struct pbs_budget_ns_struct *budget_ns_struct_p;
    
    budget_ns_struct_p = &(budget_struct_p->budget_ns_struct);

    /*Initialize the budget_enforcement_timer including the callback function*/
    hrtimer_init(   &(budget_ns_struct_p->budget_enforcement_timer), 
                    CLOCK_MONOTONIC, 
                    HRTIMER_MODE_REL);
                    
    (budget_ns_struct_p->budget_enforcement_timer).function = budget_ns_BET_callback;
}

void pbs_budget_ns_uninit(  struct pbs_budget_struct *budget_struct_p)
{
    struct pbs_budget_ns_struct *budget_ns_struct_p;
    
    budget_ns_struct_p = &(budget_struct_p->budget_ns_struct);
    
    /*Cancel the budget_enforcement_timer*/
    hrtimer_cancel(&(budget_ns_struct_p->budget_enforcement_timer));
}

