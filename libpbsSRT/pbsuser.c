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

#include "pbs_cmd.h"

#include "pbsuser.h"

int pbs_SRT_setup(  uint64_t period, uint64_t start_bandwidth, 
                    int history_length,
                    pbsSRT_predictor_t *predictor,
                    SRT_handle *handle, 
			        char Lflag, int logCount, char *logFileName)
{
	pid_t mypid;
	int	min_priority;

	struct sched_param my_sched_params;

	int procfile;

    job_mgt_cmd_t cmd;

	int ret_val;

	mypid =  getpid();

#ifdef VERBOSE_PBS

	printf("SRT task has pid: %i\n\n", mypid);
#endif

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

	if(Lflag == 1)
	{
		handle->log_file = fopen(logFileName, "w");
		if(handle->log_file == NULL)
		{
			perror("pbs_SRT_setup: Failed to open log file: ");
			ret_val = -1;
			goto streight_exit;
		}

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
		
		handle->log_index = 0;
		handle->log_size = logCount;
		handle->logging_enabled = 1;
	}
	else
	{
		handle->log_file = NULL;
		handle->log = NULL;
		handle->log_index = 0;
		handle->log_size = 0;
		handle->logging_enabled = 0;
	}

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

	ret_val = ioctl(procfile, PBS_IOCTL_JBMGT_PERIOD, period);
	if(ret_val)
	{
		perror("pbs_SRT_setup: ioctl with PBS_IOCTL_JBMGT_PERIOD failed!\n");
		goto close_exit;
	}
	handle->period = period;

    cmd.cmd = PBS_JBMGT_CMD_SETUP;
    cmd.args[0] = period;
    cmd.args[1] = (unsigned long)(2.0 * (double)(1 << 16));
    ret_val = write(procfile, &cmd, sizeof(cmd));
    if(ret_val != sizeof(cmd))
    {
        perror("pbs_SRT_setup: write of a PBS_JBMGT_CMD_SETUP cmd failed!\n");
        goto close_exit;
    }
    
	ret_val = ioctl(procfile, PBS_IOCTL_JBMGT_SRT_RUNTIME, start_bandwidth);
	if(ret_val) 
	{
		perror("pbs_SRT_setup: ioctl with PBS_IOCTL_JBMGT_SRT_RUNTIME failed!\n");
		goto close_exit;
	}
	handle->start_bandwidth = start_bandwidth;

	ret_val = ioctl(procfile, PBS_IOCTL_JBMGT_SRT_HISTLEN, history_length);
	if(ret_val)
	{
		perror("pbs_SRT_setup: ioctl with PBS_IOCTL_JBMGT_SRT_HISTLEN failed!\n");
		goto close_exit;
	}
	handle->history_length = history_length;

    /*FIXME should be able to remove this*/
	/*ret_val = ioctl(procfile, PBS_IOCTL_JBMGT_START, 0);
	if(ret_val)
	{
		perror("pbs_SRT_setup: ioctl with PBS_IOCTL_JBMGT_SRT_START failed!\n");
		goto close_exit;
	}*/

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

//FIXME: Need to handle closes forced by the system,
//like when the allocator task closes and the SRT task is forced to close
//FIXME: The structure of this function should be improved. e.g. should not be
//returning from 3 different places
//FIXME: Need to better handle initial conditions. Right now, the predictor is 
//initially updated with garbage.
int pbs_begin_SRT_job(SRT_handle *handle)
{
	struct SRT_job_log dummy;
	u64 runtime2;
	int ret;

    job_mgt_cmd_t cmd;
	int ret_val;

	if((handle->logging_enabled == 1) && (handle->log_index < handle->log_size))
	{
		ret = read(handle->procfile, &(handle->log[handle->log_index]), sizeof(struct SRT_job_log));
		if(ret != sizeof(struct SRT_job_log))
		{
			return -1;
		}
		
		runtime2 = (handle->log[handle->log_index]).runtime2;
		(handle->log_index)++;
	}
	else
	{
		ret = read(handle->procfile, &dummy, sizeof(struct SRT_job_log));
		if(ret != sizeof(struct SRT_job_log))
		{
			return -1;
		}
		
		runtime2 = dummy.runtime2;
	}

    /*Update the predictor and predict the execution time of the next job*/
    cmd.cmd = PBS_JBMGT_CMD_NEXTJOB;
    ret = handle->predictor->update(handle->predictor->state, 
                                    runtime2,
                                    &(cmd.args[0]), &(cmd.args[1]),
                                    &(cmd.args[2]), &(cmd.args[3]));
    if(ret == -1)
    {
        cmd.args[0] = handle->start_bandwidth;
        cmd.args[1] = 0;
        cmd.args[2] = handle->start_bandwidth;
        cmd.args[3] = 0;
    }

    if((handle->logging_enabled == 1) && (handle->log_index <= handle->log_size))
    {
        handle->pu_c0[handle->log_index-1] = cmd.args[0];
        handle->pstd_c0[handle->log_index-1] = cmd.args[1];
        handle->pu_cl[handle->log_index-1] = cmd.args[2];
        handle->pstd_cl[handle->log_index-1] = cmd.args[3];
    }
    
    /*Issue the command with the updated prediction*/
    ret_val = write(handle->procfile, &cmd, sizeof(cmd));
    if(ret_val != sizeof(cmd))
    {
        perror("pbs_begin_SRT_job: write of a PBS_JBMGT_CMD_NEXTJOB cmd failed!\n");
        return -1;
    }

	return 0;
}

void pbs_SRT_close(SRT_handle *handle)
{
	int i;
	struct SRT_job_log *log_entry;

    job_mgt_cmd_t cmd;
	int ret_val;

    /*Stop adaptive budget allocation and enforcement for the task*/
    cmd.cmd = PBS_JBMGT_CMD_STOP;
    ret_val = write(handle->procfile, &cmd, sizeof(cmd));
    if(ret_val != sizeof(cmd))
    {
        perror("pbs_SRT_close: write of a PBS_JBMGT_CMD_STOP cmd failed!\n");
    }
    
	close(handle->procfile);

	if(handle->logging_enabled == 1)	
	{
		fprintf(handle->log_file, "%i, %llu, %llu, %i, 0, 0\n",	getpid(),
											(unsigned long long)handle->period, 
											(unsigned long long)handle->start_bandwidth, 
											handle->history_length);

		for(i = 0; i < handle->log_index; i++)
		{
			log_entry = &(handle->log[i]);
			fprintf(handle->log_file, "%lu, %lu, %lu, %lu, %lu, %u, %u, %lu, %lu, %lu, %lu\n",
												(unsigned long)log_entry->runtime,
												(unsigned long)log_entry->runtime2,
                                                (unsigned long)log_entry->miss,
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

