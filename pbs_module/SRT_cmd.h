#ifndef SRT_CMD_INCLUDE
#define SRT_CMD_INCLUDE

#include "Common_cmd.h"

#define SRT_CMD_SETUP     (0)
/*
    0) period (us)
    1) &budget_type (output varriable, enum type)
    2) &reservation_period (output varriable, u64 type)
*/

#define SRT_CMD_START     (1)
/*
*/

#define SRT_CMD_NEXTJOB   (2)
/*
    0) u_c0
    1) var_c0
    2) u_cl
    3) var_cl
    4) alpha_fp //a 32bit fixed-point number with 16 fractional bits
*/

#define SRT_CMD_STOP      (3)
/*
*/

#define SRT_CMD_GETSUMMARY (4)
/*
    0) *SRT_summary_s
*/

#define SRT_CMD_MAX       (5)
#define SRT_CMD_MAXARGS   (4)
typedef struct job_mgt_cmd_s
{
    int             cmd;
    s64         args[5];         
} job_mgt_cmd_t;

/*For each time-related varriable, a VIC-related varriable has been created*/
struct SRT_job_log
{
    u64 runtime2;
    u64 runVIC2;
    u64 abs_releaseTime;
    u64 abs_LFT;

    u32 last_sp_budget_allocated;
    u32 last_sp_budget_used_sofar;
};

typedef struct SRT_summary_s
{
    /*The total budget allocated to this task since 
    the first scheduling period for this task.*/
    s64 allocated_budget;
    
    /*Total comsumed budget (vs allocated budget)*/
    s64 consumed_budget_ns;
    s64 consumed_budget_VIC;

    /*The total number of jobs that missed the 
    corresponding deadline since the first job.*/
    s64 total_misses;

    /*Total available CPU capacity*/
    s64 total_CPU_budget_capacity;
    
} SRT_summary_t;

#endif
