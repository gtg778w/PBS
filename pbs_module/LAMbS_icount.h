#ifndef LAMbS_ICOUNT_INCLUDE
#define LAMbS_ICOUNT_INCLUDE
#include <linux/kernel.h>
#include <linux/perf_event.h>
#include <asm/unistd.h>

typedef struct LAMbS_icount_s
{
    u64 icount;
    u64 icount_kernel;
    /* do we want these? running is time running, enabled is time enabled*/ 
    u64 running;
    u64 enabled;
} LAMbS_icount_t;

extern struct perf_event_attr icount_attr;
extern struct perf_event_attr icount_attr_kernel;
extern struct perf_event* icount_event_kernel;
extern struct perf_event* icount_event;
extern bool debug;

int LAMbS_icount_init(bool debug);
void LAMbS_icount_uninit(void);

void LAMbS_icount_get(LAMbS_icount_t* mostat);
void LAMbS_icount_getDelta(LAMbS_icount_t* mostat, u64 *delta_icount_p);

#endif
