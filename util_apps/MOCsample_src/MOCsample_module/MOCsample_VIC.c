#include "MOCsample.h"
#include <linux/ktime.h>
#include <linux/hrtimer.h>

/*__attribute__ ((weak)) u64 LAMbS_VIC_get(u64* timestamp_p)
{
    return 0;
}*/

u64 LAMbS_VIC_get(u64* timestamp_p);

int MOCsample_VIC_init(struct MOCsample_s* MOCsample_p)
{
    return 0;
}

u64 MOCsample_VIC_read(struct MOCsample_s* MOCsample_p)
{
    return LAMbS_VIC_get(NULL);
}

void MOCsample_VIC_free(struct MOCsample_s* MOCsample_p)
{
    return;
}

const MOCsample_t MOCsample_VIC_template =
{
    .init   = MOCsample_VIC_init,
    .read   = MOCsample_VIC_read,
    .free   = MOCsample_VIC_free,
    .state  = NULL,
    .last_sample= 0,
    .running_total  = 0
};

