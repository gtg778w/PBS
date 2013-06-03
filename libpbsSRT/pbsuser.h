#ifndef PBSSRT_INCLUDE
#define PBSSRT_INCLUDE

#include <stdint.h>
#include <stdio.h>

#include "pbsSRT_predictor.h"

/*This is the user level header file for the PBS module for SRT tasks*/

enum {  pbsSRT_LOGLEVEL_NONE,
        pbsSRT_LOGLEVEL_SUMMARY,
        pbsSRT_LOGLEVEL_FULL};

#define pbsSRT_LOGLEVEL_MIN (pbsSRT_LOGLEVEL_NONE)
#define pbsSRT_LOGLEVEL_MAX (pbsSRT_LOGLEVEL_FULL)

typedef struct SRT_handle_s
{
    int                 procfile;
    pbsSRT_predictor_t  *predictor;
    uint64_t            period;
    uint64_t            start_bandwidth;
    double              alpha_squared;

    FILE                *log_file;
    struct SRT_job_log  *log;
    /*FIXME: remove these buffers to reduce overhead*/
    int64_t             *pu_c0;
    int64_t             *pstd_c0;
    int64_t             *pu_cl;
    int64_t             *pstd_cl;
    int64_t             job_count;
    int                 log_size;
    int                 loglevel;

} SRT_handle;

int pbsSRT_setup(   uint64_t period, uint64_t start_bandwidth, 
                    double alpha,
                    pbsSRT_predictor_t *predictor,
                    SRT_handle *handle, 
                    int loglevel, int logCount, char *logFileName);

int pbsSRT_sleepTillFirstJob(SRT_handle *handle);
int pbsSRT_sleepTillNextJob(SRT_handle *handle);

void pbsSRT_close(SRT_handle *handle);

#endif
