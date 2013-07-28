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
/*
    sched_clock
*/

#include <linux/fs.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <linux/buffer_head.h>
/*stuff related to file io*/

#include "LAMbS_VIC.h"
#include "LAMbS_models.h"

#include "LAMbS_VICtimer.h"

static int test_started = 0;
static struct LAMbS_VICtimer_s test_VICtimer;

static int  VICtimer_test_length, VICtimer_test_index;
static u64  VICtimer_test_interval; 
static u64  *VICtimer_test_target_VIC, *VICtimer_test_callback_VIC;
static u64  *VICtimer_test_target_ns,  *VICtimer_test_callback_ns;
static u64  *VICtimer_test_instrateinv;

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
    
    /*Log the inverse instruction retirement rate*/
    VICtimer_test_instrateinv[VICtimer_test_index] = LAMbS_current_instretirementrate_inv;
    
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
    VICtimer_test_target_VIC = kmalloc( (sizeof(u64) * test_length *5),
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
    VICtimer_test_instrateinv =     &(VICtimer_test_callback_ns[test_length]);
    
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


/*
    allocate space for the string buffer of appropriate length
        2^64 ~= 10^20
        <sign><20 digit number><comma><tab>
        7 pieces of data: VIC_target, VIC_callback, VIC_error, ns_target, ns_callback,
        ns_error, inst_retirement_rate
        1 + 24*7 + 1= 170bytes
        
    extend the address space
    
    open the log file /var/log/VICtimer_test_log.csv
    
    loop through the results
    
        print the results as string to the buffer
        
        write the buffer to the file
        
    sync the file
    
    close the file
    
    restore the address space
    
    free the string buffer
    
*/
static int _LAMbS_VICtimer_test_write_log(  s64 *max_VIC_error_p,
                                            s64 *min_VIC_error_p,
                                            s64 *max_ns_error_p,
                                            s64 *min_ns_error_p)
{
    int ret = 0;
    
    char *string_buffer;
    int string_len;
    ssize_t string_write_len;
    mm_segment_t oldfs;
    
    struct file* filp = NULL;
    
    loff_t pos = 0;
    
    s64 VIC_error, max_VIC_error, min_VIC_error;
    s64 ns_error,  max_ns_error,  min_ns_error;

    int i;
    
    /*Allocate the string buffer*/
    string_buffer = kmalloc( (sizeof(char) * 256), GFP_KERNEL);
    if(NULL == string_buffer)
    {
        printk("_LAMbS_VICtimer_test_write_log: kmalloc failed for string buffer");
        ret = -1;
        goto exit0;
    }

    /*Save the previous address space and extend the current address space*/
    oldfs = get_fs();
    set_fs(get_ds());

    /*Open the standrad log file for VICtimer test*/
    filp = filp_open(   "/var/log/VICtimer_test_log.csv", 
                        (O_WRONLY | O_CREAT | O_TRUNC), 
                        (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH));
    if(IS_ERR(filp))
    {
        printk(KERN_INFO "_LAMbS_VICtimer_test_write_log: filp_open failed");
        ret = -1;
        goto exit1;
    }

    /*Initialize the min and max error values*/
    max_VIC_error = 0;
    min_VIC_error = 0;
    max_ns_error = 0;
    min_ns_error = 0;

    /*Loop over the samples*/
    for(i = 0; i < VICtimer_test_index; i++)
    {
        /*Compute the error, and max and min errors*/
        VIC_error = VICtimer_test_target_VIC[i] - VICtimer_test_callback_VIC[i];
        max_VIC_error = (VIC_error > max_VIC_error)? VIC_error : max_VIC_error;
        min_VIC_error = (VIC_error < min_VIC_error)? VIC_error : min_VIC_error;
        
        ns_error = VICtimer_test_target_ns[i] - VICtimer_test_callback_ns[i];
        max_ns_error = (ns_error > max_ns_error)? ns_error : max_ns_error;
        min_ns_error = (ns_error < min_ns_error)? ns_error : min_ns_error;
        
        /*Create the text to write to file*/
        string_len = snprintf(  string_buffer, 
                                (size_t)256, 
                                "\t%lu,\t%lu,\t%li,\t%lu,\t%lu,\t%li,\t0x%lx\n", 
                                (unsigned long)VICtimer_test_target_VIC[i],
                                (unsigned long)VICtimer_test_callback_VIC[i],
                                (long)VIC_error,
                                (unsigned long)VICtimer_test_target_ns[i],
                                (unsigned long)VICtimer_test_callback_ns[i],
                                (long)ns_error,
                                (unsigned long)VICtimer_test_instrateinv[i]);
        
        string_len = (string_len > 256)? 256 : string_len;
        
        /*Write to file*/
        string_write_len = vfs_write(filp, string_buffer, string_len, &pos);
        if(string_write_len < string_len)
        {
            printk(KERN_INFO "_LAMbS_VICtimer_test_write_log: vfs_write failed");
            ret = -1;
            goto exit2;
        }
    }
    
    /*Write out to the output varriables*/
    *max_VIC_error_p = max_VIC_error;
    *min_VIC_error_p = min_VIC_error;
    *max_ns_error_p  = max_ns_error;
    *min_ns_error_p  = min_ns_error;

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

int LAMbS_VICtimer_stop_test(void)
{
    int ret;
    
    s64 max_VIC_error, min_VIC_error;
    s64 max_ns_error,  min_ns_error;
    
    
    if(1 != test_started)
    {
        printk(KERN_INFO    "LAMbS_VICtimer_stop_test: the test is currently not in "
                            "progress. Call LAMbS_VICtimer_start_test to start the test");
        return -1;
    }

    /*Cancel the VICtimer*/
    ret = LAMbS_VICtimer_cancel(&test_VICtimer);
    
    /*Write the VICtimer test results to a log file*/
    ret = _LAMbS_VICtimer_test_write_log(   &max_VIC_error,
                                            &min_VIC_error,
                                            &max_ns_error,
                                            &min_ns_error);
    if(ret < 0)
    {
        printk(KERN_INFO    "LAMbS_VICtimer_stop_test: _LAMbS_VICtimer_test_write_log"
                            " failed");
        goto error0;
    }
    
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
error0:
    return -1;
}

