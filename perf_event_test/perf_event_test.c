#include <linux/init.h>
#include <linux/module.h>
#include <linux/ioctl.h>
#include <linux/perf_event.h>
#include <asm/unistd.h>
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
struct perf_event_attr pe_inst;
struct perf_event *event_inst;
u64 count_inst;
/*
struct perf_event_attr pe_power;
struct perf_event *event_power;
u64 count_power;
*/

static int __init start_count(void) {
    /*int fd;*/
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
    pe_inst.exclude_kernel = 1; /* should it be? */
    pe_inst.exclude_hv = 1;
    
    event_inst = perf_event_create_kernel_counter(&pe_inst, 0, NULL, NULL, NULL);

    if (!event_inst) {
	printk(KERN_ERR "Cannot allocate event_inst");
    }
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
    printk(KERN_ALERT "reporting instructions\n");
    count_inst = perf_event_read_value(event_inst,&enabled, &running);
    printk(KERN_ALERT "Used %lld instructions\n", count);
    printk(KERN_ALERT "perf_event_test removed\n");
}

module_init(start_count);
module_exit(end_count);

MODULE_LICENSE("GPL");
