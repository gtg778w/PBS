#include <linux/init.h>
#include <linux/module.h>
#include <asm/unistd.h>
#include <linux/cpufreq.h>

/*
long perf_event_open(struct perf_event_attr *hw_event, 
		    pid_t pid, 
		    int cpu,
		    int group_fd,
		    unsigned long flags) {
    int ret;
    ret = sysl(__NR_perf_event_open, hw_event, pid, cpu, group_fd, flags);
    return ret;
}*/


int LAMbS_cpufreq_set(struct cpufreq_policy *policy, unsigned int freq);

struct cpufreq_policy policy;

static int __init change_freq(void) {
    int ret = 0;
    
    ret = cpufreq_get_policy(&policy, 0);

    if (!ret) {
	printk(KERN_NOTICE "Got cpufreq_policy");
    } else {
	printk(KERN_NOTICE "ERROR: didn't get cpufreq_policy: %d\n", ret);
	goto err;
    }

    printk(KERN_ALERT "perf_event_test loaded\n");

    ret = LAMbS_cpufreq_set(&policy, 1000000);
    if (!ret) {
        printk(KERN_NOTICE "frequency changed. Hopefully.\n");
    } else {
        goto err;
	printk(KERN_NOTICE "error ret value: %d\n",ret);
    }

/*
    memset(&pe_power, 0, sizeof(struct perf_event_attr));
    pe_power.type 
*/
    /*perf_event_enable(event_inst);*/
err:
    
    return ret;
}

static void __exit end_count(void) {
}


module_init(change_freq);
module_exit(end_count);

MODULE_LICENSE("GPL");
