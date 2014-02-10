#include <asm/uaccess.h>
#include "MOCsample.h"
#include "MOCsample_timer_command.h"

ssize_t MOCsample_timer_write(MOCsample_t *MOCsample_p, const char __user *src, size_t count)
{
    MOCsample_timer_command_t command;
    ssize_t ret = count;
    
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
            printk(KERN_INFO "MOCsample_timer_write: start command issued.");
            break;
        case MOCsample_TIMER_COMMAND_STOP:
            printk(KERN_INFO "MOCsample_timer_write: stop command issued.");
            break;
        default:
            printk(KERN_INFO "MOCsample_timer_write: invalid command");
            ret = -EINVAL;
            goto error0;
    }
    
error0:
    return ret;
}

