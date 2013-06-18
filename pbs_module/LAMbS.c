
#include <linux/kernel.h>
/*
    printk
*/

#include "LAMbS_mo.h"

int LAMbS_init(void)
{
    int ret;
    
    ret = LAMbS_mo_init(0);
    if(0 != ret)
    {
        printk(KERN_INFO "LAMbS_init: LAMbS_mo_init failed");
        goto error0;
    }
    
    return 0;
    
error0:
    return ret;
}

void LAMbS_uninit(void)
{
    LAMbS_mo_uninit();
}

