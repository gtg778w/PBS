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
#include <unistd.h>
/*
read
fcntl
getpid
*/

#include <sched.h>

#include <stdint.h>

#include "pbsuser.h"

int pbs_SRT_setup(uint64_t period, uint64_t start_bandwidth, int history_length, SRT_handle *handle, 
			char Lflag, int logCount, char *logFileName)
{
	pid_t mypid;
	int	min_priority;

	struct sched_param my_sched_params;

	int procfile;

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
	if(ret_val = sched_setscheduler(mypid, SCHED_FIFO, &my_sched_params))
	{
		perror("ERROR: ");
		fprintf(stderr, "Failed to set scheduling policy!\n");		
		goto streight_exit;
	}

/***************************************************************************/
//check if logging is enabled and if so allocate memory for it

	if(Lflag == 1)
	{
		handle->log_file = fopen(logFileName, "w");
		if(handle->log_file == NULL)
		{
			perror("Failed to open log file: ");
			ret_val = -1;
			goto streight_exit;
		}

		handle->log = (struct SRT_job_log*)malloc(logCount*sizeof(struct SRT_job_log));
		if(handle->log == NULL)
		{
			perror("ERROR: ");
			fprintf(stderr, "Failed to allocate memory for log!\n");		
			ret_val = -1;
			goto lclose_exit;
		}

		ret_val = mlock(handle->log, (logCount*sizeof(struct SRT_job_log)));
		if(ret_val)
		{
			perror("mlock failed for the log array");
			ret_val = -1;
			goto free_exit;
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
	ret_val = open("/proc/sched_rt_jb_mgt", O_RDONLY);
	if(ret_val == -1)
	{
		perror("Failed to open \"/proc/sched_rt_jb_mgt\":\n");
		goto free_exit;
	}
	procfile = ret_val;

	ret_val = ioctl(procfile, PBS_IOCTL_JBMGT_PERIOD, period);
	if(ret_val)
	{
		perror("ioctl with PBS_IOCTL_JBMGT_PERIOD failed!\n");
		goto close_exit;
	}
	handle->period = period;

	ret_val = ioctl(procfile, PBS_IOCTL_JBMGT_SRT_RUNTIME, start_bandwidth);
	if(ret_val) 
	{
		perror("ioctl with PBS_IOCTL_JBMGT_SRT_RUNTIME failed!\n");
		goto close_exit;
	}
	handle->start_bandwidth = start_bandwidth;

	ret_val = ioctl(procfile, PBS_IOCTL_JBMGT_SRT_HISTLEN, history_length);
	if(ret_val)
	{
		perror("ioctl with PBS_IOCTL_JBMGT_SRT_HISTLEN failed!\n");
		goto close_exit;
	}
	handle->history_length = history_length;

	ret_val = ioctl(procfile, PBS_IOCTL_JBMGT_START, 0);
	if(ret_val)
	{
		perror("ioctl with PBS_IOCTL_JBMGT_SRT_START failed!\n");
		goto close_exit;
	}

	handle->procfile = procfile;
	return ret_val;

close_exit:
	close(procfile);
	handle->procfile = 0;
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
int pbs_begin_SRT_job(SRT_handle *handle)
{
	struct SRT_job_log dummy;
	int ret;

	if((handle->logging_enabled == 1) && (handle->log_index < handle->log_size))
	{
		ret = read(handle->procfile, &(handle->log[handle->log_index]), sizeof(struct SRT_job_log));
		if(ret != sizeof(struct SRT_job_log))
		{
			return -1;
		}
		(handle->log_index)++;
	}
	else
	{
		ret = read(handle->procfile, &dummy, sizeof(struct SRT_job_log));
		if(ret != sizeof(struct SRT_job_log))
		{
			return -1;
		}
	}

	return 0;
}

void pbs_SRT_close(SRT_handle *handle)
{
	int i;
	struct SRT_job_log *log_entry;

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
			fprintf(handle->log_file, "%lu, %lu, %lu, %lu, %u %u\n", 	
												(unsigned long)log_entry->runtime,
                                                (unsigned long)log_entry->miss,
                                                (unsigned long)log_entry->abs_releaseTime,
                                                (unsigned long)log_entry->abs_LFT,
                                                log_entry->last_sp_compt_allocated,
                                                log_entry->last_sp_compt_used_sofar);
		}

		fclose(handle->log_file);
		free(handle->log);
	}
}

