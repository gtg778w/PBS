#include <linux/hrtimer.h>
#include "LAMbS_molookup.h"

extern void LAMbS_cpufreq_sched(u64 LAMbS_mo_schedule[]);
extern struct LAMbS_mo_struct LAMbS_mo_struct;

static int __init change_freq(void);
static void __exit end_count(void);
enum hrtimer_restart next_rp(struct hrtimer* timer);

