#ifndef LAMbS_VIC_TIMER_INCLUDE
#define LAMbS_VIC_TIMER_INCLUDE

#include <linux/kernel.h>
#include <linux/hrtimer.h>

enum LAMbS_VICtimer_state { LAMbS_VICTIMER_INACTIVE=0x1, 
                            LAMbS_VICTIMER_HRTARMED=0x2, 
                            LAMbS_VICTIMER_CALLBACK=0x3,
                            LAMbS_VICTIMER_CALLBACK_STORM=0x4};

enum LAMbS_VICtimer_mode  { LAMbS_VICTIMER_ABS=0, 
                            LAMbS_VICTIMER_REL=1};

enum LAMbS_VICtimer_restart {   LAMbS_VICTIMER_NORESTART=0, 
                                LAMbS_VICTIMER_RESTART=1};

struct LAMbS_VICtimer_s;

typedef  enum LAMbS_VICtimer_restart 
        (*LAMbS_VICtimer_callback_t)(struct LAMbS_VICtimer_s*, u64 VIC_current);

typedef struct LAMbS_VICtimer_s
{
    struct list_head            activelist_entry;
    struct hrtimer              hrtimer;
    LAMbS_VICtimer_callback_t   function;
    u64                         target_VIC;
    enum LAMbS_VICtimer_state   state;
} LAMbS_VICtimer_t;



void LAMbS_VICtimer_motransition(void);

int LAMbS_VICtimer_cancel(  LAMbS_VICtimer_t *LAMbS_VICtimer_p);

int LAMbS_VICtimer_start(   LAMbS_VICtimer_t *LAMbS_VICtimer_p,
                            s64 target_VIC,
                            enum LAMbS_VICtimer_mode mode);

void LAMbS_VICtimer_init(   LAMbS_VICtimer_t *LAMbS_VICtimer_p);

int LAMbS_VICtimer_mechanism_init(void);
void LAMbS_VICtimer_mechanism_clear(void);

int LAMbS_VICtimer_start_test(int test_length, u64 VIC_interval);
int LAMbS_VICtimer_stop_test(void);

#endif

