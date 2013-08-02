/*
 * PBS/gov_test/gov_test.c
 *
 * This will set the cpu_freqency to minimum frequency and then
 * accept timings from the allocator to spend time in different MOs
 *
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/smp.h>

/* ? */

#include <linux/cpufreq.h>
#include <linux/mutex.h>

/* mutex for frequency changing */

#include <linux/types.h>
#include <linux/cpu.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/ktime.h>

/* ktime_set() */

#include <linux/hrtimer.h>

/* 
 * hrtimer_forward_now()
 */

#include <linux/interrupt.h>

/* tasklet_hrtimer
 * tasklet_hrtimer_init()
 * tasklet_hrtimer_start()
 * tasklet_hrtimer_cancel()
 */


#include "LAMbS_governor_main.h"


#define MIN_THRESH 10000

/* I think this is needed by per_cpu and it seems easier just to add it */
static DEFINE_PER_CPU(unsigned int, cpu_max_freq);
static DEFINE_PER_CPU(unsigned int, cpu_min_freq);
static DEFINE_PER_CPU(unsigned int, cpu_cur_freq);  /*current frequency */
static DEFINE_PER_CPU(unsigned int, cpu_set_freq);  /* desired frequency */
static DEFINE_PER_CPU(unsigned int, cpu_is_managed); /* governor represents cpu */

static DEFINE_MUTEX(setfreq_mutex);
struct tasklet_hrtimer LAMbS_tasklet_hrtimer;

struct cpufreq_policy *policy_p;
u64 maximum_transition_latency;
int moi;
u64* schedule;
int running = 0;

static int LAMbS_cpufreq_notifier(struct notifier_block *nb, unsigned long val,
				  void *data) {
    struct cpufreq_freqs *freq = data;
    if (!per_cpu(cpu_is_managed, freq->cpu)) {
	return 0;
    }

    if (val == CPUFREQ_POSTCHANGE) {
	printk(KERN_NOTICE "saving cpu_cur_freq of cpu %u to be %u kHz\n", freq->cpu, freq->new);
	per_cpu(cpu_cur_freq, freq->cpu) = freq->new;
    }
    return 0;
}



static struct notifier_block lambs_cpufreq_notifier_block = {
    .notifier_call = LAMbS_cpufreq_notifier
};


enum hrtimer_restart schedule_next_moi(struct hrtimer* timer) {
    /* number of transitions count? */
    while (moi < LAMbS_mo_struct.count - 1) {
	/*printk(KERN_ALERT "in schedule_next_moi: schedule[%d] = %llu\n", moi, schedule[moi]);*/
	if (schedule[moi]) {
	    moi++;
	    hrtimer_forward_now(timer, ktime_set(0,schedule[moi]));
	    /*printk(KERN_ALERT "hrtimer restarted\n");*/
	    LAMbS_freq_set(LAMbS_mo_struct.table[moi]);
	    /*printk(KERN_ALERT "hrtimer: schedule[%d] = %llu ns @ %d kHz\n", moi, schedule[moi], LAMbS_mo_struct.table[moi]);*/
	    
	    return HRTIMER_RESTART;
	} else {
	    moi++;
	    continue;
	}

    }
    
    /* 
     * if we make it to the end of the schedule, cancel the timer because
     * it will be restarted next LAMbS_cpufreq_sched()
     */

    tasklet_hrtimer_cancel(&LAMbS_tasklet_hrtimer);
    return HRTIMER_NORESTART;
}

void LAMbS_cpufreq_sched(u64 LAMbS_mo_schedule[]) {
    int i;
    int active;
    u64 leftovers = 0;
    moi = 0;
    schedule = LAMbS_mo_schedule;
    
    for(i = 0; i < LAMbS_mo_struct.count; i++) {
	printk(KERN_ALERT "schedule[%d] = %llu ns\n", i, schedule[i]);
	if (schedule[i] && (schedule[i] < maximum_transition_latency)) {
	    leftovers += schedule[i];
	    schedule[i] = 0;
	    printk(KERN_ALERT "schedule[%d] below threshold, zeroed and added to leftovers\n",i);
	} else {
	    if (leftovers && schedule[i]) {
		schedule[i] += leftovers;
		printk(KERN_ALERT "%lluns added to schedule[%d] and leftovers zeroed\n", leftovers, i);
		leftovers = 0;
	    }
	}
    }

    if (leftovers) {
	printk(KERN_ALERT "WARNING: %lluns leftover! Not assigned to any MO\n", leftovers);
    }

    active = tasklet_hrtimer_start(&LAMbS_tasklet_hrtimer, ktime_set(0,schedule[moi]), HRTIMER_MODE_REL);

    
    /* debug: check to see if schedule finished and timer was cancelled */

    if (active) {
        printk(KERN_ALERT "hrtimer already active\n");
    } else {
	printk(KERN_ALERT "hrtimer started for first time\n");
    }
    
}



EXPORT_SYMBOL_GPL(LAMbS_cpufreq_sched);

/* 
 * Actually do the frequency change. Requires a mutex_lock. Code can eventually be 
 * trimmed to not log, etc. but for now it will basically reuse the gov_test code.
 */

int LAMbS_freq_set(u32 freq) {
    int ret = -EINVAL;

    if (!per_cpu(cpu_is_managed, policy_p->cpu)) {
	printk(KERN_NOTICE "freq not set: cpu_is_managed for cpu %u = false\n", policy_p->cpu);
	goto err;
    }

    per_cpu(cpu_set_freq, policy_p->cpu) = freq;

     /*pretty sure this isn't needed. __cpufreq_driver_target does this 
    
    if (freq < per_cpu(cpu_min_freq, policy->cpu)) {
	freq = per_cpu(cpu_min_freq, policy->cpu);
    }
    if (freq > per_cpu(cpu_max_freq, policy->cpu)) {
	freq = per_cpu(cpu_max_freq, policy->cpu);
    }
   */ 

    ret = __cpufreq_driver_target(policy_p, freq, CPUFREQ_RELATION_L);


err:
    if (ret) {
	printk(KERN_ALERT "LAMbS_cpufreq_set NOT set on cpu%u, freq %ukHz (return: %d", policy_p->cpu, freq, ret);
    } else {
	printk(KERN_NOTICE "LAMbS_cpufreq_set for cpu %u, freq %u kHz\n", policy_p->cpu, freq);
    }
    return ret;
}


EXPORT_SYMBOL_GPL(LAMbS_freq_set);

/* from Documentation/cpu-freq/governors.txt
 *
 * If you need other "events" externally of your driver, _only_ use the
 * cpufreq_governor_l(unsigned int cpu, unsigned int event) call to the
 * CPUfreq core to ensure proper locking.
 */

/* this code mostly repurposed from cpufreq_userspace.c. I got rid of most
 * of the SMP stuff, though the locks and per_cpu remain just in case */


static int cpufreq_governor_lambs(struct cpufreq_policy *policy, unsigned int event) {
    unsigned int cpu = policy->cpu;
    int rc = 0;
    
    switch (event) {
    case CPUFREQ_GOV_START:

	/*mutex_lock(&setfreq_mutex);*/
	/* one processor only! */
	
	cpufreq_register_notifier(&lambs_cpufreq_notifier_block, CPUFREQ_TRANSITION_NOTIFIER);
	
	/* macros for setting policy per cpu. Probably not need for our nonSMP
	 * but maybe. */

	per_cpu(cpu_is_managed, cpu) = 1;
	per_cpu(cpu_min_freq, cpu) = policy->min;
	per_cpu(cpu_max_freq, cpu) = policy->max;
	per_cpu(cpu_cur_freq, cpu) = policy->cur;
	per_cpu(cpu_set_freq, cpu) = policy->cur;
	
	/*mutex_unlock(&setfreq_mutex);*/

	printk(KERN_NOTICE "managing cpu %u started (%u - %u kHz, currently %u kHz)\n",
	    cpu, per_cpu(cpu_min_freq, cpu), per_cpu(cpu_max_freq, cpu),
	    per_cpu(cpu_cur_freq, cpu));

	/* keep local pointer for frequency setting */
	policy_p = policy;

	/* try to get maximum transition latency or just use default of 10us */
	if (policy->cpuinfo.transition_latency) {
	    maximum_transition_latency = (u64)policy->cpuinfo.transition_latency;
	    printk(KERN_ALERT "policy->cpuinfo.transition_latency = %llu set as threshold", maximum_transition_latency);
	} else {
	    maximum_transition_latency = MIN_THRESH;
	    printk(KERN_ALERT "policy->cpuinfo.transition_latency = NULL, %llu is default", maximum_transition_latency);
	}

	/* initialize timer */
	tasklet_hrtimer_init(&LAMbS_tasklet_hrtimer, &schedule_next_moi, CLOCK_MONOTONIC, HRTIMER_MODE_REL);

	break;
    case CPUFREQ_GOV_STOP:
	mutex_lock(&setfreq_mutex);
	
	/* one core only! */
	
	cpufreq_unregister_notifier(&lambs_cpufreq_notifier_block, CPUFREQ_TRANSITION_NOTIFIER);
	
	per_cpu(cpu_is_managed, cpu) = 0;
	per_cpu(cpu_max_freq, cpu) = 0;
	per_cpu(cpu_min_freq, cpu) = 0;
	per_cpu(cpu_set_freq, cpu) = 0;
	
	mutex_unlock(&setfreq_mutex);

	tasklet_hrtimer_cancel(&LAMbS_tasklet_hrtimer);

	printk(KERN_NOTICE "managing cpu %u stopped\n", cpu);

	break;
    case CPUFREQ_GOV_LIMITS:
	printk(KERN_NOTICE "limit event for cpu %u: %u - %u kHz, currently %u kHz, "
		 "last set to %u kHz\n", cpu, policy->min, policy->max,
		 per_cpu(cpu_cur_freq, cpu), per_cpu(cpu_set_freq, cpu));
	
	/*mutex_lock(&setfreq_mutex);*/
	
	if (policy->max < per_cpu(cpu_set_freq, cpu)) {
	    __cpufreq_driver_target(policy, policy->max, CPUFREQ_RELATION_H);
	} else if (policy->min > per_cpu(cpu_set_freq, cpu)) {
	    __cpufreq_driver_target(policy, policy->min, CPUFREQ_RELATION_L);
	} else {
	    __cpufreq_driver_target(policy, per_cpu(cpu_set_freq, cpu),
				    CPUFREQ_RELATION_L);
	}
	per_cpu(cpu_min_freq, cpu) = policy->min;
	per_cpu(cpu_max_freq, cpu) = policy->max;
	per_cpu(cpu_cur_freq, cpu) = policy->cur;
	
	mutex_unlock(&setfreq_mutex);
	
	break;
    }
    return rc;
}



struct cpufreq_governor cpufreq_gov_lambs = {
    .name	= "lambs",
    .governor	= cpufreq_governor_lambs,
    .owner	= THIS_MODULE,
};

/* remove verbose */

static int __init cpufreq_gov_lambs_init(void) {
    int ret;
    ret = LAMbS_mo_init(0);
    if (!ret) {
        return cpufreq_register_governor(&cpufreq_gov_lambs);
    } else {
	printk(KERN_ERR "LAMbS_mo_init failed with error %d", ret);
	return ret;
    }
}

static void __exit cpufreq_gov_lambs_exit(void) {
    cpufreq_unregister_governor(&cpufreq_gov_lambs);
    LAMbS_mo_uninit(); 
    printk(KERN_ALERT "LAMbS_governor removed successfully\n");
}

MODULE_AUTHOR("Michael Giardino <giardino@ece.gatech.edu>");
MODULE_DESCRIPTION("CPUfreq policy governor for LAMbS");
MODULE_LICENSE("GPL");

module_init(cpufreq_gov_lambs_init);
module_exit(cpufreq_gov_lambs_exit);
