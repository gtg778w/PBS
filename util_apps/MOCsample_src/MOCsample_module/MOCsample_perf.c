#include <linux/kernel.h>
#include <linux/perf_event.h>

#include "MOCsample.h"

int MOCsample_inst_init(struct MOCsample_s* MOCsample_p)
{
    int ret = 0;
    struct perf_event_attr perf_attr = {0};
    struct perf_event* perf_event = NULL;

    perf_attr.type = PERF_TYPE_HARDWARE;
    perf_attr.size = sizeof(struct perf_event_attr);
    perf_attr.config = PERF_COUNT_HW_INSTRUCTIONS;
    perf_attr.disabled = 0;
    perf_attr.exclude_kernel = 1;
    perf_attr.exclude_user = 0; /*possibly unnecessary, but can't hurt */
    perf_attr.exclude_hv = 1; /* for running in VMs */
    perf_event = perf_event_create_kernel_counter(&perf_attr, -1, current, NULL, NULL);
    if(IS_ERR(perf_event))
    {
        printk(KERN_INFO "MOCsample_module: perf_event_create_kernel_counter failed!");
        ret = PTR_ERR(perf_event);
        MOCsample_p->state = NULL;
        goto exit0;
    }
    else
    {
        MOCsample_p->state = perf_event;
    }
    
exit0:
    return ret;
}

int MOCsample_cycl_init(struct MOCsample_s* MOCsample_p)
{
    int ret = 0;
    struct perf_event_attr perf_attr = {0};
    struct perf_event* perf_event = NULL;

    perf_attr.type = PERF_TYPE_HARDWARE;
    perf_attr.size = sizeof(struct perf_event_attr);
    perf_attr.config = PERF_COUNT_HW_CPU_CYCLES;
    perf_attr.disabled = 0;
    perf_attr.exclude_kernel = 0;
    perf_attr.exclude_user = 0; /*possibly unnecessary, but can't hurt */
    perf_attr.exclude_hv = 0; /* for running in VMs */
    perf_event = perf_event_create_kernel_counter(&perf_attr, 0, NULL, NULL, NULL);

    if(IS_ERR(perf_event))
    {
        ret = PTR_ERR(perf_event);
        MOCsample_p->state = NULL;
        goto exit0;
    }
    else
    {
        MOCsample_p->state = perf_event;
    }
    
exit0:
    return ret;
}

u64 MOCsample_perf_read(struct MOCsample_s* MOCsample_p)
{
    struct perf_event* perf_event;
    u64 next_sample, enabled = 0, running = 0;

    perf_event  = (struct perf_event*)MOCsample_p->state;
    next_sample = perf_event_read_value(perf_event, &enabled, &running);
    
    return next_sample;
}

void MOCsample_perf_free(struct MOCsample_s* MOCsample_p)
{
    struct perf_event* perf_event = (struct perf_event*)MOCsample_p->state;
    if(NULL != perf_event)
    {
        perf_event_release_kernel(perf_event);
    }
}

const MOCsample_t MOCsample_inst_template =
{
    .init   = MOCsample_inst_init,
    .read   = MOCsample_perf_read,
    .free   = MOCsample_perf_free,
    .state  = NULL,
    .last_sample= 0,
    .running_total  = 0
};

const MOCsample_t MOCsample_cycl_template =
{
    .init   = MOCsample_cycl_init,
    .read   = MOCsample_perf_read,
    .free   = MOCsample_perf_free,
    .state  = NULL,
    .last_sample= 0,
    .running_total  = 0
};

