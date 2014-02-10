#include <asm/uaccess.h>
#include "MOCsample.h"
#include "MOCsample_timer_command.h"

int MOCsample_timer_start(  MOCsample_t *MOCsample_p,
                            u64         period,
                            u64         sample_count)
{
    printk(KERN_INFO "MOCsample_timer_write: start command issued.");
    return 0;
}

int MOCsample_timer_stop(   MOCsample_t *MOCsample_p,
                            u64         buffer_len,
                            MOCsample_timed_sample_t __user *buffer,
                            u64         *sample_count)
{
    printk(KERN_INFO "MOCsample_timer_write: stop command issued.");
    return 0;
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
        goto error0;
    }
    
    /*Copy the command into kernel space*/
    if(copy_from_user(&command, src, count))
    {
        ret = -EFAULT;
        goto error0;
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
                goto error0;
            }
            break;
        case MOCsample_TIMER_COMMAND_STOP:
            ret = MOCsample_timer_stop( MOCsample_p,
                                        command.arguments[MOCsample_TIMER_COMMAND_STOP_BUFFSIZ],
                                        (MOCsample_timed_sample_t __user *)command.arguments[MOCsample_TIMER_COMMAND_STOP_BUFFPTR],
                                        &(command.arguments[MOCsample_TIMER_COMMAND_STOP_VLDELMS]));
            if(ret < 0)
            {
                printk(KERN_INFO "MOCsample_timer_write: MOCsample_timer_stop failed");
                goto error0;
            }
            break;
        default:
            printk(KERN_INFO "MOCsample_timer_write: invalid command");
            ret = -EINVAL;
            goto error0;
    }

    ret = count;    
error0:
    return ret;
}

