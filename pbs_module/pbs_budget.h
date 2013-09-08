#ifndef PBS_BUDGET_INCLUDE
#define PBS_BUDGET_INCLUDE

#include <linux/kernel.h>

#include "pbsCommon_cmd.h"

#include "pbs_budget_ns_helper.h"
#include "pbs_budget_VIC_helper.h"

struct SRT_struct;

struct pbs_budget_struct
{
    struct preempt_notifier         pin_notifier;
    
    struct pbs_budget_ns_struct     budget_ns_struct;
    
    struct pbs_budget_VIC_struct    budget_VIC_struct;
    
    //the budget allocated in a scheduling period
    u64 sp_budget;

    u16 flags;
};

//the following are bits in the flags field 
//of the above data structure
#define PBS_BUDGET_SLEEPING     (0x1)
#define PBS_BUDGET_THROTTLED    (0x2)
#define PBS_BUDGET_JOBCOMPLETE  (0x4)

int pbs_budget_settype(enum pbs_budget_type budget_type);
enum pbs_budget_type pbs_budget_gettype(void);

void    init_CPUusage_statistics(void);
void    update_CPUusage_statistics(void);

void pbs_budget_firstjob(struct SRT_struct *ss);
void pbs_budget_jobboundary1(struct SRT_struct *ss);
void pbs_budget_jobboundary2(struct SRT_struct *ss);

void pbs_budget_refresh(struct SRT_struct *ss);

#define pbs_budget_set(SRT_struct_p, budget) \
(SRT_struct_p->budget_struct.sp_budget = budget)

void pbs_budget_init(struct SRT_struct *ss);
void pbs_budget_uninit(struct SRT_struct *ss);

#endif
