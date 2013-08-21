#include "bw_mgt.h"
#include "pbs_budget.h"

#include "LAMbS_VIC.h"
/*
    LAMbS_clock
    LAMbS_VIC_get
*/

#include "LAMbS_VICtimer.h"
/*
    LAMbS_VICtimer
*/

#define PBA_IS_THROTTLED(pba_struct_p) (pba_struct_p->flags & PBA_THROTTLED)
#define PBA_IS_SLEEPING(pba_struct_p)  (pba_struct_p->flags & PBA_SLEEPING)

void pba_init(struct SRT_struct *SRT_struct_p)
{
    struct pba_struct *pba_struct_p;

    pba_struct_p = &(SRT_struct_p->pba_struct);

    //initialize the budget enforcement VICtimer
    LAMbS_VICtimer_init(    &(pba_struct_p->VICtimer));
    (pba_struct_p->VICtimer).function = pbs_budget_enforcement_VICtimer_callback;

    //initialize the preempt notifier
    preempt_notifier_init(  &(pba_struct_p->pin_notifier), 
                            &pba_pops);
    hlist_add_head( &(pba_struct_p->pin_notifier.link), 
                    &(SRT_struct_p->task->preempt_notifiers));

    printk(KERN_INFO "pba_init called %d", SRT_struct_p->task->pid);
}


void pba_uninit(struct SRT_struct *SRT_struct_p)
{
    //uninitialize the the preempt_notifier
    hlist_del(&(SRT_struct_p->pba_struct.pin_notifier.link));

    //cancel the bandwidth enforcement timer if it is active
    hrtimer_cancel(&(SRT_struct_p->pba_struct.hrtimer));

    printk(KERN_INFO "pba_uninit called %d", SRT_struct_p->task->pid);

    printk(KERN_INFO "The task overused its allocation by more than %dns," 
                    " %d times. Max overuse: %llu\n",
                        THROTTLE_THREASHOLD_NS,
                        SRT_struct_p->overuse_count,
                        (unsigned long long)SRT_struct_p->maximum_overuse);
}

