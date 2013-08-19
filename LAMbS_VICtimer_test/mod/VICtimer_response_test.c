/******************************************************************************
The following code is mainly for testing the VICtimer mechanism for performance 
and bugs 
*******************************************************************************/

#include <linux/module.h>
/*
module_param and other related stuf
*/
#include <linux/kernel.h>

#include <linux/slab.h>
/*
    kmalloc
    kfree
*/

#include <linux/sched.h>
/*
    sched_clock
*/

#include <linux/fs.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <linux/buffer_head.h>
/*stuff related to file io*/

#include "LAMbS_VIC.h"
#include "LAMbS_VICtimer.h"

#include "LAMbS_mo.h"

static struct LAMbS_VICtimer_s test_VICtimer;

static struct LAMbS_motrans_notifier_s test_motrans_notifier;

static unsigned long    test_length;
module_param(test_length, ulong, 0);

static unsigned long    test_interval;
module_param(test_interval, ulong, 0);

static unsigned long    test_index;

static s64  *start_VIC_array, *target_VIC_array, *callback_VIC_array;
static s64  *target_ns_array,  *callback_ns_array, *modechange_array;
static s64  callback_storm_count;
static s64  failed_start_count;

/****************************************************************************************

    The following code sets and resets hooks for functions executed at reservation-period 
    boundaries before and after updates to the estimated instruction retirement rate.
    
****************************************************************************************/

static void _VICtimer_response_test_rp_stop(void);
static void _VICtimer_response_test_rp_start(void);

void VICtimer_response_expt_start(void)
{
    LAMbS_VICtimer_pretrans_callback = _VICtimer_response_test_rp_stop;
    LAMbS_VICtimer_psttrans_callback = _VICtimer_response_test_rp_start;
}

void VICtimer_response_expt_stop(void)
{
    LAMbS_VICtimer_psttrans_callback = NULL;
    LAMbS_VICtimer_pretrans_callback = NULL;
}

/****************************************************************************************

    The following code consists of functions that start the VICtimer, stop the VICtimer,
    and the VICtimer callback function.

****************************************************************************************/

static enum LAMbS_VICtimer_restart 
    VICtimer_response_test_callback(struct LAMbS_VICtimer_s* VICtimer_p,
                                    u64 VIC_current)
{
    enum LAMbS_VICtimer_restart ret;
    s64 next_target;

    /*Log the target time (ns) and callback time (ns)*/        
    target_ns_array[test_index] = (hrtimer_get_expires(&(VICtimer_p->hrtimer))).tv64;
    callback_ns_array[test_index] = (hrtimer_cb_get_time(&(VICtimer_p->hrtimer))).tv64;
    
    /*Log the target VIC and callback VIC*/
    target_VIC_array[test_index] = VICtimer_p->target_VIC;
    callback_VIC_array[test_index] = VIC_current;
    
    /*Increment the test index*/
    test_index++;
    
    /*Only rearm the timer if the test index is less than the test length*/
    if(test_index >= test_length)
    {
        /*Stop the experiment: reset the hooks for reservation-period boundaries*/
        VICtimer_response_expt_stop();
    
        ret = LAMbS_VICTIMER_NORESTART;
    }
    else
    {
        /*Log the start VIC*/
        start_VIC_array[test_index] = VIC_current;
    
        /*Initially set the modechange_array entry to 0*/
        modechange_array[test_index] = 0;
    
        /*Set the next target as the current target plus the test interval*/
        next_target = VIC_current + test_interval;
    
        VICtimer_p->target_VIC = next_target;    
    
        ret = LAMbS_VICTIMER_RESTART;
    }
    
    return ret;
}

/*
Cancel the VICtimer associated with the VIC timer
*/
static void _VICtimer_response_test_rp_stop(void)
{
    /*Cancel the timer if it was acive before*/
    LAMbS_VICtimer_cancel(&test_VICtimer);
}

/*
    Start the test for the next reservation period
*/
static void _VICtimer_response_test_rp_start(void)
{
    int ret;

    s64 start_VIC, start_timestamp;
    s64 target_VIC;

    /*Check that the test_index is less than the number of tests requested*/
    if(test_index >= test_length)
    {
        /*Stop the experiment*/
        VICtimer_response_expt_stop();
        
        return;
    }
    
    /*Check for a callback storm condition*/
    if(LAMbS_VICTIMER_CALLBACK_STORM == test_VICtimer.state)
    {
        test_VICtimer.state = LAMbS_VICTIMER_INACTIVE;
        callback_storm_count++;
    }
    
    /*Get the current VIC*/
    start_VIC = LAMbS_VIC_get(&start_timestamp);
    
    /*Log the current VIC*/
    start_VIC_array[test_index] = start_VIC;

    /*Initially set the modechange_array entry to 0*/
    modechange_array[test_index] = 0;
    
    /*Set the target VIC*/
    target_VIC = start_VIC + test_interval;
    
    /*Start the timer with a relative target of test_interval*/
    ret = LAMbS_VICtimer_start( &test_VICtimer,
                                target_VIC,
                                LAMbS_VICTIMER_ABS);
    if(ret < 0)
    {
        static int flag = 1;
        
        if(1 == flag)
        {
            printk( KERN_INFO    
                    "_VICtimer_response_test_rp_start: LAMbS_VICtimer_start failed!");
            flag = 0;
        }
    }
    else if(ret == 1)
    {
        failed_start_count++;
        printk(KERN_INFO    "_VICtimer_response_test_rp_start: The VIC interval %li "
                            "is too short. Start VIC = %li. Abs VIC = %li. "
                            "LAMbS_VICtimer_start failed!",
                            (long)test_interval,
                            (long)start_VIC,
                            (long)target_VIC);
    }
}

/****************************************************************************************

Allocate and initialize arrays to be used for logging test results.

****************************************************************************************/

int VICtimer_response_test_alloc(void)
{    
    /*Allocate space for the test results*/
    start_VIC_array = kmalloc(  (sizeof(u64) * test_length *6),
                                            GFP_KERNEL);
    if(NULL == start_VIC_array)
    {
        printk(KERN_INFO    "VICtimer_response_test_alloc: kmalloc failed for "
                            "start_VIC_array");
        goto error0;
    }
    target_VIC_array    = &(start_VIC_array[test_length]);
    callback_VIC_array  = &(target_VIC_array[test_length]);
    target_ns_array     = &(callback_VIC_array[test_length]);
    callback_ns_array   = &(target_ns_array[test_length]);
    modechange_array    = &(callback_ns_array[test_length]);
    
    callback_storm_count = 0;
    failed_start_count = 0;
    return 0;
    
error0:
    return -1;
}

/****************************************************************************************

    Write results of test out to log file, including callback errors

****************************************************************************************/

static int _VICtimer_response_test_write_log( void )
{
    int ret = 0;
    
    char *string_buffer;
    int string_len;
    ssize_t string_write_len;
    mm_segment_t oldfs;
    
    struct file* filp = NULL;
    
    loff_t pos = 0;
    
    s64 VIC_error;
    s64 ns_error;

    int i;
    
    /*Print the callback-storm count*/
    printk(KERN_INFO "VICtimer_response_test: %li callback storms", 
                    (long int)callback_storm_count);
    printk(KERN_INFO "VICtimer_response_test: %li failed starts", 
                    (long int)failed_start_count);
    
    /*
        Allocate the string buffer 
        
        Appropriate length:
        2^64 ~= 10^20
        <sign><20 digit number><comma><tab>
        7 pieces of data: VIC_start, VIC_target, VIC_callback, VIC_error, 
                          ns_target, ns_callback, ns_error,
        1 + 24*7 + 1= 170bytes    
    */
    
    /**/
    string_buffer = kmalloc( (sizeof(char) * 256), GFP_KERNEL);
    if(NULL == string_buffer)
    {
        printk("_VICtimer_response_test_write_log: kmalloc failed for string buffer");
        ret = -1;
        goto exit0;
    }

    /*Save the previous address space and extend the current address space*/
    oldfs = get_fs();
    set_fs(get_ds());

    /*Open the standrad log file for VICtimer test*/
    filp = filp_open(   "/var/log/VICtimer_response_test_log.csv", 
                        (O_WRONLY | O_CREAT | O_TRUNC), 
                        (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH));
    if(IS_ERR(filp))
    {
        printk(KERN_INFO "_VICtimer_response_test_write_log: filp_open failed");
        ret = -1;
        goto exit1;
    }

    /*Loop over the samples*/
    for(i = 0; i < test_index; i++)
    {
        /*Compute the error, and max and min errors*/
        VIC_error = target_VIC_array[i] - callback_VIC_array[i];        
        ns_error = target_ns_array[i] - callback_ns_array[i];
        
        /*Create the text to write to file*/
        string_len = snprintf(  string_buffer, 
                                (size_t)256, 
                                "\t%lu,\t%lu,\t%lu,\t%li,\t%lu,\t%lu,\t%li,\t%lu\n",
                                (unsigned long)start_VIC_array[i],
                                (unsigned long)target_VIC_array[i],
                                (unsigned long)callback_VIC_array[i],
                                (long)VIC_error,
                                (unsigned long)target_ns_array[i],
                                (unsigned long)callback_ns_array[i],
                                (long)ns_error,
                                (unsigned long)modechange_array[i]);
        
        string_len = (string_len > 256)? 256 : string_len;
        
        /*Write to file*/
        string_write_len = vfs_write(filp, string_buffer, string_len, &pos);
        if(string_write_len < string_len)
        {
            printk(KERN_INFO "_VICtimer_response_test_write_log: vfs_write failed");
            ret = -1;
            goto exit2;
        }
    }
    
exit2:
    /*Sync the file*/
    vfs_fsync(filp, 0);
    
    /*Close the log file*/
    filp_close(filp, NULL);
exit1:
    /*Restore the original address space*/
    set_fs(oldfs);

    /*Free the string buffer*/
    kfree(string_buffer);
exit0:
    return ret;
}

/****************************************************************************************

    MO transition callback function for detecting when a cheng in MO takes place

****************************************************************************************/

static void _VICtimer_test_motrans_callback( 
                                    struct LAMbS_motrans_notifier_s *motrans_notifier_p,
                                    s32 old_moi, s32 new_moi)
{
    /*Set the modechange_array entry to 0*/
    modechange_array[test_index] = 1;
}

/****************************************************************************************

    Module initialization and cleanup code

****************************************************************************************/


static void VICtimer_response_test_uninit(void)
{
    int ret;
    
    /*Stop the experiment: unhook the start and stop functions*/
    VICtimer_response_expt_stop();
    
    /*unhook the MO notifier*/
    LAMbS_motrans_unregister_notifier(&test_motrans_notifier);
    
    /*Stop the current test if running: Cancel the test VICtimer*/
    _VICtimer_response_test_rp_stop();
    
    /*Write the VICtimer test results to a log file*/
    ret = _VICtimer_response_test_write_log();
    if(ret < 0)
    {
        printk(KERN_INFO    "VICtimer_response_test_uninit: _VICtimer_response_test_write_log"
                            " failed");
    }
    
    /*Free the memory allocated for the test*/    
    kfree(start_VIC_array);
}


static int VICtimer_response_test_init(void)
{
    int ret;

    /*Validate the parameters passed to the module*/
    if(test_interval < 30000)
    {
        printk(KERN_INFO    "VICtimer_response_test_init: The parameters "
                            "test_interval must be greater than 30000 "
                            "virtual instructions.");
        goto error0;
    }
        
    /*Allocate memory to store the results*/
    ret = VICtimer_response_test_alloc();
    if(0 != ret)
    {
        printk(KERN_INFO    "VICtimer_response_test_init: VICtimer_response_test_alloc "
                            "failed!");
        goto error0;
    }

    /*Initialize the varriables for the experiment*/
    test_index  = 0;

    /*Initialize the VICtimer*/
    LAMbS_VICtimer_init( &test_VICtimer);
    /*Set the VICtimer callback function*/
    test_VICtimer.function = VICtimer_response_test_callback;
    
    /*Start the experiment: Set the RP-boundary callback hooks*/
    VICtimer_response_expt_start();
    
    /*Initialize the MOtrans notifier*/
    test_motrans_notifier.callback = _VICtimer_test_motrans_callback;
    
    /*Hook the MOtrans notifier*/
    LAMbS_motrans_register_notifier(&test_motrans_notifier);
    
    /*Start the test for the current RP: start the VICtimer*/
    _VICtimer_response_test_rp_start();
    
    return 0;
error0:
    return -1;
}

module_init(VICtimer_response_test_init);
module_exit(VICtimer_response_test_uninit);

MODULE_LICENSE("GPL");
