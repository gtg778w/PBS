#include <linux/module.h>
#include <linux/kernel.h>

#include <linux/slab.h>
/*
    kmalloc
    kfree
*/

#include <linux/hrtimer.h>
/*
    hrtimer
*/

#include <linux/fs.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <linux/buffer_head.h>
/*stuff related to file io*/

#define HRTIMER_RESPONSE_TEST_THRESHOLD (3000)

static struct hrtimer hrtimer_response_test_timer;

static u64  lcg_state;
static u64  _divider;

static unsigned long  test_index;
static unsigned long  test_length = 5000;
module_param(test_length, ulong, 0);

static u64  interval_base_u64;

static unsigned long interval_base = 1000000;
module_param(interval_base, ulong, 0);

static u64 interval_noisemag_u64;

static unsigned long interval_noisemag = 10000;
module_param(interval_noisemag, ulong, 0);

static s64  *hrtimer_response_test_start_ns = NULL;
static s64  *hrtimer_response_test_target_ns = NULL;
static s64  *hrtimer_response_test_callback_ns = NULL;

static s64 inline _hrtimer_response_test_getNextInterval(void)
{
    s64 interval_noise;
    s64 interval;
    
    /*Generate the next random number
    Wikipedia attributes the following numbers to Donald Knuth*/
    lcg_state = 6364136223846793005 * lcg_state + 1442695040888963407;
    
    /*Get a value between 0 and 2*noisemag*/
    interval_noise = (lcg_state/_divider);
    /*Subtract noisemag to get a value between -noisemag and +noisemag*/
    interval_noise = interval_noise - interval_noisemag_u64;
    
    /*Add the noise value to the base value to get the actual interval*/
    interval = interval_noise + interval_base_u64;
    
    return interval;
}

static enum hrtimer_restart hrtimer_response_test_callback(struct hrtimer *timer)
{
    enum hrtimer_restart ret;
    
    s64 ns_current;
    s64 timer_interval;
    s64 next_expires;
    s64 next_soft_expires;
    
    /*Get the current time*/
    ns_current = (timer->base->get_time()).tv64;

    /*Log various information*/
    hrtimer_response_test_target_ns[test_index] = (hrtimer_get_expires(timer)).tv64;
    hrtimer_response_test_callback_ns[test_index] = ns_current;
    
    /*Get the interval to set for the next timer*/
    timer_interval = _hrtimer_response_test_getNextInterval();
    
    /*Set the absolute target*/
    next_expires = ns_current + timer_interval;
    
    /*Set the soft target*/
    next_soft_expires = next_expires - (HRTIMER_RESPONSE_TEST_THRESHOLD/2);
    
    /*Set the new expiration time for the timer*/
    hrtimer_set_expires_range(  timer, 
                                (ktime_t){.tv64=next_soft_expires}, 
                                (ktime_t){.tv64=(HRTIMER_RESPONSE_TEST_THRESHOLD/2)});
    
    /*Increment the test index*/
    test_index++;
    
    /*Only rearm the timer if the test index is less than the test length*/
    if(test_index >= test_length)
    {
        /*Set the return value to NO_RESTART*/
        ret = HRTIMER_NORESTART;
    }
    else
    {
        /*Log the timer start-time of the next timer callback*/
        hrtimer_response_test_start_ns[test_index] = ns_current;
        
        /*Set the return value to RESTART*/
        ret = HRTIMER_RESTART;
    }
    
    return ret;

}

static int hrtimer_response_test_alloc(void)
{
    /*Allocate space for the test results*/
    hrtimer_response_test_start_ns = kmalloc(  (sizeof(u64) * test_length *3),
                                                GFP_KERNEL);
    if(NULL == hrtimer_response_test_start_ns)
    {
        printk(KERN_INFO    "hrtimer_response_test_alloc: kmalloc failed for "
                            "hrtimer_response_test_start_ns");
        goto error0;
    }
    hrtimer_response_test_target_ns =   &(hrtimer_response_test_start_ns[test_length]);
    hrtimer_response_test_callback_ns = &(hrtimer_response_test_target_ns[test_length]);
    
    return 0;
error0:
    return -1;
}

static void hrtimer_response_test_free(void)
{
    kfree(  hrtimer_response_test_start_ns);
    hrtimer_response_test_start_ns =    NULL;    
    hrtimer_response_test_target_ns =   NULL;
    hrtimer_response_test_callback_ns = NULL;
}

/*
    allocate space for the string buffer of appropriate length
        2^64 ~= 10^20
        <sign><20 digit number><comma><tab>
        4 pieces of data: start time, target time, callback time, callback error,
        1 + 24*4 + 1= 100bytes
        
    extend the address space
    
    open the log file /var/log/hrtimer_response_test_log.csv
    
    loop through the results
    
        print the results as string to the buffer
        
        write the buffer to the file
        
    sync the file
    
    close the file
    
    restore the address space
    
    free the string buffer
    
*/
static int _hrtimer_response_test_write_log(void)
{
    int ret = 0;
    
    char *string_buffer;
    int string_len;
    ssize_t string_write_len;
    mm_segment_t oldfs;
    
    struct file* filp = NULL;
    
    loff_t pos = 0;
    
    s64 interval;
    s64 callback_error;

    int i;
    
    /*Allocate the string buffer*/
    string_buffer = kmalloc( (sizeof(char) * 128), GFP_KERNEL);
    if(NULL == string_buffer)
    {
        printk("_hrtimer_response_test_write_log: kmalloc failed for string buffer");
        ret = -1;
        goto exit0;
    }

    /*Save the previous address space and extend the current address space*/
    oldfs = get_fs();
    set_fs(get_ds());

    /*Open the standrad log file for VICtimer test*/
    filp = filp_open(   "/var/log/hrtimer_response_test_log.csv", 
                        (O_WRONLY | O_CREAT | O_TRUNC), 
                        (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH));
    if(IS_ERR(filp))
    {
        printk(KERN_INFO "_hrtimer_response_test_write_log: filp_open failed");
        ret = -1;
        goto exit1;
    }

    /*Loop over the samples*/
    for(i = 0; i < test_index; i++)
    {
        /*Compute the interval*/
        interval = hrtimer_response_test_target_ns[i] - hrtimer_response_test_start_ns[i];
        
        /*Compute the error*/
        callback_error =    hrtimer_response_test_target_ns[i] - 
                            hrtimer_response_test_callback_ns[i];
        
        /*Create the text to write to file*/
        string_len = snprintf(  string_buffer, 
                                (size_t)128, 
                                "\t%li,\t%li,\t%li,\t%li,\t%li\n",
                                (long)hrtimer_response_test_start_ns[i],
                                (long)hrtimer_response_test_target_ns[i],
                                (long)hrtimer_response_test_callback_ns[i],
                                (long)interval,
                                (long)callback_error);
        
        string_len = (string_len > 128)? 128 : string_len;
        
        /*Write to file*/
        string_write_len = vfs_write(filp, string_buffer, string_len, &pos);
        if(string_write_len < string_len)
        {
            printk(KERN_INFO "_hrtimer_response_test_write_log: vfs_write failed");
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


static int __init   hrtimer_response_test_init(void)
{
    int ret;

    s64 min_interval;

    s64 ns_current;
    s64 timer_interval;
    s64 next_expires;
    s64 next_soft_expires;

    /*Validate the parameters passed to the module*/
    min_interval = ((s64)interval_base) - interval_noisemag;
    if(min_interval < HRTIMER_RESPONSE_TEST_THRESHOLD)
    {
        printk(KERN_INFO    "hrtimer_response_test_init: The parameters "
                            "interval_base and interval_noisemag must be "
                            "assigned such that "
                            "(interval_base-interval_noisemag) > %li",
                            (long int)HRTIMER_RESPONSE_TEST_THRESHOLD);
        goto error0;
    }
    
    interval_base_u64 = (u64)interval_base;
    interval_noisemag_u64 = (u64)interval_noisemag;
    
    /*Allocate memory to store the results*/
    ret = hrtimer_response_test_alloc();
    if(0 != ret)
    {
        printk(KERN_INFO    "hrtimer_response_test_init: hrtimer_response_test_alloc "
                            "failed!");
        goto error0;
    }

    /*Initialize the test index*/
    test_index = 0;
    
    /*Initialize the state of the random generator*/
    lcg_state = 0;

    /*A divider is needed to extract the high order bits of an LCG*/
    _divider = 0xffffffffffffffff / (interval_noisemag_u64 * 2);

    /*Initialize the timer*/
    hrtimer_init(   &hrtimer_response_test_timer,
                    CLOCK_MONOTONIC,
                    HRTIMER_MODE_ABS);
    hrtimer_response_test_timer.function = hrtimer_response_test_callback;
    
    /*Get the current time*/
    ns_current = (hrtimer_response_test_timer.base->get_time()).tv64;

    /*Get the interval to set for the next timer*/
    timer_interval = _hrtimer_response_test_getNextInterval();
    
    /*Set the absolute target*/
    next_expires = ns_current + timer_interval;
    
    /*Set the soft target*/
    next_soft_expires = next_expires - (HRTIMER_RESPONSE_TEST_THRESHOLD/2);
    
    /*Log the timer start-time of the next timer callback*/
    hrtimer_response_test_start_ns[test_index] = ns_current;
    
    /*Start the timer*/
    hrtimer_start_range_ns( &hrtimer_response_test_timer,
                            (ktime_t){.tv64=next_soft_expires},
                            (HRTIMER_RESPONSE_TEST_THRESHOLD/2),
                            HRTIMER_MODE_ABS);
    
    return 0;
error0:
    return -1;
}

static void __exit  hrtimer_response_test_uninit(void)
{
    int ret;
    
    /*Cancel the hrtimer if it is active*/
    hrtimer_cancel(&(hrtimer_response_test_timer));
    
    /*Write the results out to the log file*/
    ret = _hrtimer_response_test_write_log();
    if(ret < 0)
    {
        printk(KERN_INFO "hrtimer_response_test_uninit: "
                            "_hrtimer_response_test_write_log failed!");
    }
    
    /*Free the memory allocated for storing results*/
    hrtimer_response_test_free();
}

module_init(hrtimer_response_test_init);
module_exit(hrtimer_response_test_uninit);

MODULE_LICENSE("GPL");

