#include "MOCsample.h"
#include <linux/ktime.h>
#include <linux/hrtimer.h>

int MOCsample_nsec_init(struct MOCsample_s* MOCsample_p)
{
   return 0;
}

u64 MOCsample_nsec_read(struct MOCsample_s* MOCsample_p)
{
    ktime_t now_ktime;
    
    now_ktime = ktime_get();
    return now_ktime.tv64;
}

void MOCsample_nsec_free(struct MOCsample_s* MOCsample_p)
{
    return;
}

const MOCsample_t MOCsample_nsec_template =
{
    .init   = MOCsample_nsec_init,
    .read   = MOCsample_nsec_read,
    .free   = MOCsample_nsec_free,
    .state  = NULL,
    .last_sample= 0,
    .running_total  = 0
};

