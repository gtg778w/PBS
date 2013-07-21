
#include "LAMbS_mo.h"
#include "LAMbS_VIC.h"
#include "LAMbS_models.h"
#include "LAMbS_VICtimer.h"

s64 LAMbS_VICtimer_threshold = 3000;

/*The threshold should be configured based on timer resolution*/
#define LAMbS_VICTIMER_THRESHOLD (LAMbS_VICtimer_threshold)

#define LAMbS_VICTIMER_CALLBACK_LIMIT (10)

struct list_head LAMbS_VICtimer_activelist = LIST_HEAD_INIT(LAMbS_VICtimer_activelist);

/*The following function checks if the ns_to_target for the given VIC timer is within
LAMbS_VICTIMER_THRESHOLD and calls the callback function.

If the callback function returns LAMbS_VICTIMER_RESTART, the ns_to_target is recomputed
and checked, and the callback function called again if needed. This iterative call 
maybe repeated up to LAMbS_VICTIMER_CALLBACK_LIMIT times. Then, the VICtimer is disabled 
and removed to prevent the CPU from being stuck in this tight loop. The state of a 
misbehaving timer is set to LAMbS_VICTIMER_CALLBACK_STORM. It should no longer be
possible to start the timer without manually changing the state.

It is expected that the following function is called with interrupts disabled
*/

static enum LAMbS_VICtimer_restart 
        LAMbS_VICtimer_check_callback(  LAMbS_VICtimer_t *LAMbS_VICtimer_p,
                                        s64 *ns_to_target_p)
{
    enum LAMbS_VICtimer_restart ret;
    enum LAMbS_VICtimer_restart callback_ret;

    s64 target_VIC;    
    s64 current_VIC;
    s64 current_VIC_timestamp;
    
    s64 VIC_to_target;
    s64 ns_to_target;

    int callback_count;

    /*Get the target VIC*/
    target_VIC  = LAMbS_VICtimer_p->target_VIC;
        
    /*Get the current VIC value*/
    current_VIC = LAMbS_VIC_get(&current_VIC_timestamp);
        
    /*Compute the VIC remaining to target VIC*/
    VIC_to_target = target_VIC - current_VIC;
        
    /*Compute the ns remaining to target*/
    ns_to_target  = LAMBS_models_multiply_shift(VIC_to_target, 
                                                LAMbS_current_instretirementrate_inv, 
                                                LAMbS_MODELS_FIXEDPOINT_SHIFT);
        
    /*Check if the timer is within the threshold of the 
    target or overshot it*/
    if( ns_to_target < LAMbS_VICTIMER_THRESHOLD )
    {
        /*Set the callback_count to 0*/
        callback_count = 0;
        
        do
        {
            /*Check if the callback function has already been called back too many 
            times*/
            callback_count++;
            if(callback_count > LAMbS_VICTIMER_CALLBACK_LIMIT)
            {
                /*This timer is misbehaving. Disable it.*/
                
                /*Remove it from the actv list*/
                list_del(&(LAMbS_VICtimer_p->activelist_entry));
                
                /*Set the state to CALLBACK_STORM*/
                LAMbS_VICtimer_p->state = LAMbS_VICTIMER_CALLBACK_STORM;
                
                /*Set the return value to no restart*/
                ret = LAMbS_VICTIMER_NORESTART;
                
                /*Break out of the loop*/
                break;
            }
            

            /*Change the state of the timer to CALLBACK*/
            LAMbS_VICtimer_p->state = LAMbS_VICTIMER_CALLBACK;
            
            /*Call the callback function*/
            callback_ret = LAMbS_VICtimer_p->function(LAMbS_VICtimer_p, current_VIC);
            
            /*Check if the callback function requested a restart*/
            if(LAMbS_VICTIMER_RESTART == callback_ret)
            {
                /*Get the new VIC target*/
                target_VIC  = LAMbS_VICtimer_p->target_VIC;
                
                /*Get the current VIC value*/
                current_VIC = LAMbS_VIC_get(&current_VIC_timestamp);
                
                /*Compute the VIC remaining to target VIC*/
                VIC_to_target = target_VIC - current_VIC;

                /*Compute the ns remaining to target*/
                ns_to_target  =  
                    LAMBS_models_multiply_shift(VIC_to_target, 
                                                LAMbS_current_instretirementrate_inv, 
                                                LAMbS_MODELS_FIXEDPOINT_SHIFT);
                
                /*Change the state of the timer back to HRTARMED*/
                LAMbS_VICtimer_p->state = LAMbS_VICTIMER_HRTARMED;

                /*The timer has to restart*/
                ret = LAMbS_VICTIMER_RESTART;
            }
            else
            {
                /*The timer will not restart*/
                ret = LAMbS_VICTIMER_NORESTART;
                
                /*Change the state of the timer to INACTIVE*/
                LAMbS_VICtimer_p->state = LAMbS_VICTIMER_INACTIVE;
                
                /*Remove the timer from the active list*/
                list_del(&(LAMbS_VICtimer_p->activelist_entry));
                
                /*Break out of the loop*/
                break;
            }
            
        }while( ( ns_to_target <    LAMbS_VICTIMER_THRESHOLD) );
    }
    else
    {
        /*The timer has not yet reached its target*/
    
        /*The timer has to restart*/
        ret = LAMbS_VICTIMER_RESTART;
        
        /*The timer should remain in the active list*/
        
        /*The state of the timer should remain ACTIVE*/
    }
    
    *ns_to_target_p = ns_to_target;
    return ret;
}

/*
Note that timer callbacks are called with interrupts disabled
There should be no problem with disabling the irqs again
*/
static enum hrtimer_restart LAMbS_VICtimer_hrtcallback(struct hrtimer *timer)
{
    enum hrtimer_restart ret;
    enum LAMbS_VICtimer_restart callback_ret;
    unsigned long irq_flags;
    
    LAMbS_VICtimer_t *LAMbS_VICtimer_p;

    s64 ns_to_target;
    ktime_t ktime_to_target;
    
    /*Enter critical section. Note, timer callback functions should diable interrupts
    anyway, but just in case*/
    local_irq_save(irq_flags);
    
        /*Get the pointer of the VICtimer structure and the traget VIC */
        LAMbS_VICtimer_p = container_of(timer, struct LAMbS_VICtimer_s, hrtimer);
        
        /*Check the status of the timer and take appropriate action*/
        callback_ret = 
            LAMbS_VICtimer_check_callback(  LAMbS_VICtimer_p,
                                            &ns_to_target);
    
        /*If the timer has to be rearmed*/
        if( LAMbS_VICTIMER_RESTART == callback_ret )
        {
            /*Reset the timer with the new relative target ns_to_target*/
            ns_to_target = ns_to_target - (LAMbS_VICTIMER_THRESHOLD/2);
            ktime_to_target.tv64 = ns_to_target;
            hrtimer_forward_now(timer, ktime_to_target);
            ret = HRTIMER_RESTART;
        }
        else
        {
            /*The timer should not be restarted*/
            ret = HRTIMER_NORESTART;
        }

    /*Leave the critical section*/    
    local_irq_restore(irq_flags);
    
    return ret;
}

/*
    Called on transition of the mode of operation

    - The notifying function has a line with "BUG_ON(irqs_disabled());" which implies 
      that this function should never be called with interrupts disabled.

    - This function should not have to call schedule or _schedule. If one of the 
      VIC_timer functions set the NEED_RESCHED flag for the task, the task should be 
      recheduled at least during return from interrupt or syscall, if not earlier.
*/
void LAMbS_VICtimer_motransition(int old_moi, int new_moi)
{
    unsigned long irq_flags;

    s64 instruction_ret_rate_inv;
    
    struct list_head *next_actvlist_node;
    struct list_head *temp_node;
    
    LAMbS_VICtimer_t *LAMbS_VICtimer_p;
    
    enum LAMbS_VICtimer_restart timer_reset;
    s64  ns_to_target;
    ktime_t ktime_to_target;
    
    /*Enter critical section.*/
    local_irq_save(irq_flags);

        /*Get the new instruction retirement rate*/
        instruction_ret_rate_inv = instruction_retirement_rate_inv[new_moi];
        
        /*Loop through the list of active VIC timers and reset their targets
        or call their callbacks as appropriate*/
        list_for_each_safe(next_actvlist_node, temp_node, &LAMbS_VICtimer_activelist)
        {
            /*Get the address of the timer based on the address of the linked list node*/
            LAMbS_VICtimer_p = container_of(    next_actvlist_node, 
                                                struct LAMbS_VICtimer_s, 
                                                activelist_entry);
                                                
            /*Check the status of the timer and take appropriate action*/
            timer_reset = LAMbS_VICtimer_check_callback(LAMbS_VICtimer_p,
                                                        &ns_to_target);
                                        
            /*Reset the timer if necessary*/
            if(LAMbS_VICTIMER_RESTART == timer_reset)
            {
                /*Set the ktime_t value*/
                ktime_to_target.tv64 = ns_to_target;
                /*Start the timer*/
                hrtimer_start(  &(LAMbS_VICtimer_p->hrtimer), 
                                ktime_to_target, 
                                HRTIMER_MODE_REL);
            }
            else
            {
                /*Cancel the timer*/
                hrtimer_cancel(&(LAMbS_VICtimer_p->hrtimer));
            }
        }

    /*Leave the critical section*/
    local_irq_restore(irq_flags);    
}

/* Cancel a VICtimer. This function should not be called from the argument VICtimer's
callback function. The callback function should just return the appropriate return value 
to the callback mechanism to stop the VICtimer.

return      explaination
value:
-1          the VICtimer was in CALLBACK or unknown state.

0:          the VICtimer was in INACTIVE or CALLBACK_STORM state.
    
1:          the VICtimer state was HRTARMED.

*/
int LAMbS_VICtimer_cancel(  LAMbS_VICtimer_t *LAMbS_VICtimer_p)
{
    int ret;
    unsigned long irq_flags;
    
    /*Enter critical section.*/
    local_irq_save(irq_flags);

        switch(LAMbS_VICtimer_p->state)
        {
            case LAMbS_VICTIMER_HRTARMED:
                /*Cancel the hrtimer*/
                hrtimer_cancel(&(LAMbS_VICtimer_p->hrtimer));
                
                /*Change the state of the timer to INACTIVE*/
                LAMbS_VICtimer_p->state = LAMbS_VICTIMER_INACTIVE;
                
                /*Remove the timer from the active list*/
                list_del(&(LAMbS_VICtimer_p->activelist_entry));
                
                ret = 1;
                break;
            
            default:
                /*Unknown state*/
                printk(KERN_INFO "LAMbS_VICtimer_cancel: WARNING LAMbS_VICtimer_cancel "
                                 "called for VICtimer in unknown state (%i)", 
                                 LAMbS_VICtimer_p->state);
                ret = -1;
                break;
                
            case LAMbS_VICTIMER_CALLBACK:
                /*Bad state*/
                printk(KERN_INFO "LAMbS_VICtimer_cancel: WARNING LAMbS_VICtimer_cancel "
                                 "called for VICtimer in CALLBACK state");
                ret = -1;
                break;
            
            case LAMbS_VICTIMER_CALLBACK_STORM:
                /*The VICtimer should already be cancelled so no worries*/
                
            case LAMbS_VICTIMER_INACTIVE:
                ret = 0;
                break;
        }
        
    /*Leave the critical section*/
    local_irq_restore(irq_flags);
    
    return ret;
}


/* Start the VICtimer

return      explaination
value:
-1          The timer has been identified as being misbehaving and will not be started.

0:          Successfully setup the timer.
    
1:          The timer target is (for all practical purposes) in the past.
*/
int LAMbS_VICtimer_start(   LAMbS_VICtimer_t *LAMbS_VICtimer_p,
                            s64 target_VIC,
                            enum LAMbS_VICtimer_mode mode)
{
    int ret = 0;
    
    s64 current_VIC;
    s64 current_VIC_timestamp;
    
    s64 VIC_to_target;
    s64 ns_to_target;
    ktime_t ktime_to_target;
    
    unsigned long irq_flags;
    
    /*Check that this is not a misbehaving timer*/
    if( LAMbS_VICTIMER_CALLBACK_STORM == LAMbS_VICtimer_p->state )
    {
        printk(KERN_INFO "LAMbS_VICtimer_start: attempt to arm a VICtimer in state "
                         "LAMbS_VICTIMER_CALLBACK_STORM");
        ret = -1;
        goto exit0;
    }
    
    /*Enter critical section.*/
    local_irq_save(irq_flags);

        /*Get the current VIC value*/
        current_VIC = LAMbS_VIC_get(&current_VIC_timestamp);
        
        /*Check the mode of the VIC timer*/
        switch(mode)
        {
            case LAMbS_VICTIMER_ABS:
                VIC_to_target = target_VIC - current_VIC;
                break;
            
            case LAMbS_VICTIMER_REL:
                VIC_to_target = target_VIC;
                target_VIC = target_VIC + current_VIC;
                break;
                
            default:
                /*If anything other than ABS or REL is passed as the mode, give a warning
                and assume REL*/
                printk(KERN_INFO    "LAMbS_VICtimer_start: unknown mode (%i) passed as "
                                    "argument. ", mode);
                ret = -1;
                goto exit0;
        }

        /*update the LAMbS_VICtimer_t structure with the target_VIC*/
        LAMbS_VICtimer_p->target_VIC = target_VIC;

        /*Compute the ns remaining to target*/
        ns_to_target  =  
            LAMBS_models_multiply_shift(VIC_to_target, 
                                        LAMbS_current_instretirementrate_inv, 
                                        LAMbS_MODELS_FIXEDPOINT_SHIFT);

        /*Check if the timer is within the threshold of the 
        target or overshot it*/
        if( ns_to_target < LAMbS_VICTIMER_THRESHOLD )
        {

/*FIXME*/
/*{
    printk(KERN_INFO    "LAMbS_VICtimer_start: target_VIC    = %lu",
                        (unsigned long)target_VIC);
    printk(KERN_INFO    "LAMbS_VICtimer_start: VIC_to_target = %lu",
                        (unsigned long)VIC_to_target);
    printk(KERN_INFO    "LAMbS_VICtimer_start: ns_per_int = %lu",
                        (unsigned long)LAMbS_current_instretirementrate_inv);
    printk(KERN_INFO    "LAMbS_VICtimer_start: ns_to_target =  %lu",
                        (unsigned long)ns_to_target);
}*/
            ret = 1;
            goto exit1;
        }
        
        /*Subtract half of the threshold*/
        ns_to_target = ns_to_target - (LAMbS_VICTIMER_THRESHOLD/2);
        
        /*Set the VIC timer to active state and add it to the active queue*/
        list_add(   &(LAMbS_VICtimer_p->activelist_entry), 
                    &(LAMbS_VICtimer_activelist));
        
        /*Start the timer*/
        ktime_to_target.tv64 = ns_to_target;
        hrtimer_start(&(LAMbS_VICtimer_p->hrtimer), ktime_to_target, HRTIMER_MODE_REL);
        
exit1:
    /*Leave the critical section*/
    local_irq_restore(irq_flags);
exit0:
    return ret;
}

/* Initialize a VICtimer structure*/
void LAMbS_VICtimer_init(   LAMbS_VICtimer_t *LAMbS_VICtimer_p)
{
    /* Initialize the hrtimer */
    hrtimer_init(   &(LAMbS_VICtimer_p->hrtimer), 
                    CLOCK_MONOTONIC, 
                    HRTIMER_MODE_REL);
    LAMbS_VICtimer_p->hrtimer.function = LAMbS_VICtimer_hrtcallback;

    /* Initialize the activelist_entry */
    INIT_LIST_HEAD(&(LAMbS_VICtimer_p->activelist_entry));
    
    /* Initialize the state */
    LAMbS_VICtimer_p->state = LAMbS_VICTIMER_INACTIVE;
    
    /* Intialize the remaining entries */
    LAMbS_VICtimer_p->function = NULL;
    LAMbS_VICtimer_p->target_VIC = 0;
}

/*Initialize the VIC timer mechanism. 

The purpose of this function is mainly to callibrate the threshold 
LAMbS_VICTIMER_THRESHOLD*/
int LAMbS_VICtimer_mechanism_init(void)
{
    int ret = 0;
    struct timespec ts;
    
    /*Get timer resolution to compute the threshold value*/
    ret = hrtimer_get_res(CLOCK_MONOTONIC, &ts);
    
    /*Set the threshold to the timer resolution if larger than the default value*/
    LAMbS_VICtimer_threshold =  (ts.tv_nsec > LAMbS_VICtimer_threshold)? 
                                ts.tv_nsec : LAMbS_VICtimer_threshold;

    printk(KERN_INFO "LAMbS_VICtimer_mechanism_init: threshold = %luns",
                     (unsigned long) LAMbS_VICtimer_threshold);

    /*Initialize the active list to be empty*/
    INIT_LIST_HEAD(&LAMbS_VICtimer_activelist);

    return ret;
}

/*Perform VICtimer related clean-up. This mainly checks the VICtimer active list
and cancels all timers on the list, cleaning the list*/
void LAMbS_VICtimer_mechanism_clear(void)
{
    unsigned long irq_flags;

    struct list_head *next_actvlist_node;
    struct list_head *temp_node;
    
    LAMbS_VICtimer_t *LAMbS_VICtimer_p;

    /*Enter critical section.*/
    local_irq_save(irq_flags);
    
        /*Loop through the linked list and deactivate all VICtimers*/
        list_for_each_safe(next_actvlist_node, temp_node, &LAMbS_VICtimer_activelist)
        {
            /*Get the address of the timer based on the address of the linked list node*/
            LAMbS_VICtimer_p = container_of(    next_actvlist_node, 
                                                struct LAMbS_VICtimer_s, 
                                                activelist_entry);
            
            /*Cancel the hrtimer*/
            hrtimer_cancel(&(LAMbS_VICtimer_p->hrtimer));
            
            /*Remove it from the actv list*/
            list_del(&(LAMbS_VICtimer_p->activelist_entry));
            
            /*Set the state to INACTIVE*/
            LAMbS_VICtimer_p->state = LAMbS_VICTIMER_INACTIVE;
            
        }
    
    /*Leave the critical section*/
    local_irq_restore(irq_flags);
    
}

/******************************************************************************
The following code is mainly for testing the VICtimer mechanism for performance 
and bugs 
*******************************************************************************/

#include <linux/slab.h>
/*
    kmalloc
    kfree
*/

static int test_started = 0;
static struct LAMbS_VICtimer_s test_VICtimer;

static int  VICtimer_test_length, VICtimer_test_index;
static u64  VICtimer_test_interval; 
static u64  *VICtimer_test_target_VIC, *VICtimer_test_callback_VIC;

static enum LAMbS_VICtimer_restart 
    LAMbS_VICtimer_test_callback(   struct LAMbS_VICtimer_s* VICtimer_p, 
                                    u64 VIC_current)
{
    enum LAMbS_VICtimer_restart ret;
    u64 next_target;
    
    /*Log the target VIC and callback VIC*/
    VICtimer_test_target_VIC[VICtimer_test_index] = VICtimer_p->target_VIC;
    VICtimer_test_callback_VIC[VICtimer_test_index] =   VIC_current;
    
    /*Set the next target as the current target plus the test interval*/
    
    next_target =   VIC_current + VICtimer_test_interval;
                    
    VICtimer_p->target_VIC = next_target;
    
    /*Increment the test index*/
    VICtimer_test_index++;
    
    /*Only rearm the timer if the test index is less than the test length*/
    if(VICtimer_test_index >= VICtimer_test_length)
    {
        ret = LAMbS_VICTIMER_NORESTART;
    }
    else
    {
        ret = LAMbS_VICTIMER_RESTART;
    }
    
    return ret;
}

int LAMbS_VICtimer_start_test(int test_length, u64 VIC_interval)
{
    int ret;
    
    /*Test that the test has not already started*/
    if(0 != test_started)
    {
        printk(KERN_INFO    "LAMbS_VICtimer_start_test: test is already in progress. "
                            "Call LAMbS_VICtimer_stop_test first.");
        goto error0;
    }
    
    /*Allocate space for the test results*/
    VICtimer_test_target_VIC = kmalloc( (sizeof(u64) * test_length *2), 
                                        GFP_KERNEL);
    if(NULL == VICtimer_test_target_VIC)
    {
        printk(KERN_INFO    "LAMbS_VICtimer_start_test: kmalloc failed for "
                            "VICtimer_test_target_VIC");
        ret = -ENOMEM;
        goto error0;
    }
    
    VICtimer_test_callback_VIC = &(VICtimer_test_target_VIC[test_length]);
    
    /*Initialize the varriables for the experiment*/
    VICtimer_test_length = test_length;
    VICtimer_test_index  = 0;
    
    /*Set the interval length*/
    VICtimer_test_interval  = VIC_interval;
    
    /*Initialize the timer*/
    LAMbS_VICtimer_init( &test_VICtimer);
    test_VICtimer.function = LAMbS_VICtimer_test_callback;
    
    /*Start the timer with a relative target*/
    ret = LAMbS_VICtimer_start( &test_VICtimer,
                                VIC_interval,
                                LAMbS_VICTIMER_REL);
    if(ret < 0)
    {
        printk(KERN_INFO    "LAMbS_VICtimer_start_test: LAMbS_VICtimer_start failed!");
        goto error1;
    }
    else if(ret == 1)
    {
        printk(KERN_INFO    "LAMbS_VICtimer_start_test: The VIC interval is too short. "
                            "LAMbS_VICtimer_start failed!");
        goto error1;
    }
    
    printk(KERN_INFO    "LAMbS_VICtimer_start_test: VIC = %lu",
                        (long unsigned)LAMbS_VIC_get(NULL));
    
    test_started = 1;
    
    return 0;

error1:
    /*Reset the test length to 0*/
    VICtimer_test_length = 0;
    /*Free the memory allocated for the test*/    
    kfree(VICtimer_test_target_VIC);
    /*Set the pointers to NULL*/
    VICtimer_test_target_VIC = NULL;
    VICtimer_test_callback_VIC = NULL;
error0:
    return -1;
}

int LAMbS_VICtimer_stop_test(void)
{
    int ret;
    int i;
    
    s64 error, max_error, min_error;
    
    if(1 != test_started)
    {
        printk(KERN_INFO    "LAMbS_VICtimer_stop_test: the test is currently not in "
                            "progress. Call LAMbS_VICtimer_start_test to start the test");
        return -1;
    }

    /*Cancel the VICtimer*/
    ret = LAMbS_VICtimer_cancel(&test_VICtimer);

    max_error = 0;
    min_error = 0;
    /*Loop through the results and print them*/
    printk(KERN_INFO    "LAMbS_VICtimer_stop_test: "
                        "{target_VIC, callback_VIC, error}[%i] = {",
                        VICtimer_test_index);
    for(i = 0; i < VICtimer_test_index; i++)
    {
        error = VICtimer_test_target_VIC[i] - VICtimer_test_callback_VIC[i];
        max_error = (error > max_error)? error : max_error;
        min_error = (error < min_error)? error : min_error;
        
        printk(KERN_INFO    "\t\t%lu,\t%lu,\t%li", 
                            (unsigned long)VICtimer_test_target_VIC[i],
                            (unsigned long)VICtimer_test_callback_VIC[i],
                            (long)error);
    }
    printk(KERN_INFO "}");
    
    /*Print the maximum and minimum callback_VIC error*/
    printk(KERN_INFO    "LAMbS_VICtimer_stop_test: max_error = %li", 
                        (long)max_error);
    printk(KERN_INFO    "LAMbS_VICtimer_stop_test: min_error = %li", 
                        (long)min_error);
    
    printk(KERN_INFO    "LAMbS_VICtimer_stop_test: VIC = %lu",
                        (long unsigned)LAMbS_VIC_get(NULL));
    
    
    /*Free the memory allocated for the test*/    
    kfree(VICtimer_test_target_VIC);

    /*Reset the test_started flag*/    
    test_started = 0;
    
    return 0;
}

