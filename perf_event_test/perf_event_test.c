#include <linux/init.h>
#include <linux/module.h>
#include <linux/ioctl.h>
#include <linux/perf_event.h>
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


#define MSR_PKG_ENERGY_STATUS 0x611
#define MSR_PKG_POWER_INFO 0x614
#define MSR_RAPL_POWER_UNIT 0x606

int LAMbS_cpufreq_set(struct cpufreq_policy *policy, unsigned int freq);

struct perf_event_attr pe_inst;
struct perf_event *event_inst;
struct cpufreq_policy *policy;

u64 count_inst;
u64 c=100000000;
u64 sum=0;
/*
struct perf_event_attr pe_power;
struct perf_event *event_power;
u64 count_power;
*/

static long long read_msr(unsigned int ecx) {
    unsigned int edx = 0, eax = 0;
    unsigned long long result = 0;
    __asm__ __volatile__("rdmsr" : "=a"(eax), "=d"(edx) : "c"(ecx));
    result = eax | (unsigned long long)edx << 0x20;
    printk(KERN_ALERT "read_msr() read 0x%016llx (0x%08x:0x%08x) from MSR 0x%08x\n", result, edx, eax, ecx);
    return result;
}

static int __init start_count(void) {
    /*int fd;*/
    long long i;
    u64 enabled = 0;
    u64 running = 0;
    int ret;
    unsigned int ecx = MSR_PKG_ENERGY_STATUS;
    long long energy;
    policy->min = 1200000;
    policy->max = 2300000;
    policy->governor = "lambs";

    printk(KERN_ALERT "perf_event_test loaded\n");
    /*
     get event_inst file descriptor 
    fd = get_unused_fd_flags(O_RDWR);
    
    if (fd < 0) {
	return fd;
    }
    */

    /* set size based upon current version of perf_event */
    memset(&pe_inst, 0, sizeof(struct perf_event_attr));
    /* instruction retired is a basic hardware type */
    pe_inst.type = PERF_TYPE_HARDWARE;
    pe_inst.size = sizeof(struct perf_event_attr);
    pe_inst.config = PERF_COUNT_HW_INSTRUCTIONS;
    /* should count be enabled when started? */
    pe_inst.disabled = 0;
    pe_inst.exclude_kernel = 0; /* should it be? */
    pe_inst.exclude_user = 1;
    pe_inst.exclude_hv = 1;
    pe_inst.pinned = 1;
    
    /*printk(KERN_ALERT "currently running on %d", smp_processor_id());*/
    event_inst = perf_event_create_kernel_counter(&pe_inst, 0, NULL, NULL, NULL);

    ret = LAMbS_cpufreq_set(policy, 1800000);
    if (!ret) {
        printk(KERN_NOTICE "frequency changed. Hopefully.\n");
    } else {
        printk(KERN_NOTICE "error ret value: %d\n",ret);
    }

    sum = c;
    for (i = 0; i < c; i++) {
	c--;
    }
    if (!event_inst) {
	printk(KERN_ERR "Cannot allocate event_inst");
    }

    count_inst = perf_event_read_value(event_inst,&enabled, &running);
    
    printk(KERN_ALERT "c= %lld inst= %lld\n", c, count_inst);

    energy = read_msr(ecx);
    energy = energy * 15.3;
    energy = energy / 1000000;
    printk(KERN_ALERT "energy: %lld J\n", energy);
    ecx = 0x606;
    energy = read_msr(ecx);


/*
    memset(&pe_power, 0, sizeof(struct perf_event_attr));
    pe_power.type 
*/
    /*perf_event_enable(event_inst);*/

    return 0;
}

static void __exit end_count(void) {
    u64 enabled = 0;
    u64 running = 0;
    /*perf_event_disable(event_inst);*/
    count_inst = perf_event_read_value(event_inst,&enabled, &running);
    printk(KERN_ALERT "reporting instructions\n");
    printk(KERN_ALERT "Used %lld instructions\n", count_inst);
    printk(KERN_ALERT "perf_event_test removed\n");
}


module_init(start_count);
module_exit(end_count);

MODULE_LICENSE("GPL");
