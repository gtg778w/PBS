
#define PBS_JBMGT_CMD_SETUP     (0)
/*
    0) period (us)
*/

#define PBS_JBMGT_CMD_START     (1)
/*
*/

#define PBS_JBMGT_CMD_NEXTJOB   (2)
/*
    0) u_c1
    1) std_c1
    2) u_cl
    3) std_cl
*/

#define PBS_JBMGT_CMD_STOP      (3)
/*
*/

#define PBS_JBMGT_CMD_GETSUMMARY (4)
/*
    0) *SRT_summary_s
*/

#define PBS_JBMGT_CMD_MAX       (5)
#define PBS_JBMGT_CMD_MAXARGS   (4)
typedef struct job_mgt_cmd_s
{
    int             cmd;
    s64         args[4];
} job_mgt_cmd_t;

/*For each time-related varriable, a VIC-related varriable has been created*/
struct SRT_job_log
{
    u64 runtime2;
    u64 runVIC2;
    u64 runtime;
    u64 runVIC;
    u64 abs_releaseTime;
    u64 abs_LFT;

    u32 last_sp_compt_allocated;
    u32 last_sp_compt_used_sofar;
};

typedef struct SRT_summary_s
{
    /*The total budget allocated to this task since 
    the first scheduling period for this task.*/
    s64 cumulative_budget;
    
    /*Same as cumulative_budget, but after any saturation.*/
    s64 cumulative_budget_sat;

    /*Total comsumed budget (vs allocated budget)*/
    s64 consumed_budget;
    s64 consumed_VIC;

    /*The total number of jobs that missed the 
    corresponding deadline since the first job.*/
    s64 total_misses;
} SRT_summary_t;

