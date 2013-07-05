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
#include <linux/cpufreq.h>
#include <linux/mutex.h>
#include <linux/types.h>
#include <linux/cpu.h>
#include <linux/init.h>
#include <linux/printk.h>


/* I think this is needed by per_cpu and it seems easier just to add it */
static DEFINE_PER_CPU(unsigned int, cpu_max_freq);
static DEFINE_PER_CPU(unsigned int, cpu_min_freq);
static DEFINE_PER_CPU(unsigned int, cpu_cur_freq);  /*current frequency */
static DEFINE_PER_CPU(unsigned int, cpu_set_freq);  /* desired frequency */
static DEFINE_PER_CPU(unsigned int, cpu_is_managed); /* governor represents cpu */

static DEFINE_MUTEX(setfreq_mutex);

static int lambs_cpufreq_notifier(struct notifier_block *nb, unsigned long val,
				  void *data) {
    struct cpufreq_freqs *freq = data;
    if (!per_cpu(cpu_is_managed, freq->cpu)) {
	return 0;
    }

    if (val == CPUFREQ_POSTCHANGE) {
	printk(KERN_NOTICE "saving cpu_cur_freq of cpu %u to be %u kHz\n",
		 freq->cpu, freq->new);
	per_cpu(cpu_cur_freq, freq->cpu) = freq->new;
    }
    return 0;
}

static struct notifier_block lambs_cpufreq_notifier_block = {
    .notifier_call = lambs_cpufreq_notifier
};

int LAMbS_cpufreq_sched(u64 LAMbS_mo_schedule) {
    
}

EXPORT_SYMBOL_GPL(LAMbS_cpufreq_sched);

int LAMbS_cpufreq_set(struct cpufreq_policy *policy, unsigned int freq) {
    int ret = -EINVAL;

    printk(KERN_NOTICE "LAMbS_cpufreq_set for cpu %u, freq %u kHz\n", policy->cpu, freq);

    mutex_lock(&setfreq_mutex);
    if (!per_cpu(cpu_is_managed, policy->cpu)) {
	printk(KERN_NOTICE "freq not set: cpu_is_managed for cpu %u = false\n", policy->cpu);
	goto err;
    }

    per_cpu(cpu_set_freq, policy->cpu) = freq;

    if (freq < per_cpu(cpu_min_freq, policy->cpu)) {
	freq = per_cpu(cpu_min_freq, policy->cpu);
    }
    if (freq > per_cpu(cpu_max_freq, policy->cpu)) {
	freq = per_cpu(cpu_max_freq, policy->cpu);
    }

    ret = __cpufreq_driver_target(policy, freq, CPUFREQ_RELATION_L);

err:
    mutex_unlock(&setfreq_mutex);
    return ret;
}
EXPORT_SYMBOL_GPL(LAMbS_cpufreq_set);
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
	mutex_lock(&setfreq_mutex);
	/* one processor only! */
	cpufreq_register_notifier(&lambs_cpufreq_notifier_block, 
				    CPUFREQ_TRANSITION_NOTIFIER);
	
	/* macros for setting policy per cpu. Probably not need for our nonSMP
	 * but maybe. */

	per_cpu(cpu_is_managed, cpu) = 1;
	per_cpu(cpu_min_freq, cpu) = policy->min;
	per_cpu(cpu_max_freq, cpu) = policy->max;
	per_cpu(cpu_cur_freq, cpu) = policy->cur;
	per_cpu(cpu_set_freq, cpu) = policy->cur;
	
	printk(KERN_NOTICE "managing cpu %u started (%u - %u kHz, currently %u kHz)\n",
	        cpu, per_cpu(cpu_min_freq, cpu), per_cpu(cpu_max_freq, cpu),
		per_cpu(cpu_cur_freq, cpu));
	mutex_unlock(&setfreq_mutex);
	
	/* get frequency table */
	
	break;
    case CPUFREQ_GOV_STOP:
	mutex_lock(&setfreq_mutex);
	/* one core only! */
	cpufreq_unregister_notifier(&lambs_cpufreq_notifier_block,
				    CPUFREQ_TRANSITION_NOTIFIER);
	per_cpu(cpu_is_managed, cpu) = 0;
	per_cpu(cpu_max_freq, cpu) = 0;
	per_cpu(cpu_min_freq, cpu) = 0;
	per_cpu(cpu_set_freq, cpu) = 0;
	printk(KERN_NOTICE "managing cpu %u stopped\n", cpu);
	mutex_unlock(&setfreq_mutex);
	break;
    case CPUFREQ_GOV_LIMITS:
	mutex_lock(&setfreq_mutex);
	printk(KERN_NOTICE "limit event for cpu %u: %u - %u kHz, currently %u kHz, "
		 "last set to %u kHz\n", cpu, policy->min, policy->max,
		 per_cpu(cpu_cur_freq, cpu), per_cpu(cpu_set_freq, cpu));
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

static int __init cpufreq_gov_lambs_init(void) {
    return cpufreq_register_governor(&cpufreq_gov_lambs);
}

static void __exit cpufreq_gov_lambs_exit(void) {
    cpufreq_unregister_governor(&cpufreq_gov_lambs);
}

MODULE_AUTHOR("Michael Giardino <giardino@ece.gatech.edu>");
MODULE_DESCRIPTION("CPUfreq policy governor for LAMbS");
MODULE_LICENSE("GPL");

module_init(cpufreq_gov_lambs_init);
module_exit(cpufreq_gov_lambs_exit);
