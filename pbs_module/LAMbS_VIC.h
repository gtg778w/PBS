#ifndef LAMbS_VIC_INCLUDE
#define LAMbS_VIC_INCLUDE

#include <linux/kernel.h>

#include <linux/sched.h>

#define LAMbS_clock() sched_clock()

int setup_sched_clock(void);

int     LAMbS_VIC_init(void);
u64     LAMbS_VIC_get(u64* timestamp_p);
void    LAMbS_VIC_uninit(void);

#endif
