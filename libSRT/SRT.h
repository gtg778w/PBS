#ifndef PBSSRT_INCLUDE
#define PBSSRT_INCLUDE

#include <stdint.h>
#include <stdio.h>

typedef void* (*SRT_Predictor_alloc_func_t)(void);

typedef void (*SRT_Predictor_init_func_t)( void    *state);

typedef int (*SRT_Predictor_update_func_t)(void    *state, int64_t exec_time,
                                    int64_t *pu_c0, int64_t *pvar_c0,
                                    int64_t *pu_cl, int64_t *pvar_cl);

typedef int (*SRT_Predictor_predict_func_t)(void   *state,
                                    int64_t *pu_c0, int64_t *pvar_c0,
                                    int64_t *pu_cl, int64_t *pvar_cl);

typedef void (*SRT_Predictor_free_func_t)( void    *state);

typedef struct SRT_Predictor_s
{
    void *state;
    SRT_Predictor_alloc_func_t     alloc;
    SRT_Predictor_init_func_t      init;
    SRT_Predictor_update_func_t    update;
    SRT_Predictor_predict_func_t   predict;
    SRT_Predictor_free_func_t      free;
} SRT_Predictor_t;

/*This is the user level header file for the PBS module for SRT tasks*/

enum {  SRT_LOGLEVEL_NONE,
        SRT_LOGLEVEL_SUMMARY,
        SRT_LOGLEVEL_FULL};

#define SRT_LOGLEVEL_MIN (SRT_LOGLEVEL_NONE)
#define SRT_LOGLEVEL_MAX (SRT_LOGLEVEL_FULL)

typedef struct SRT_handle_s
{
    int                 procfile;
    int                 budget_type;
    struct SRT_Predictor_s   *predictor;
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

int SRT_setup(  uint64_t period, uint64_t estimated_mean_exectime, 
                double alpha,
                struct SRT_Predictor_s  *predictor,
                SRT_handle *handle, 
                int loglevel, int logCount, char *logFileName);

int SRT_sleepTillFirstJob(SRT_handle *handle);
int SRT_sleepTillNextJob(SRT_handle *handle);

void SRT_close(SRT_handle *handle);

#endif
