#include <linux/gfp.h>
#include <asm/uaccess.h>
#include "MOCsample.h"
#include "MOCsample_timer_command.h"

/*The threshold should be configured based on timer resolution*/
s64 MOCsample_timer_threshold_ns = 3000;

void MOCsample_timer_threshold_init(void)
{
    struct timespec ts;
    
    /*Get timer resolution to compute the threshold value*/
    hrtimer_get_res(CLOCK_MONOTONIC, &ts);
    
    /*Set the threshold to the timer resolution if larger than the default value*/
    MOCsample_timer_threshold_ns =  (ts.tv_nsec > MOCsample_timer_threshold_ns)? 
                                    ts.tv_nsec : MOCsample_timer_threshold_ns;
}

static enum hrtimer_restart MOCsample_timer_callback(struct hrtimer *timer)
{
    enum hrtimer_restart ret = HRTIMER_RESTART;
    ktime_t now;
    u64 overruns;
    u64 sample;
    u64 sample_idx;
    
    struct tasklet_hrtimer  *tasklet_hrtimer_p;
    MOCsample_timer_t   *MOCsample_timer_p;
    MOCsample_t         *MOCsample_p;
    
    tasklet_hrtimer_p   = container_of(timer, struct tasklet_hrtimer, timer);
    MOCsample_timer_p   = container_of(tasklet_hrtimer_p, struct MOCsample_timer_s, timer);
    MOCsample_p         = container_of(MOCsample_timer_p, struct MOCsample_s, timer);
    
    now = hrtimer_cb_get_time(timer);
    overruns = hrtimer_forward( timer,
                                now,
                                MOCsample_timer_p->sample_period);
    if(overruns > 0)
    {
        sample      = MOCsample_p->read(MOCsample_p);
        sample_idx  = MOCsample_timer_p->sampled_count;
        
        MOCsample_timer_p->samples_buffer[sample_idx].time_stamp = ktime_to_ns(now);
        MOCsample_timer_p->samples_buffer[sample_idx].MOCsample  = sample;
        
        sample_idx++;
        MOCsample_timer_p->sampled_count = sample_idx;
        
        if(sample_idx == MOCsample_timer_p->buffer_length)
        {
            printk(KERN_INFO "MOCsample_timer_callback: Sampling Complete!");
            ret = HRTIMER_NORESTART;
        }
    }
    
    return ret;
}

static MOCsample_timed_sample_t *alloc_timed_samples(u64 sample_count)
{
    u64 log_size;
    u64 page_order;
    MOCsample_timed_sample_t *samples_buffer = NULL;
    
    if(sample_count > 0)
    {
        log_size            =   sample_count * sizeof(MOCsample_timed_sample_t);
        page_order          =   get_order(log_size);
        samples_buffer      =   (MOCsample_timed_sample_t *)__get_free_pages(GFP_KERNEL, page_order);
        if(NULL == samples_buffer)
        {
            printk(KERN_INFO "alloc_timed_samples: __get_free_pages failed for order = %llu", (long long unsigned)page_order);
        }
    }
    
    return samples_buffer;
}

static void free_timed_samples(u64 sample_count, MOCsample_timed_sample_t *samples_buffer)
{
    u64 log_size;
    u64 page_order;

    if(sample_count > 0)
    {        
        log_size            =   sample_count * sizeof(MOCsample_timed_sample_t);
        page_order          =   get_order(log_size);
        free_pages((unsigned long)samples_buffer, (unsigned int)page_order);
    }
}

void    MOCsample_timer_free(   MOCsample_t *MOCsample_p)
{
    MOCsample_timer_t *MOCsample_timer_p = &(MOCsample_p->timer);
    
    tasklet_hrtimer_cancel(&(MOCsample_timer_p->timer));
    free_timed_samples( MOCsample_timer_p->buffer_length, 
                        MOCsample_timer_p->samples_buffer);
    MOCsample_timer_p->sample_period = (ktime_t){.tv64=~(u64)0};
    MOCsample_timer_p->sampled_count = 0;
    MOCsample_timer_p->buffer_length = 0;
    MOCsample_timer_p->samples_buffer= NULL;
}

void    MOCsample_timer_init(   MOCsample_t *MOCsample_p)
{
    MOCsample_timer_t *MOCsample_timer_p = &(MOCsample_p->timer);
    
    MOCsample_timer_p->sample_period = (ktime_t){.tv64=~(u64)0};
    MOCsample_timer_p->sampled_count = 0;
    MOCsample_timer_p->buffer_length = 0;
    MOCsample_timer_p->samples_buffer= NULL;
    
    tasklet_hrtimer_init(   &(MOCsample_timer_p->timer),
                            MOCsample_timer_callback,
                            CLOCK_MONOTONIC,
                            HRTIMER_MODE_REL);
}

static int MOCsample_timer_start(   MOCsample_t *MOCsample_p,
                                    u64         period,
                                    u64         sample_count)
{
    int ret;
    MOCsample_timed_sample_t    *samples_buffer = NULL;
    MOCsample_timer_t           *MOCsample_timer_p;
    
    u64 sample_idx = 0;
    u64 now, sample;
    u64 soft_target;
    
    MOCsample_timer_p = &(MOCsample_p->timer);
    
    /*In case the timer_start function was called earlier, cancel the hrtimer and
    free any allocated memory*/
    MOCsample_timer_free(MOCsample_p);
    
    /*If there is at least one sample*/
    if(sample_count > 0)
    {
        /*Allocate the samples buffer*/
        samples_buffer = alloc_timed_samples(sample_count);
        if(NULL == samples_buffer)
        {
            printk(KERN_INFO "MOCsample_timer_start: alloc_timed_samples failed");
            ret = -ENOMEM;
            goto exit0;
        }
    
        /*Get the current time and sample and store it in the buffer*/
        now     = ktime_to_ns(hrtimer_cb_get_time(&(MOCsample_timer_p->timer.timer)));
        sample  = MOCsample_p->read(MOCsample_p);

        sample_idx  = 0;
        samples_buffer[sample_idx].time_stamp = now;
        samples_buffer[sample_idx].MOCsample  = sample;
        sample_idx++;

        /*If there is more than one sample*/        
        if(sample_idx < sample_count)
        {
            /*Compute the first sample-time target*/
            soft_target = now + period - (MOCsample_timer_threshold_ns/2);
            
            /*Initialize the timer accordingly*/
            hrtimer_start_range_ns( &(MOCsample_timer_p->timer.timer), 
                                    (ktime_t){.tv64=soft_target},
                                    (MOCsample_timer_threshold_ns/2), 
                                    HRTIMER_MODE_ABS);
        }
        
    }

    /*Initialize the fields of the MOCsample_timer structure*/
    MOCsample_timer_p->sampled_count    = sample_idx;
    MOCsample_timer_p->buffer_length    = sample_count;
    MOCsample_timer_p->samples_buffer   = samples_buffer;
    
    /*The time-to-target is the period minus half the threshold*/
    MOCsample_timer_p->sample_period    = (ktime_t){.tv64=period};

    /*No errors so far. The return code is 0*/
    ret = 0;
    
exit0:
    return ret;
}

int MOCsample_timer_stop(   MOCsample_t *MOCsample_p,
                            u64         buffer_len,
                            void __user *buffer,
                            u64         *sample_count_p)
{
    int ret;
    MOCsample_timer_t           *MOCsample_timer_p;
    
    u64 samples_available;
    u64 samples_to_copy;
    u64 bytes_to_copy;
    
    void *source;
    
    MOCsample_timer_p = &(MOCsample_p->timer);
    
    /*Cancel the timer*/
    tasklet_hrtimer_cancel(&(MOCsample_timer_p->timer));
    
    /*Determine how many samples to copy*/
    samples_available   = MOCsample_timer_p->sampled_count;
    samples_to_copy = (samples_available < buffer_len)? samples_available : buffer_len;
    bytes_to_copy   = samples_to_copy * sizeof(MOCsample_timed_sample_t);
    source  = (void*)MOCsample_timer_p->samples_buffer;
    
    /*Copy the samples to user space if there are samples to copy*/
    if(samples_to_copy > 0)
    {
        if(copy_to_user( buffer, source, bytes_to_copy))
        {
            ret = -EFAULT;
            goto exit0;
        }
    }
    
    /*Write the sample_count*/
    *sample_count_p = samples_available;

    /*Cancel the timer and free any allocated log memory*/
    MOCsample_timer_free(MOCsample_p);

    /*Clean exit. return 0*/
    ret = 0;
exit0:
    return ret;
}

ssize_t MOCsample_timer_write(MOCsample_t *MOCsample_p, const char __user *src, size_t count)
{
    MOCsample_timer_command_t command;
    ssize_t ret;
    
    /*Check that the size is correct*/
    if( count != sizeof(MOCsample_timer_command_t))
    {
        printk(KERN_INFO "MOCsample_timer_write: count must equal the size of MOCsample_timer_command_t");
        ret = -EINVAL;
        goto exit0;
    }
    
    /*Copy the command into kernel space*/
    if(copy_from_user(&command, src, count))
    {
        ret = -EFAULT;
        goto exit0;
    }
    
    /*Check the command*/
    switch(command.command)
    {
        case MOCsample_TIMER_COMMAND_START:            
            ret = MOCsample_timer_start(MOCsample_p,
                                        command.arguments[MOCsample_TIMER_COMMAND_START_PERIOD],
                                        command.arguments[MOCsample_TIMER_COMMAND_START_COUNT]);
            if(ret < 0)
            {
                printk(KERN_INFO "MOCsample_timer_write: MOCsample_timer_start failed");
                goto exit0;
            }
            break;
            
        case MOCsample_TIMER_COMMAND_STOP:
            ret = MOCsample_timer_stop( MOCsample_p,
                                        command.arguments[MOCsample_TIMER_COMMAND_STOP_BUFFSIZ],
                                        (void __user *)command.arguments[MOCsample_TIMER_COMMAND_STOP_BUFFPTR],
                                        &(command.arguments[MOCsample_TIMER_COMMAND_STOP_VLDELMS]));
            if(ret < 0)
            {
                printk(KERN_INFO "MOCsample_timer_write: MOCsample_timer_stop failed");
                goto exit0;
            }
            else
            {
                /*Copy the command back to user space*/
                if(copy_to_user((void __user *)src, &command, count))
                {
                    ret = -EFAULT;
                    goto exit0;
                }
            }
            break;
            
        default:
            printk(KERN_INFO "MOCsample_timer_write: invalid command");
            ret = -EINVAL;
            goto exit0;
    }

    ret = count;    
exit0:
    return ret;
}

