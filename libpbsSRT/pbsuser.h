#ifndef PBSSRT_INCLUDE
#define PBSSRT_INCLUDE

#include <stdint.h>
#include <stdio.h>

struct libPredictor_s;

/*This is the user level header file for the PBS module for SRT tasks*/

enum {  pbsSRT_LOGLEVEL_NONE,
        pbsSRT_LOGLEVEL_SUMMARY,
        pbsSRT_LOGLEVEL_FULL};

#define pbsSRT_LOGLEVEL_MIN (pbsSRT_LOGLEVEL_NONE)
#define pbsSRT_LOGLEVEL_MAX (pbsSRT_LOGLEVEL_FULL)

enum pbs_budget_type;

typedef struct SRT_handle_s
{
    int                 procfile;
    int                 budget_type;
    struct libPredictor_s   *predictor;
    uint64_t            period;
    uint64_t            reservation_period;

    uint64_t            estimated_mean_exectime;
    uint32_t            alpha_fp;

    FILE                *log_file;
    struct SRT_job_log  *log;

    int64_t             job_count;
    int64_t             *pu_c0;
    int64_t             *pvar_c0;
    int64_t             *pu_cl;
    int64_t             *pvar_cl;
    int                 log_size;
    int                 loglevel;

} SRT_handle;

int pbsSRT_setup(   uint64_t period, uint64_t estimated_mean_exectime, 
                    double alpha,
                    struct libPredictor_s   *predictor,
                    SRT_handle *handle, 
                    int loglevel, int logCount, char *logFileName);

int pbsSRT_sleepTillFirstJob(SRT_handle *handle);
int pbsSRT_sleepTillNextJob(SRT_handle *handle);

void pbsSRT_close(SRT_handle *handle);

#endif
