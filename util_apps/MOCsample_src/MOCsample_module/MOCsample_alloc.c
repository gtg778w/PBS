#include <linux/slab.h>
#include <linux/preempt.h>
/*#include <linux/sched.h>*/
#include "MOCsample.h"

/*Slab cache for MOCsample_t objects*/
struct  kmem_cache *MOCsample_slab_cache = NULL;

/*The number of SRT tasks active in the system*/
static atomic_t MOCsample_alloc_count;

enum MOCsample_active_e {   MOCSAMPLE_INACTIVE=0,
                            MOCSAMPLE_ACTIVE=1};

enum MOCsample_active_e MOCsample_active = MOCSAMPLE_INACTIVE;
    
int MOCsample_alloc_init(void)
{
    int ret = 0;
    
    /*Initialize the slab cache*/
    MOCsample_slab_cache = KMEM_CACHE(MOCsample_s, SLAB_HWCACHE_ALIGN);
    if(NULL == MOCsample_slab_cache)
    {
        ret = -ENOMEM;
        goto exit0;
    }
    
    /*Reset the slabcache counter*/
    atomic_set(&MOCsample_alloc_count, 0);
    
    /*Set the MOCsample mechanism state to ACTIVE*/
    MOCsample_active = MOCSAMPLE_ACTIVE;

exit0:
    return ret;
}

void MOCsample_alloc_uninit(void)
{
    int local_count;
    
    /*Set the MOCsample mechanism state to INACTIVE*/
    MOCsample_active = MOCSAMPLE_INACTIVE;

    /*Disable Preemption*/
    preempt_disable();
    
        /*Check the state of the atomic variable*/
        local_count = atomic_read(&MOCsample_alloc_count);
        if(local_count != 0)
        {
            printk(KERN_INFO "<MOCsample_module> MOCsample_alloc_uninit: WARNING: "
                             "MOCsample_alloc_uninit called while "
                             "MOCsample_alloc_count is nonzero!");
            goto exit0;
        }
        
        /*Release the slab cache allocator*/
        kmem_cache_destroy(MOCsample_slab_cache);
    
exit0:
    /*Enable preemption*/
    preempt_enable();
}

int MOCsample_alloc(    const MOCsample_t *template, 
                        MOCsample_t **retval_p)
{
    int ret = 0;
    MOCsample_t *new_MOCsample_p = NULL;
    
    preempt_disable();
    
        if(MOCsample_active == MOCSAMPLE_ACTIVE)
        {
            /*attempt to allocate an MOCsample_t object*/
            new_MOCsample_p = kmem_cache_alloc(MOCsample_slab_cache, GFP_KERNEL);
            if(NULL == new_MOCsample_p)
            {                
                ret = -ENOMEM;
                goto error0;
            }
            
            /*initialize the MOCsample_t object*/
            *new_MOCsample_p = *template;
            ret = template->init(new_MOCsample_p);
            if(0 != ret)
            {
                goto error1;
            }

            new_MOCsample_p->timer = (struct MOCsample_timer_s){{0}};

            /*Increment the slab cache counter*/
            atomic_inc(&MOCsample_alloc_count);
        }
        else
        {
            ret = -EBUSY;
            goto error0;
        }
    
    preempt_enable();
    
    *retval_p = new_MOCsample_p;
    return 0;
    
error1:
    kmem_cache_free(MOCsample_slab_cache, new_MOCsample_p);
error0:
    preempt_enable();
    return ret;
}

void MOCsample_free(MOCsample_t *MOCsample_p)
{    
    preempt_disable();
 
        /*Uninit the MOCsample_t object*/
        MOCsample_p->free(MOCsample_p);
    
        /*Free the MOCsample_t object*/
        kmem_cache_free(MOCsample_slab_cache, MOCsample_p);
    
        /*Decrement the counter*/
        atomic_dec(&MOCsample_alloc_count);
                
    preempt_enable();
}

