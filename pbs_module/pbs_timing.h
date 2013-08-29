#ifndef PBS_TIMING_INCLUDE
#define PBS_TIMING_INCLUDE

#include <linux/kernel.h>
/* 
printk() 
*/

#include <linux/errno.h>
/* 
error codes 
*/

#include <linux/ktime.h>
/*
ktime_t
*/

#include <linux/time.h>
/*
timespec
*/

#include <linux/hrtimer.h>
/*
hrtimer
*/

#include <linux/list.h>
/*
all linked list related stuff
*/

#include <linux/sched.h>
/*
schedule
*/

extern ktime_t  scheduling_period_ns;
extern s64      allocator_runtime_ns;
extern s64      saturation_level;

#define INIT_TIMING_STRUCT(pts) \
do{\
    INIT_LIST_HEAD(&(pts->timing_list_entry));\
    pts->next_task_period_boundary.tv64 = 0;\
    pts->task_period = scheduling_period_ns;\
}while(0)

struct SRT_timing_struct
{
    struct list_head    timing_list_entry;
    ktime_t     next_task_period_boundary;
    ktime_t     task_period;
};


int first_sleep_till_next_period(struct SRT_timing_struct *tq_entry);
int sleep_till_next_period(struct SRT_timing_struct *tq_entry);
void remove_from_timing_queue(struct SRT_timing_struct *tq_entry);

int setup_allocator(s64 period, s64 runtime);
int start_pbs_timing(void);
void assign_budgets(void);
void first_sched_period_tick(void);
void sched_period_tick(void);
int stop_pbs_timing(char not_allocator);

#endif //#ifdef PBS_TIMING_INCLUDE

