
#include "LAMbS_icount.h"

struct perf_event* icount_event; /*is this the best place to declare this? */
struct perf_event* icount_event_kernel; 
struct LAMbS_icount_t* icount;

int LAMbS_icount_init()
{
    struct perf_event_attr icount_attr;
    
    memset(&icount_attr, 0, sizeof(struct perf_event_attr));

    icount_attr.type = PERF_TYPE_HARDWARE;
    icount_attr.size = sizeof(struct perf_event_attr);
    icount_attr.config = PERF_COUNT_HW_INSTRUCTIONS;
    
    /* should this be disabled upon starting? probably not, because
     * we can grab the current instruction count at the beginning of a res
     * period */
    icount_attr.disabled = 0;
    icount_attr.exclude_kernel = 0;
    icount_attr.exclude_user = 0; /*possibly unnecessary, but can't hurt */
    icount_attr.exclude_hv = 1; /* for running in VMs */
    /* function(perf_event_attr, int cpu, task (process?), overflow handler, context) */
    icount_event = perf_event_create_kernel_counter(&icount_attr, 0, NULL, NULL, NULL);

    if (!icount_event) {
	printk(KERN_ERR "Cannot allocate instruction counter");
	return -1; /* should this be something else? */
    }

    return 0;
}

void LAMbS_icount_uninit()
{
    if(icount_event) {
    	perf_event_release_kernel(icount_event);
	printk(KERN_INFO "Instruction count register released (user).");
    }
}

void LAMbS_icount_get(LAMbS_icount_t* icount)
{
    /* remove and replace with icount.enabled and .running if we add 
     * it to struct LAMbS_icount_t */
    u64 enabled = 0;
    u64 running = 0;

    icount->icount = perf_event_read_value(icount_event, &enabled, &running);
    icount->running = running;
    icount->enabled = enabled;
    
}

void LAMbS_icount_getDelta(LAMbS_icount_t* icount, u64 *delta_icount_p)
{
    u64 icount_old;
    icount_old = icount->icount;
    LAMbS_icount_get(icount);
    if (unlikely(icount_old > icount->icount)) { /* did we decide this wasn't needed? */
	*delta_icount_p = icount->icount + ((1 << sizeof(u64)) - icount_old);
    } else {
	*delta_icount_p = icount->icount - icount_old;
    }

}

