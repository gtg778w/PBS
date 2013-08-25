#ifndef PBS_BUDGET_VIC_HELPER_INCLUDE
#define PBS_BUDGET_VIC_HELPER_INCLUDE

#include <linux/kernel.h>
#include "LAMbS_VICtimer.h"

struct pbs_budget_struct;

/*For each time-related variable, a VIC-based variable has been created*/
struct pbs_budget_VIC_struct
{
    //bandwidth enforcement timer
    struct LAMbS_VICtimer_s budget_enforcement_timer;

    //last task activation time
    u64 last_actv_VIC;

    //total runtime in the current scheduling period
    u64 total_rp_VIC;
    
    //the last job activation time
    u64 jb_actv_VIC;

    //total runtime of the current job
    u64 total_jb_VIC;
        
    //last job activation time for the second definition of job
    u64 jb_actv_VIC2;

    //total runtime of the current job for the second definition of job
    u64 total_jb_VIC2;
};

u64 pbs_budget_VIC_get_jbusage1(struct pbs_budget_struct *budget_struct_p,
                                u64 now_VIC,    int is_sleeping);

u64 pbs_budget_VIC_get_jbusage2(struct pbs_budget_struct *budget_struct_p,
                                u64 now_VIC,    int is_sleeping);

u64 pbs_budget_VIC_get_rpusage( struct pbs_budget_struct *budget_struct_p,
                                u64 now_VIC,    int is_sleeping);


void pbs_budget_VIC_firstjob(   struct pbs_budget_struct *budget_struct_p,
                                u64 now_VIC);

void pbs_budget_VIC_jobboundary1(   struct pbs_budget_struct *budget_struct_p,
                                    u64 now_VIC);

void pbs_budget_VIC_jobboundary2(   struct pbs_budget_struct *budget_struct_p,
                                    u64 now_VIC);


#define pbs_budget_VIC_refresh(budget_struct_p) \
    (budget_struct_p->budget_VIC_struct).total_rp_VIC = 0;


void pbs_budget_VIC_BET_start(  struct pbs_budget_struct *budget_struct_p);

void pbs_budget_VIC_BET_cancel( struct pbs_budget_struct *budget_struct_p);


void pbs_budget_VIC_schedin(struct pbs_budget_struct *budget_struct_p,
                            u64 now_VIC);

void pbs_budget_VIC_schedout(   struct pbs_budget_struct *budget_struct_p,
                                u64 now_VIC);
                                

void pbs_budget_VIC_init(   struct pbs_budget_struct *budget_struct_p);

void pbs_budget_VIC_uninit( struct pbs_budget_struct *budget_struct_p);


#endif
