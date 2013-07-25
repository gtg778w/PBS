#include "pbs_timing.h"
#include "jb_mgt.h"
#include "bw_mgt.h"
#include "pba.h"
/*
    init/uninit the pbs_hrtick_timer
    rt_runtime_update
*/

#include "LAMbS.h"
/*
    measurement-related code for energy, icount, mostat
*/

#include "LAMbS_models.h"
/*
    LAMbS_models_init
*/

#include "LAMbS_VIC.h"
/*
    LAMbS_clock
*/

/**********************************************************************

Global variables and functions associated with the scheduling period timer.

***********************************************************************/

struct hrtimer  sp_timer;
ktime_t     scheduling_period_ns;
s64         allocator_runtime_ns;

ktime_t     expires_prev;
ktime_t     expires_next;
u64         allocator_actv_time;
u64         allocator_stop_time;

/**********************************************************************/

//- Use a sorted linkled list structure for wakeup. (insertion is O(n) in the worst case and wakeup is O(1))
//- need wakeup check to be faster than sleep
struct list_head    timing_queue_head;

struct task_struct  *allocator;

int allocator_overrun_count;
int allocator_boundaryrun_count;             
extern loaddata_list_header_t   *loaddata_list_header;

/**********************************************************************/
//high resolution timer callbacks

enum hrtimer_restart sp_timer_func(struct hrtimer *fired_timer)
{
    ktime_t now, temp;
    int overrun;

    now = hrtimer_cb_get_time(fired_timer);
    temp = hrtimer_get_expires(fired_timer);
    overrun = hrtimer_forward(fired_timer, now, scheduling_period_ns);

    if(overrun)
    {
        expires_prev = temp;
        expires_next = hrtimer_get_expires(fired_timer);

        //Wakeup the allocator task
        if(wake_up_process(allocator) == 0)
        {
            static unsigned char flag = 1;

            if(flag)
            {
                flag = 0;
                printk(KERN_INFO "WARNING: allocator detected running at "
                        "scheduling period boundary!");
            }

            //Task already running
            allocator_boundaryrun_count++;
        }

        //record the activation time for the allocator
        allocator_actv_time = LAMbS_clock();

    }

    return HRTIMER_RESTART;
}

void allocator_schedin(   struct preempt_notifier *notifier, 
                          int cpu)
{

}

void allocator_schedout(    struct preempt_notifier *notifier, 
                            struct task_struct *next)
{
    s64 max_runtime, last_runtime;

    allocator_stop_time = LAMbS_clock();

    //log the run_time for the allocator
    last_runtime= allocator_stop_time - allocator_actv_time;
    max_runtime = loaddata_list_header->max_allocator_runtime;
    max_runtime = (last_runtime > max_runtime)? last_runtime : max_runtime;
    loaddata_list_header->prev_sp_boundary  = expires_prev.tv64;
    loaddata_list_header->last_allocator_runtime    = last_runtime;
    loaddata_list_header->max_allocator_runtime = max_runtime;

    if(last_runtime > allocator_runtime_ns)
    {
        //Task already running
        allocator_overrun_count++;
    }
}

static struct preempt_ops allocator_pops = {
                            .sched_in = allocator_schedin,
                            .sched_out= allocator_schedout,
                         };
struct preempt_notifier allocator_preempt_notifier;


#define insert_asnd(list, prev, insertible, next)       \
do                                                      \
{                                                       \
                                                        \
    for(next = prev->next; next != (list); prev = next, next = next->next)  \
    {                                                   \
        if( (((struct SRT_timing_struct*)next)->next_task_period_boundary.tv64) > ((insertible)->next_task_period_boundary.tv64)) \
            break;                                      \
    }                                                   \
                                                        \
    __list_add(&((insertible)->timing_list_entry), prev, next); \
                                                        \
}while(0)

void first_sched_period_tick(void)
{
    /*Take initial readings of absolute instruction count, energy, and time spent so far
    in each mode of operation and initialize the model coefficients*/
    LAMbS_models_measurements_init();
}

void sched_period_tick(void)
{
    struct list_head *pos, *prev, *next;
    struct SRT_timing_struct* wakeable;
    struct SRT_struct *bwrefreshable;

    int sp_till_deadline, sp_per_tp;

    /*Determine the number of instruction retired, energy consumed, and time spent in 
    each mode of operation over the previous reservation period*/
    LAMbS_measure_delta(&(loaddata_list_header->icount_last_sp),
                        &(loaddata_list_header->energy_last_sp),
                        loaddata_list_header->mostat_last_sp);
                            
    /*Accumulate the total amount of energy consumed*/
    loaddata_list_header->energy_total += loaddata_list_header->energy_last_sp;

    //extract all tasks to be woken
    if(timing_queue_head.next == &timing_queue_head)
        return;

    pos = timing_queue_head.next;
    while(1)
    {
        wakeable = (struct SRT_timing_struct*)pos;
        bwrefreshable = (struct SRT_struct*)pos;

        if(expires_prev.tv64 < wakeable->next_task_period_boundary.tv64)
            break;

        //remove it from the list
        pos = pos->prev;
        __list_del(pos, (wakeable->timing_list_entry).next);

        //assign it a new value
        wakeable->next_task_period_boundary.tv64 += wakeable->task_period.tv64;

        //insert it back into the list in order
        prev = &timing_queue_head;
        insert_asnd(&timing_queue_head, prev, wakeable, next);

        //increment the task's queue length
        (bwrefreshable->queue_length)++;
        (bwrefreshable->loaddata->queue_length) = 
            (unsigned short)(bwrefreshable->queue_length);

        //wakeup the associated task
        wake_up_process(bwrefreshable->task);

        //move to the next task in the list
        pos = pos->next;
    }

    //refresh bandwidth for the tasks
    pos = timing_queue_head.next;
    while(pos != &timing_queue_head)
    {
        //determine the next task in the list
        bwrefreshable = (struct SRT_struct*)pos;

        //decrement sp_till_deadline
        sp_till_deadline = (bwrefreshable->loaddata)->sp_till_deadline;
        sp_per_tp = (bwrefreshable->loaddata)->sp_per_tp;
        sp_till_deadline = (sp_till_deadline == 1) ? 
                            sp_per_tp : (sp_till_deadline - 1);
        (bwrefreshable->loaddata)->sp_till_deadline = sp_till_deadline;

        pba_refresh_budget(bwrefreshable);

        //move to the next task in the list
        pos = pos->next;
    }

}

void assign_bandwidths(void)
{
    struct list_head *pos;
    struct SRT_struct *SRT_struct;
    ktime_t now;
    u64 bw_sum, allocation;
    s64 saturation_level;
    unsigned char saturate;

    //check if the allocations have to be saturated 
    //(the sum of the bw is larger than some threshold)
    bw_sum = 0;
    pos = timing_queue_head.next;
    while(pos != &timing_queue_head)
    {
        //determine the next task in the list
        SRT_struct = (struct SRT_struct*)pos;

        bw_sum = bw_sum + allocation_array[SRT_struct->allocation_index];

        //accumulate the total budget allocated before saturation
        SRT_struct->summary.cumulative_budget = 
            SRT_struct->summary.cumulative_budget + 
            allocation_array[SRT_struct->allocation_index];

        //move to the next task in the list
        pos = pos->next;
    }

    //compute the appropriate saturation level according to time remaining
    //until the beginning of the next scheduling period
    now = hrtimer_cb_get_time(&sp_timer);
    saturation_level = (expires_next.tv64 - now.tv64)*95/100;

    saturate = (bw_sum > saturation_level)? 1: 0;

    pos = timing_queue_head.next;
    while(pos != &timing_queue_head)
    {
        //determine the next task in the list
        SRT_struct = (struct SRT_struct*)pos;

        //saturate the allocation if necessary
        allocation = allocation_array[SRT_struct->allocation_index];
        if(saturate)
        {
            allocation = (allocation*saturation_level)/bw_sum;
            allocation_array[SRT_struct->allocation_index] = allocation;
        }

        //modify its bandwidth allocation
        pba_set_budget(SRT_struct, allocation);
        
        //accumulate the total budget allocated
        SRT_struct->summary.cumulative_budget_sat = 
            SRT_struct->summary.cumulative_budget_sat + allocation;

        //move to the next task in the list
        pos = pos->next;
    }

//FIXME
    {
        static int i = 0;
        int ret;
        
        if(i < 10)
        {
            i++;
        }
        else if( i == 10)
        {
            ret = LAMbS_VICtimer_start_test(5000, 3000000);
            if(ret == -1)
            {
                printk(KERN_INFO "assign_bandwidths: LAMbS_VICtimer_start_test failed!");
            }
            
            i++;
        }
    }
}


int setup_allocator(s64 period, s64 runtime)
{
    scheduling_period_ns.tv64   = period;
    allocator_runtime_ns        = runtime;
    
    //FIXME
    printk(KERN_INFO "allocator scheduling period: %lli runtime: %lli\n", 
            scheduling_period_ns.tv64, allocator_runtime_ns);

    return 0;
}

//this function should only be called in the context of the allocator task
//this function should be called with preemption disabled once
int start_pbs_timing(void)
{
    ktime_t now, soft, hard;
    s64 delta;

//step1
    allocator = current;

    //initialize the preempt notifier
    preempt_notifier_init(  &allocator_preempt_notifier, 
                            &allocator_pops);
    hlist_add_head( &(allocator_preempt_notifier.link), 
                    &(allocator->preempt_notifiers));

    //beyond this point, start_pbs_timing will return success (0)

//step3
    //initialize the sp_timer
    hrtimer_init(&sp_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    sp_timer.function = sp_timer_func;
    now = hrtimer_cb_get_time(&sp_timer);
    hrtimer_forward(&sp_timer, now, scheduling_period_ns);
    
    //start the sp_timer
    soft = hrtimer_get_softexpires(&sp_timer);
    hard = hrtimer_get_expires(&sp_timer);
    delta = ktime_to_ns(ktime_sub(hard, soft));
    hrtimer_start_range_ns(&sp_timer, soft, delta, HRTIMER_MODE_ABS_PINNED);
    expires_next = hrtimer_get_expires(&sp_timer);

//step4
    //initialize various variables and mechanisms
    allocator_boundaryrun_count = 0;
    allocator_overrun_count = 0;
    INIT_LIST_HEAD(&timing_queue_head);

    return 0;
}

//this function should be called with preemption disabled
//and only when the allocator is in the ALLOCATOR_LOOP state
int stop_pbs_timing(char not_allocator)
{
    int ret = 0;

    struct list_head *pos;
    struct SRT_struct *SRT_struct;

//undo step3
    //cancel the scheduling period timer
    hrtimer_cancel(&sp_timer);

    if(not_allocator)
    {
        //if this was called from a task other than the allocator task,
        //might still need to wakeup the allocator task, since the timer 
        //is no longer active to wake it up
        wake_up_process(allocator);
    }

//nothing to be done for step4

//handle what happens after step 4
    //wake up all the SRT tasks, restoring their bandwidth timers
    pos = timing_queue_head.next;
    while(pos != &timing_queue_head)
    {
        //determine the next task in the list
        SRT_struct = (struct SRT_struct*)pos;

        if(SRT_struct->state == SRT_LOOP)
        {
            ret = wake_up_process(SRT_struct->task);
            if(ret != 1)
            {
                printk(KERN_INFO "wake_up_process returned failure for %d!", 
                        SRT_struct->task->pid);
            }
        }

        //move to the next task in the list
        pos = pos->next;
    }


    //print any allocator related statistics
    if(allocator_boundaryrun_count > 0)
    {
        printk(KERN_INFO "Allocator detected running at scheduling period"
                " boundary %i times!\n", allocator_boundaryrun_count);        
    }

    if(allocator_overrun_count > 0)
    {
        printk(KERN_INFO "Allocator detected exceeding predicted maximum usage"
                " %i times!\n", allocator_overrun_count);
    }
    
    /*FIXME*/
    {
        LAMbS_VICtimer_stop_test();
    } 
    
    return 0;
}

/**********************************************************************/

//This can be called from any task
int sleep_till_next_period(struct SRT_timing_struct *tq_entry)
{
    int ret = 0;
    struct SRT_struct    *ss = (struct SRT_struct*)tq_entry;

    if(allocator_state != ALLOCATOR_LOOP)
    {
        return -ECANCELED;
    }

    //reduce the queue length by one
    (ss->queue_length)--;

    pba_nextjob(ss);

    //update the log with information from the last completed job
    (ss->log).abs_releaseTime   =(ss->loaddata)->job_release_time;
    (ss->log).abs_LFT           = expires_next.tv64; //the latest possible finishing time is the end of this scheduling period

    //set the release-time in loaddata to the release time of the next job.
    (ss->loaddata)->job_release_time +=(ss->timing_struct).task_period.tv64;
    
    //FIXME
    if((ss->queue_length) == 0)
    {
        set_current_state(TASK_INTERRUPTIBLE);
        preempt_enable_no_resched();
        schedule();
        preempt_disable();
    }
    else
    {
        if((ss->queue_length) > 0)
        {
            /*There are more jobs accumulated and so the last job missed its deadline.
            The next job has already been released*/
            (ss->summary.total_misses)++;
        }
        else /*if((ss->queue_length) < 0)*/
        {
            /*Something really screwey is going on with negative queue lengths*/
            static unsigned char flag = 1;

            if(flag == 1)
            {
                flag = 0;
                printk(KERN_INFO "WARNING queue_length of task %d is negative!", current->pid);
            }
        }
    }

    return ret;
}


//This can be called from any task
int first_sleep_till_next_period(struct SRT_timing_struct *tq_entry)
{
    int ret = 0;

    struct list_head    *prev, *next;
    struct SRT_struct   *SRT_struct = (struct SRT_struct*)tq_entry;

    if(allocator_state != ALLOCATOR_LOOP)
    {
        return -ECANCELED;
    }

    pba_init(SRT_struct);

    //from this point forward, first_sleep_till_next_period can only return success (0)
    tq_entry->next_task_period_boundary = expires_next;

    //add the job to the list
    prev = &timing_queue_head;
    insert_asnd(&timing_queue_head, prev, tq_entry, next);

    //correct sp_till_deadline and insert loaddata entry into the loaddata list
    (SRT_struct->loaddata)->sp_till_deadline=1;
    insert_loaddata(SRT_struct->loaddata);
    printk(KERN_INFO "New SRT load data inserted!\n");

    //initialize some additional statistics
    SRT_struct->overuse_count = 0;
    SRT_struct->maximum_overuse = 0;

    //update the statistics for counting job computation times
    pba_firstjob(SRT_struct);

    /*set the release-time in loaddata to the next scheduling period, 
    the release time of the first job.*/
    (SRT_struct->loaddata)->job_release_time = expires_next.tv64;

    //this is the beginning of the job 0
    //the log reflects information on the job before the one currently started (job# -1)
    //job# -1 was "released" one task period before the last scheduling period boundary
    //Update the log to reflect that
    (SRT_struct->log).abs_releaseTime   = expires_prev.tv64 - (SRT_struct->timing_struct).task_period.tv64;
    (SRT_struct->log).abs_LFT           = expires_next.tv64;    

    set_current_state(TASK_INTERRUPTIBLE);
    preempt_enable_no_resched();
    schedule();
    preempt_disable();

    return ret;
}

void remove_from_timing_queue(struct SRT_timing_struct *tq_entry)
{
    struct SRT_struct   *SRT_struct = (struct SRT_struct*)tq_entry;

    //remove it from the timing list
    __list_del( (tq_entry->timing_list_entry).prev, 
                (tq_entry->timing_list_entry).next);
    
    //remove it from the loaddata list
    remove_loaddata(SRT_struct->loaddata);

    pba_uninit(SRT_struct);

    printk(KERN_INFO "Removed load data from list. Task %d.\n", current->pid);
    
}

