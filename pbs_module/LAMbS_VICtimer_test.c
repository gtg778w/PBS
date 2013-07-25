/******************************************************************************
The following code is mainly for testing the VICtimer mechanism for performance 
and bugs 
*******************************************************************************/

#include <linux/kernel.h>

#include <linux/slab.h>
/*
    kmalloc
    kfree
*/

#include <linux/sched.h>

#include "LAMbS_VIC.h"
#include "LAMbS_models.h"

#include "LAMbS_VICtimer.h"

static int test_started = 0;
static struct LAMbS_VICtimer_s test_VICtimer;

static int  VICtimer_test_length, VICtimer_test_index;
static u64  VICtimer_test_interval; 
static u64  *VICtimer_test_target_VIC, *VICtimer_test_callback_VIC;
static u64  *VICtimer_test_target_ns,  *VICtimer_test_callback_ns;

static enum LAMbS_VICtimer_restart 
    LAMbS_VICtimer_test_callback(   struct LAMbS_VICtimer_s* VICtimer_p,
                                    u64 VIC_current)
{
    enum LAMbS_VICtimer_restart ret;
    u64 next_target;

    /*Log the target time (ns) and callback time (ns)*/        
    VICtimer_test_target_ns[VICtimer_test_index] =  
                                    (hrtimer_get_expires(&(VICtimer_p->hrtimer))).tv64;
    VICtimer_test_callback_ns[VICtimer_test_index] = 
                                    (hrtimer_cb_get_time(&(VICtimer_p->hrtimer))).tv64;
    
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
    VICtimer_test_target_VIC = kmalloc( (sizeof(u64) * test_length *4), 
                                        GFP_KERNEL);
    if(NULL == VICtimer_test_target_VIC)
    {
        printk(KERN_INFO    "LAMbS_VICtimer_start_test: kmalloc failed for "
                            "VICtimer_test_target_VIC");
        ret = -ENOMEM;
        goto error0;
    }
    VICtimer_test_callback_VIC =    &(VICtimer_test_target_VIC[test_length]);
    VICtimer_test_target_ns =       &(VICtimer_test_callback_VIC[test_length]);
    VICtimer_test_callback_ns =     &(VICtimer_test_target_ns[test_length]);
    
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
    
    s64 VIC_error, max_VIC_error, min_VIC_error;
    s64 ns_error,  max_ns_error,  min_ns_error;
    
    if(1 != test_started)
    {
        printk(KERN_INFO    "LAMbS_VICtimer_stop_test: the test is currently not in "
                            "progress. Call LAMbS_VICtimer_start_test to start the test");
        return -1;
    }

    /*Cancel the VICtimer*/
    ret = LAMbS_VICtimer_cancel(&test_VICtimer);

    max_VIC_error = 0;
    min_VIC_error = 0;
    max_ns_error = 0;
    min_ns_error = 0;
    
    /*Loop through the results and print them*/
    printk(KERN_INFO    "LAMbS_VICtimer_stop_test: "
                        "{target_VIC, callback_VIC, VIC_error}[%i] = {",
                        VICtimer_test_index);
    for(i = 0; i < VICtimer_test_index; i++)
    {
        VIC_error = VICtimer_test_target_VIC[i] - VICtimer_test_callback_VIC[i];
        max_VIC_error = (VIC_error > max_VIC_error)? VIC_error : max_VIC_error;
        min_VIC_error = (VIC_error < min_VIC_error)? VIC_error : min_VIC_error;
        
        ns_error = VICtimer_test_target_ns[i] - VICtimer_test_callback_ns[i];
        max_ns_error = (ns_error > max_ns_error)? ns_error : max_ns_error;
        min_ns_error = (ns_error < min_ns_error)? ns_error : min_ns_error;
        
        printk(KERN_INFO    "\t\t%lu,\t%lu,\t%li,\t%luns,\t%luns,\t%lins", 
                            (unsigned long)VICtimer_test_target_VIC[i],
                            (unsigned long)VICtimer_test_callback_VIC[i],
                            (long)VIC_error,
                            (unsigned long)VICtimer_test_target_ns[i],
                            (unsigned long)VICtimer_test_callback_ns[i],
                            (long)ns_error);
    }
    printk(KERN_INFO "}");
    
    /*Print the maximum and minimum callback_VIC error*/
    printk(KERN_INFO    "LAMbS_VICtimer_stop_test: max_VIC_error = %li", 
                        (long)max_VIC_error);
    printk(KERN_INFO    "LAMbS_VICtimer_stop_test: min_VIC_error = %li", 
                        (long)min_VIC_error);
    /*Print the maximum and minimum callback_ns error*/
    printk(KERN_INFO    "LAMbS_VICtimer_stop_test: max_ns_error = %li", 
                        (long)max_ns_error);
    printk(KERN_INFO    "LAMbS_VICtimer_stop_test: min_ns_error = %li", 
                        (long)min_ns_error);
                        
    printk(KERN_INFO    "LAMbS_VICtimer_stop_test: VIC = %lu",
                        (long unsigned)LAMbS_VIC_get(NULL));
    
    
    /*Free the memory allocated for the test*/    
    kfree(VICtimer_test_target_VIC);

    /*Reset the test_started flag*/    
    test_started = 0;
    
    return 0;
}

