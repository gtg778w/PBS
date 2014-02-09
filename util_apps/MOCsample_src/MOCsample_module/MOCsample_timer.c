
#include "MOCsample.h"
#include "MOCsample_timer_command.h"

ssize_t MOCsample_timer_write(MOCsample_t *MOCsample_p, const char __user *src, size_t count)
{
    int ret;
    
    if( count != sizeof(MOCsample_timer_command_t))
    {
        printk(KERN_INFO "MOCsample_timer_write: count must equal the size of MOCsample_timer_command_t");
        ret = -EINVAL;
        goto error0;
    }
    
    return count;
error0:
    return ret;
}

