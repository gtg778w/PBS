#ifndef LAMBS_GOVERNOR_H
#define LAMBS_GOVERNOR_H

#include <linux/interrupt.h>
#include "LAMbS_mo.h"
#include "LAMbS_molookup.h"
#include "LAMbS_mostat.h"

static int LAMbS_cpufreq_notifier(struct notifier_block *nb, unsigned long val, void *data);
enum hrtimer_restart schedule_next_moi(struct hrtimer* timer);
void LAMbS_cpufreq_sched(u64 LAMbS_mo_schedule[]);
static int LAMbS_freq_set(u32 freq);
static int cpufreq_governor_lambs(struct cpufreq_policy *policy, unsigned int event);
static int __init cpufreq_gov_lambs_init(void);
static void __exit cpufreq_gov_lambs_exit(void);

#endif

