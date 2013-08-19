/*
General note: VIC timer mechanism is more accurate with shorter relative VIC targets.
*/

#include <linux/module.h>
/*
EXPORT_SYMBOL
*/

#include "LAMbS_mo.h"
#include "LAMbS_VIC.h"
#include "LAMbS_models.h"
#include "LAMbS_VICtimer.h"

/*The threshold should be configured based on timer resolution*/
s64 LAMbS_VICtimer_threshold_ns = 3000;

/*
- The following threshold is just the ns threshold multiplied by the instruction 
retirement rate (initially assuming the rate is one)
- A timer with a relative deadline less than this threshold will not be started, and a 
callback function may be called back early by this threshold amount
*/
s64 LAMbS_VICtimer_threshold_VIC;

#define LAMbS_VICTIMER_CALLBACK_LIMIT (10)

struct list_head LAMbS_VICtimer_activelist = LIST_HEAD_INIT(LAMbS_VICtimer_activelist);

/*
It is expected that the following function is called with interrupts disabled

The following function checks if the VIC_to_target for the given VIC timer is within
the VIC threshold and calls the callback function in the CALLBACK state. If not, the 
ns_to_target is computed and the RESTART value is returned.

If the callback function returns LAMbS_VICTIMER_RESTART, and the timer is not in a 
CALLBACK_STORM state, the VIC_to_target is recomputed and checked, and the callback 
function called again if needed. 

This iterative call maybe repeated up to LAMbS_VICTIMER_CALLBACK_LIMIT times. 
Then, the VICtimer state is set to CALLBACK_STORM and the callback function called one
last time.
 
If the callback function return NORESTART or returns RESTART in the CALLBACK_STORM state,
the timer is removed from the active list, and the NORESTART value is returned. 

The CALLBACK_STORM state and related behavior exists to prevent the system from locking 
up due to persistent short VICtimer callback requests. Since this function should be 
called with interrupts disabled, entering an infinite loop in this function would lock up
the system.

Once a VICtimer is in the CALLBACK_STORM state, the function can no longer be started
until the state is explicitly changed.
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
    
    int callback_count;

    /*Initialize the callback_count*/
    callback_count = 0;

    /*Check if the VICtimer is armed. If not, remove it from the active list.*/
    if(LAMbS_VICTIMER_HRTARMED != LAMbS_VICtimer_p->state)
    {
        /*If it is not armed, remove it from the actv list and return*/
        list_del(&(LAMbS_VICtimer_p->activelist_entry));
        ret =  LAMbS_VICTIMER_NORESTART;
        goto exit0;
    }
    
    do
    {
        /*Compute the VIC remaining to target VIC*/
        target_VIC  = LAMbS_VICtimer_p->target_VIC;
        current_VIC = LAMbS_VIC_get(&current_VIC_timestamp);
        VIC_to_target = target_VIC - current_VIC;

        /*If the target VIC is still too far away, then restart the hrtimer with the
        new target*/
        if( (VIC_to_target > LAMbS_VICtimer_threshold_VIC) )
        {
            /*The timer should be restarted no matter what.*/
            ret = LAMbS_VICTIMER_RESTART;
            
            /*Check the instruction retirement rate. If it is zero, the VIC should not 
            increment  until a change in the current mode of operation. When the mode of
            operation changes, the function "LAMbS_VICtimer_motransition" will be called, 
            which will call this check_callback function for all active VICtimers in the
            active list including the current one.*/
            if(0 == LAMbS_current_instretirementrate)
            {
                *ns_to_target_p  =  -1;
            }
            else
            {
                /*Compute the ns remaining to target*/
                *ns_to_target_p  =  
                    LAMBS_models_multiply_shift(VIC_to_target,
                                                LAMbS_current_instretirementrate_inv,
                                                LAMbS_MODELS_FIXEDPOINT_SHIFT);
            }
            
            break;
        }

        /*Increment the callback counter*/
        callback_count++;
        
        /*If the callback function has been called back more than threshold number of 
        times, set the state to CALLBACK_STORM. The callback function is called at least 
        once in the CALLBACK_STORM state. Unless the callback function changes the state 
        back to ACTIVE, this will be the last iteration, and the timer will be 
        deactivated*/
        LAMbS_VICtimer_p->state =   (callback_count >= LAMbS_VICTIMER_CALLBACK_LIMIT)?
                                    LAMbS_VICTIMER_CALLBACK_STORM :
                                    LAMbS_VICTIMER_CALLBACK;
        
        /*Call the callback function*/
        callback_ret = LAMbS_VICtimer_p->function(LAMbS_VICtimer_p, current_VIC);
        if(LAMbS_VICTIMER_RESTART == callback_ret)
        {
            /*Check if the timer is in a callback storm state*/
            if(LAMbS_VICTIMER_CALLBACK_STORM == LAMbS_VICtimer_p->state)
            {
                /*Remove the timer from the active list*/
                list_del(&(LAMbS_VICtimer_p->activelist_entry));
                
                /*The state should remain CALLBACK_STORM*/
                
                /*Set the return value to NORESTART*/
                ret = LAMbS_VICTIMER_NORESTART;
                
                /*Exit the loop*/
                break;
            }
            else
            {
                /*The timer should remain in the active list*/
                
                /*The state should be changed back to ACTIVE*/
                LAMbS_VICtimer_p->state = LAMbS_VICTIMER_HRTARMED;
                
                /*The RESTART return value of the callback function should allow the 
                loop to continue*/
            }
        }
        else
        {
            /*Remove the timer from the active list*/
            list_del(&(LAMbS_VICtimer_p->activelist_entry));
            
            /*Set the state to inactive*/
            LAMbS_VICtimer_p->state = LAMbS_VICTIMER_INACTIVE;
            
            /*Set the return value to NORESTART*/
            ret = LAMbS_VICTIMER_NORESTART;
            
            /*The NORESTART return value of the callback function should cause 
            an exit from the loop*/
        }
        
    }while(LAMbS_VICTIMER_RESTART == callback_ret);
    
exit0:
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

    s64 ns_to_target, ns_to_soft_target;

    s64 ns_current, ns_soft_target;

    /*Enter critical section. Note, timer callback functions should diable interrupts
    anyway, but just in case*/
    local_irq_save(irq_flags);

        /**/
    
        /*Get the pointer of the VICtimer structure and the traget VIC */
        LAMbS_VICtimer_p = container_of(timer, struct LAMbS_VICtimer_s, hrtimer);
        
        /*Check the status of the timer and take appropriate action*/
        callback_ret = 
            LAMbS_VICtimer_check_callback(  LAMbS_VICtimer_p,
                                            &ns_to_target);
    
        /*  If the timer has to be rearmed. Also check that ns_to_target is nonnegative.
            A negative ns_to_target implies that the check_callback function detected
            a zero instruction retirement rate. With a zero instruction retirement rate 
            the VIC should not change until a change in the mode of operation. When the
            MO changes, the hrtimers of VICtimers will be rearmed as appropriate.*/
        if( (LAMbS_VICTIMER_RESTART == callback_ret) && (ns_to_target > 0) )
        {
            ns_current = (timer->base->get_time()).tv64;
            
            /*Compute the time to soft target and the time to target minus half of the 
            threshold*/
            ns_to_soft_target = ns_to_target - (LAMbS_VICtimer_threshold_ns/2);
            /*Compute the soft target as the current time plus the soft time to target*/
            ns_soft_target = ns_current + ns_to_soft_target;
            
            hrtimer_set_expires_range(  timer, 
                                        (ktime_t){.tv64=ns_soft_target}, 
                                        (ktime_t){.tv64=(LAMbS_VICtimer_threshold_ns/2)});
            ret = HRTIMER_RESTART;            
        }
        else
        {
            /*LAMbS_VICtimer_check_callback deletes the VICtimer from the active list if
            necessary. Nothing additional needs to be done here*/
            
            /*The timer should not be restarted*/
            ret = HRTIMER_NORESTART;
        }

    /*Leave the critical section*/    
    local_irq_restore(irq_flags);
    
    return ret;
}

/*
    Called when the current estimated instruction retirement rate changes either due to
    a change in the mode of operation or a change in the estimated retirement rate in a 
    given mode

    - This function should not have to call schedule or _schedule. If one of the 
      VIC_timer functions set the NEED_RESCHED flag for the task, the task would be 
      recheduled at least during return from interrupt or syscall, if not earlier 
      (inshallah).
      
    - This function is called from _LAMbS_models_motrans_callback, which is hopefully 
      called with interrupts disabled (inshallah).
      
    Forgive me if the inshallas offend you, but I'm hitting a little in the dark here
    and I can use all the divine help I can get.
*/
void LAMbS_VICtimer_motransition(void)
{
    struct list_head *next_actvlist_node;
    struct list_head *temp_node;
    
    LAMbS_VICtimer_t *LAMbS_VICtimer_p;
    
    enum LAMbS_VICtimer_restart timer_reset;
    s64  ns_to_target, ns_to_soft_target;
    
    /*Compute the new VIC threshold*/
    LAMbS_VICtimer_threshold_VIC = 
                    LAMBS_models_multiply_shift(LAMbS_VICtimer_threshold_ns,
                                                LAMbS_current_instretirementrate,
                                                LAMbS_MODELS_FIXEDPOINT_SHIFT);
    
    /*Loop through the list of active VIC timers and reset their targets
    or call their callbacks as appropriate*/
    list_for_each_safe(next_actvlist_node, temp_node, &LAMbS_VICtimer_activelist)
    {

        /*Get the address of the timer based on the address of the linked list node*/
        LAMbS_VICtimer_p = container_of(    next_actvlist_node, 
                                            struct LAMbS_VICtimer_s, 
                                            activelist_entry);
                                            
        /*Cancel the timer*/
        hrtimer_cancel(&(LAMbS_VICtimer_p->hrtimer));

        /*Check the status of the timer and take appropriate action*/
        timer_reset = LAMbS_VICtimer_check_callback(LAMbS_VICtimer_p,
                                                    &ns_to_target);
                                    
        /*Reset the timer if necessary. Also check that ns_to_target is greater than 
        zero. Check the comments in the function "LAMbS_VICtimer_hrtcallback" for an 
        explaination.*/
        if((LAMbS_VICTIMER_RESTART == timer_reset) && (ns_to_target > 0))
        {
            /*Compute the soft target.*/
            ns_to_soft_target = ns_to_target - (LAMbS_VICtimer_threshold_ns/2);
            
            /*Start the timer.*/
            hrtimer_start_range_ns( &(LAMbS_VICtimer_p->hrtimer), 
                                    (ktime_t){.tv64=ns_to_soft_target},
                                    (LAMbS_VICtimer_threshold_ns/2), 
                                    HRTIMER_MODE_REL);
        }
    }

    return;
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
                
                /*Remove the timer from the active list*/
                list_del(&(LAMbS_VICtimer_p->activelist_entry));
                
                /*Change the state of the timer to INACTIVE*/
                LAMbS_VICtimer_p->state = LAMbS_VICTIMER_INACTIVE;
                
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
EXPORT_SYMBOL(LAMbS_VICtimer_cancel);

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
    s64 ns_to_target, ns_to_soft_target;
    
    unsigned long irq_flags;
    
    switch(LAMbS_VICtimer_p->state)
    {
        case LAMbS_VICTIMER_HRTARMED:
            LAMbS_VICtimer_cancel(LAMbS_VICtimer_p);
            
        case LAMbS_VICTIMER_INACTIVE:            
            break;
            
        case LAMbS_VICTIMER_CALLBACK:
            /*The VICtimer should not be started from its own callback function*/
            printk(KERN_INFO "LAMbS_VICtimer_start: attempt to arm a VICtimer in state "
                            "LAMbS_VICTIMER_CALLBACK");
            ret = -1;
            goto exit0;
            
        case LAMbS_VICTIMER_CALLBACK_STORM:
            /*This timer is a misbehaving timer. Refuse to start it.*/
            printk(KERN_INFO "LAMbS_VICtimer_start: attempt to arm a VICtimer in state "
                             "LAMbS_VICTIMER_CALLBACK_STORM");
            ret = -1;
            goto exit0;
            
        default:
            printk(KERN_INFO "LAMbS_VICtimer_start: attempt to arm a VICtimer in unknown "
                             "state ");
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
                goto exit1;
        }

        /*Check if the VIC_to_target is within the threshold of the 
        target or overshot it*/
        if( (VIC_to_target < LAMbS_VICtimer_threshold_VIC) ||
            (0 == LAMbS_current_instretirementrate_inv) )
        {
            ret = 1;
            goto exit1;
        }


        /*Compute the ns remaining to target*/
        ns_to_target  =  
            LAMBS_models_multiply_shift(VIC_to_target, 
                                        LAMbS_current_instretirementrate_inv, 
                                        LAMbS_MODELS_FIXEDPOINT_SHIFT);

        /*FIXME*/
        /*Check if the ns_to_target is within the threshold of the 
        target or overshot it*/
        if( ns_to_target < LAMbS_VICtimer_threshold_ns )
        {
            printk(KERN_INFO    "LAMbS_VICtimer_start VIC_to_target not within "
                                "threshold, but ns_to_target is: "
                                "current_VIC = %li, target_VIC = %li, "
                                "inst_rate_inv = %li, "
                                "VIC_to_target = %li, VIC_threshold = %li, "
                                "ns_to_target = %li, ns_threshold = %li",
                                (long)current_VIC,
                                (long)target_VIC,
                                (long)LAMbS_current_instretirementrate_inv,
                                (long)VIC_to_target, (long)LAMbS_VICtimer_threshold_VIC,
                                (long)ns_to_target, (long)LAMbS_VICtimer_threshold_ns);
            ret = 1;
            goto exit1;
        }
                
        /*update the LAMbS_VICtimer_t structure with the target_VIC*/
        LAMbS_VICtimer_p->target_VIC = target_VIC;
        
        /*Set the state of the timer to active*/
        LAMbS_VICtimer_p->state = LAMbS_VICTIMER_HRTARMED;
        
        /*Set the VIC timer to active state and add it to the active queue*/
        list_add(   &(LAMbS_VICtimer_p->activelist_entry), 
                    &(LAMbS_VICtimer_activelist));
        
        /*Compute the soft target*/
        ns_to_soft_target = ns_to_target - (LAMbS_VICtimer_threshold_ns/2);
        
        /*Start the timer*/
        hrtimer_start_range_ns( &(LAMbS_VICtimer_p->hrtimer), 
                                (ktime_t){.tv64=ns_to_soft_target},
                                (LAMbS_VICtimer_threshold_ns/2), 
                                HRTIMER_MODE_REL);
exit1:
    /*Leave the critical section*/
    local_irq_restore(irq_flags);
exit0:
    return ret;
}
EXPORT_SYMBOL(LAMbS_VICtimer_start);

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
EXPORT_SYMBOL(LAMbS_VICtimer_init);

/*Initialize the VIC timer mechanism. 

The purpose of this function is mainly to callibrate the threshold 
LAMbS_VICtimer_threshold_ns*/
int LAMbS_VICtimer_mechanism_init(void)
{
    int ret = 0;
    struct timespec ts;
    
    /*Get timer resolution to compute the threshold value*/
    ret = hrtimer_get_res(CLOCK_MONOTONIC, &ts);
    
    /*Set the threshold to the timer resolution if larger than the default value*/
    LAMbS_VICtimer_threshold_ns =  (ts.tv_nsec > LAMbS_VICtimer_threshold_ns)? 
                                ts.tv_nsec : LAMbS_VICtimer_threshold_ns;

    LAMbS_VICtimer_threshold_VIC = 
                            LAMBS_models_multiply_shift(LAMbS_VICtimer_threshold_ns,
                                                        LAMbS_current_instretirementrate,
                                                        LAMbS_MODELS_FIXEDPOINT_SHIFT);
                                                        
    printk(KERN_INFO "LAMbS_VICtimer_mechanism_init: threshold = %luns",
                     (unsigned long) LAMbS_VICtimer_threshold_ns);

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

