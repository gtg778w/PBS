#include <linux/init.h>
#include <linux/module.h>
#include <asm/unistd.h>
#include <linux/hrtimer.h>

/*
 * struct hrtimer
 * hrtimer_init()
 * hrtimer_start()
 */

#include <linux/ktime.h>
/*
 * ktime_set
 */

#include "freq_change_test.h"
#include "LAMbS_molookup.h"

#define RP 20000000
#define RP_COUNT 4

struct hrtimer rp_timer;
u64 test_sched[32];
int rp_count = 0;


static int __init change_freq(void) {
    u64 per_mo;
    int i;

    per_mo = (u64)RP/LAMbS_mo_struct.count;
    printk(KERN_ALERT "using per_mo of %d ns", per_mo);
    for (i = 0; i < LAMbS_mo_struct.count; i++) {
	test_sched[i] = per_mo;
    }

    hrtimer_init(&rp_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    printk(KERN_ALERT "hrtimer_init: rp_timer initialized");
    rp_timer.function = &next_rp;
    next_rp(&rp_timer);

    return 0;
}

static void __exit end_count(void) {
    printk(KERN_ALERT "test module ended");
}


enum hrtimer_restart next_rp(struct hrtimer* timer) {

    if (rp_count < RP_COUNT) {
	rp_count++;
	hrtimer_start(&rp_timer, ktime_set(0, RP), HRTIMER_MODE_REL);
	printk(KERN_ALERT "hrtimer_start: rp_timer started");
	LAMbS_cpufreq_sched(test_sched);
	return HRTIMER_RESTART;
    } else {
	end_count();
	return HRTIMER_NORESTART;
    }
}

module_init(change_freq);
module_exit(end_count);

MODULE_LICENSE("GPL");
