#ifndef PBS_BUDGET_NS_HELPER_INCLUDE
#define PBS_BUDGET_NS_HELPER_INCLUDE

#include <linux/kernel.h>
#include <linux/hrtimer.h>

struct pbs_budget_struct;

/*For each time-related variable, a VIC-based variable has been created*/
struct pbs_budget_ns_struct
{
    //bandwidth enforcement timer
    struct hrtimer  budget_enforcement_timer;

    //last task activation time
    u64 last_actv_time;

    //total runtime in the current scheduling period
    u64 total_rp_runtime;
    
    //the last job activation time
    u64 jb_actv_time;

    //total runtime of the current job
    u64 total_jb_runtime;
        
    //last job activation time for the second definition of job
    u64 jb_actv_time2;

    //total runtime of the current job for the second definition of job
    u64 total_jb_runtime2;
};

/*Get the runtime of the currently running job of the task*/
u64 pbs_budget_ns_get_jbusage1( struct pbs_budget_struct *budget_struct_p,
                                u64 now,    int is_sleeping);

u64 pbs_budget_ns_get_jbusage2( struct pbs_budget_struct *budget_struct_p,
                                u64 now,    int is_sleeping);

u64 pbs_budget_ns_get_rpusage(  struct pbs_budget_struct *budget_struct_p,
                                u64 now,    int is_sleeping);

void pbs_budget_ns_firstjob(struct pbs_budget_struct *budget_struct_p,
                            u64 now);
void pbs_budget_ns_jobboundary1(struct pbs_budget_struct *budget_struct_p,
                                u64 now);
void pbs_budget_ns_jobboundary2(struct pbs_budget_struct *budget_struct_p,
                                u64 now);

void budget_ns_BET_start(   struct pbs_budget_struct *budget_struct_p);
void pbs_budget_ns_cancel_BET(  struct pbs_budget_struct *budget_struct_p);

void pbs_budget_ns_schedin( struct pbs_budget_struct *budget_struct_p,
                            u64 now);
void pbs_budget_ns_schedout(struct pbs_budget_struct *budget_struct_p,
                            u64 now);
                                
void pbs_budget_ns_init(    struct pbs_budget_struct *budget_struct_p);
void pbs_budget_ns_uninit(  struct pbs_budget_struct *budget_struct_p);

/*Refresh the budget*/
#define pbs_budget_ns_refresh(budget_struct_p) \
    (budget_struct_p->budget_ns_struct).total_rp_runtime = 0;

#endif
