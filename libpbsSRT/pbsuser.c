#include <stdio.h>
/*
fprintf
stderr
*/
#include <fcntl.h>
/*
open
*/
#include <sys/ioctl.h>
/*
ioctl
*/

#include <stdlib.h>

#include <sys/types.h>

#include <sys/mman.h>
/*
mlock
*/

#include <unistd.h>
/*
read
fcntl
getpid
*/

#include <sched.h>

#include <stdint.h>

typedef uint64_t u64;
typedef uint32_t u32;
typedef int64_t  s64;
typedef int32_t  s32;

#include "pbsSRT_cmd.h"

#include "pbsuser.h"

int pbsSRT_setup(   uint64_t period, uint64_t estimated_mean_exectime, 
                    double alpha,
                    pbsSRT_predictor_t *predictor,
                    SRT_handle *handle, 
                    int loglevel, int logCount, char *logFileName)
{
    pid_t mypid;
    int min_priority;

    struct sched_param my_sched_params;

    int procfile;

    job_mgt_cmd_t cmd;

    int ret_val;

    mypid =  getpid();

/***************************************************************************/
//setup Linux scheduling parameters

    min_priority = sched_get_priority_min(SCHED_FIFO);
    /*Set the priority*/
    my_sched_params.sched_priority = min_priority;

    /*Try to change the scheduling policy and priority*/
    ret_val = sched_setscheduler(mypid, SCHED_FIFO, &my_sched_params);
    if(0 != ret_val)
    {
        perror("ERROR: pbs_SRT_setup");
        fprintf(stderr, "pbs_SRT_setup: Failed to set scheduling policy!\n");
        goto streight_exit;
    }

/***************************************************************************/
//check if logging is enabled and if so allocate memory for it

    handle->loglevel = loglevel;
    handle->job_count = 0;
    
    if(loglevel >= pbsSRT_LOGLEVEL_SUMMARY)
    {
        handle->log_file = fopen(logFileName, "w");
        if(handle->log_file == NULL)
        {
            perror("pbs_SRT_setup: Failed to open log file: ");
            ret_val = -1;
            goto streight_exit;
        }

        if(loglevel >= pbsSRT_LOGLEVEL_FULL)
        {

            handle->log = (struct SRT_job_log*)malloc(logCount*sizeof(struct SRT_job_log));
            if(handle->log == NULL)
            {
                perror("ERROR: ");
                fprintf(stderr, "pbs_SRT_setup: Failed to allocate memory for log!\n");
                ret_val = -1;
                goto lclose_exit;
            }

            ret_val = mlock(handle->log, (logCount*sizeof(struct SRT_job_log)));
            if(ret_val)
            {
                perror("pbs_SRT_setup: mlock failed for the log array");
                ret_val = -1;
                goto free_exit;
            }

            handle->pu_c0   = (int64_t*)calloc(logCount*4, sizeof(int64_t));
            if(NULL == handle->pu_c0)
            {
                perror("pbs_SRT_setup: calloc failed for the predictor output arrays");
                ret_val = -1;
                goto free_exit;
            }
            else
            {
                handle->pstd_c0 = &(handle->pu_c0[logCount]);
                handle->pu_cl   = &(handle->pstd_c0[logCount]);
                handle->pstd_cl =  &(handle->pu_cl[logCount]);
            }
            
            ret_val = mlock(handle->pu_c0, (logCount*4 * sizeof(int64_t)));
            if(ret_val)
            {
                perror("pbs_SRT_setup: mlock failed for the predictor output arrays");
                ret_val = -1;
                goto free2_exit;
            }
            handle->log_size = logCount;
        }/* if(loglevel > pbsSRT_LOGLEVEL_SUMMARY)*/
    }/* if(loglevel > pbsSRT_LOGLEVEL_NONE)*/

/***************************************************************************/
//regster with the pbs module and setup the pbs scheduling parameters

    //setup PBS scheduling parameters
    ret_val = open("/proc/sched_rt_jb_mgt",  O_RDWR);
    if(ret_val == -1)
    {
        perror("pbs_SRT_setup: Failed to open \"/proc/sched_rt_jb_mgt\":\n");
        goto free2_exit;
    }
    procfile = ret_val;

    cmd.cmd = PBS_JBMGT_CMD_SETUP;
    cmd.args[0] = period;
    ret_val = write(procfile, &cmd, sizeof(cmd));
    if(ret_val != sizeof(cmd))
    {
        perror("pbs_SRT_setup: write of a PBS_JBMGT_CMD_SETUP cmd failed!\n");
        goto close_exit;
    }
    
    handle->period          = period;
    handle->estimated_mean_exectime 
                            = estimated_mean_exectime;
    handle->alpha_squared   = alpha * alpha;

    cmd.cmd = PBS_JBMGT_CMD_START;
    ret_val = write(procfile, &cmd, sizeof(cmd));
    if(ret_val != sizeof(cmd))
    {
        perror("pbs_SRT_setup: write of a PBS_JBMGT_CMD_START cmd failed!\n");
        goto close_exit;
    }

    ret_val = 0;

/***************************************************************************/
//The execution-time predictor should have been already setup when passed
    handle->predictor = predictor;
    
    handle->procfile = procfile;
    return ret_val;

close_exit:
    close(procfile);
    handle->procfile = 0;
free2_exit:
    free(handle->pu_c0);
    handle->pu_c0   = NULL;
    handle->pstd_c0 = NULL;
    handle->pu_cl   = NULL;
    handle->pstd_cl = NULL;
free_exit:
    free(handle->log);
    handle->log = NULL;
lclose_exit:
    fclose(handle->log_file);
    handle->log_file = NULL;
streight_exit:
    return ret_val;
}

int pbsSRT_sleepTillFirstJob(SRT_handle *handle)
{
    int ret = 0;
    job_mgt_cmd_t cmd;

    /*The execution time of the first job is not yet known.
    There is no valid data to read.
    The predictor should not be invoked*/
    
    /*Setup the NEXTJOB command with the predicted execution time
    specified in command-line arguments and 0 variance*/
    cmd.cmd = PBS_JBMGT_CMD_NEXTJOB;    
    cmd.args[0] = handle->estimated_mean_exectime;
    cmd.args[1] = 0;
    cmd.args[2] = handle->estimated_mean_exectime;
    cmd.args[3] = 0;
    
    /*Issue the NEXTJOB command*/
    ret = write(handle->procfile, &cmd, sizeof(cmd));
    if(ret != sizeof(cmd))
    {
        perror("pbsSRT_waitTillFirstJob: write of a PBS_JBMGT_CMD_NEXTJOB cmd failed!\n");
        ret = -1;
    }
    else
    {
        ret = 0;
    }

    return ret;
}

//FIXME: Need to handle closes forced by the system,
//like when the allocator task closes and the SRT task is forced to close
int pbsSRT_sleepTillNextJob(SRT_handle *handle)
{
    int ret = 0;

    u64 runtime2;

    job_mgt_cmd_t cmd;

    int64_t u_c0, std_c0;
    int64_t u_cl, std_cl;

    /*Get various data such as execution time for the job that just completed*/
    if( (handle->loglevel >= pbsSRT_LOGLEVEL_FULL) && 
        (handle->job_count < handle->log_size))
    {
        /*Store the data in the log if logging is enabled.*/
        ret = read(handle->procfile, &(handle->log[handle->job_count]), sizeof(struct SRT_job_log));
        if(ret != sizeof(struct SRT_job_log))
        {
            ret = -1; 
            goto exit0;
        }
        
        runtime2 = (handle->log[handle->job_count]).runtime2;
    }
    else
    {
        /*Store just the runtime in a local variable if logging is disabled.*/
        ret = read(handle->procfile, &runtime2, sizeof(u64));
        if(ret != sizeof(u64))
        {
            ret = -1;
            goto exit0;
        }
        
    }

    /*Increment the job count*/
    (handle->job_count)++;

    /*Update the predictor and predict the execution time of the next job*/
    ret = handle->predictor->update(handle->predictor->state,
                                    runtime2,
                                    &u_c0, &std_c0,
                                    &u_cl, &std_cl);
    if(ret == -1)
    {
        /*If the predictor is not ready to produce valid output (still warming up)
        use the budget values specified in the command-line arguments*/
        u_c0    = handle->estimated_mean_exectime;
        std_c0  = 0;
        u_cl    = handle->estimated_mean_exectime;
        std_cl  = 0;
    }
    
    /*Issue the PBS_JBMGT_CMD_NEXTJOB command with the updated prediction*/
    cmd.cmd = PBS_JBMGT_CMD_NEXTJOB;
    cmd.args[0] = u_c0;
    cmd.args[1] = (int64_t)(handle->alpha_squared * (double)std_c0);
    cmd.args[2] = u_cl;
    cmd.args[3] = (int64_t)(handle->alpha_squared * (double)std_cl);
    ret = write(handle->procfile, &cmd, sizeof(cmd));
    if(ret != sizeof(cmd))
    {
        perror("pbs_begin_SRT_job: write of a PBS_JBMGT_CMD_NEXTJOB cmd failed!\n");
        return -1;
    }

    /*If "FULL" logging is enabled, log the predicted execution time and the estimated
    variance in the prediction error*/
    if( (handle->loglevel >= pbsSRT_LOGLEVEL_FULL) && 
        (handle->job_count < handle->log_size))
    {
        handle->pu_c0[handle->job_count-1] = cmd.args[0];
        handle->pstd_c0[handle->job_count-1] = cmd.args[1];
        handle->pu_cl[handle->job_count-1] = cmd.args[2];
        handle->pstd_cl[handle->job_count-1] = cmd.args[3];
    }
    
    ret = 0;

exit0:
    return ret;
}

void pbsSRT_close(SRT_handle *handle)
{
    int i;
    struct SRT_job_log *log_entry;

    job_mgt_cmd_t cmd;
    SRT_summary_t summary = {0, 0, 0, 0};
    int ret_val;
    
    int64_t relative_LFT;
    unsigned long miss;

    /*Stop adaptive budget allocation and enforcement for the task*/
    cmd.cmd = PBS_JBMGT_CMD_STOP;
    ret_val = write(handle->procfile, &cmd, sizeof(cmd));
    if(ret_val != sizeof(cmd))
    {
        perror("pbs_SRT_close: write of a PBS_JBMGT_CMD_STOP cmd failed!\n");
    }
    
    if(handle->loglevel >= pbsSRT_LOGLEVEL_SUMMARY)
    {
        cmd.cmd = PBS_JBMGT_CMD_GETSUMMARY;
        cmd.args[0] = (s64)&summary;
        ret_val = write(handle->procfile, &cmd, sizeof(cmd));
        if(ret_val != sizeof(cmd))
        {
            perror("pbs_SRT_close: write of a PBS_JBMGT_CMD_GETSUMMARY cmd failed!\n");
        }
        
        fprintf(handle->log_file,   "%i, %llu, %llu, %llu, %llu, %llu, %llu, %llu, "
                                    "0, 0, 0\n\n", getpid(),
                                    (unsigned long long)handle->period,
                                    (unsigned long long)handle->estimated_mean_exectime,
                                    (unsigned long long)handle->job_count,
                                    (unsigned long long)summary.cumulative_budget,
                                    (unsigned long long)summary.cumulative_budget_sat,
                                    (unsigned long long)summary.consumed_budget,
                                    (unsigned long long)summary.total_misses);

        if(handle->loglevel >= pbsSRT_LOGLEVEL_FULL)
        {

            for(i = 0; i < handle->job_count; i++)
            {
                log_entry = &(handle->log[i]);
                relative_LFT = log_entry->abs_LFT - log_entry->abs_releaseTime;
                miss = (relative_LFT > handle->period);
                fprintf(handle->log_file,   "%lu, %lu, %lu, "
                                            "%lu, %lu, %u, %u, %lu, %lu, %lu, %lu\n",
                                            (unsigned long)log_entry->runtime,
                                            (unsigned long)log_entry->runtime2,
                                            miss,
                                            (unsigned long)log_entry->abs_releaseTime,
                                            (unsigned long)log_entry->abs_LFT,
                                            log_entry->last_sp_compt_allocated,
                                            log_entry->last_sp_compt_used_sofar,
                                            (unsigned long)handle->pu_c0[i],
                                            (unsigned long)handle->pstd_c0[i],
                                            (unsigned long)handle->pu_cl[i],
                                            (unsigned long)handle->pstd_cl[i]);
            }

            fclose(handle->log_file);
            free(handle->log);
        }
    }
    
    close(handle->procfile);
}

