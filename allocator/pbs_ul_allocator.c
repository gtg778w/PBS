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
/*
strtoul
*/

#include <sys/types.h>
#include <unistd.h>
/*
read
getpid
*/

#include <sys/mman.h>
/*
memory mapping stuff
*/

#include <sched.h>

#include <string.h>
/*
strlen
*/

#include <errno.h>
/*
errno
*/

#include <math.h>
/*
sqrt
*/

#include "pbs.h"


struct SRT_record
{
	uint64_t 		SRT_budget;
    uint64_t        job_release_time;
	uint32_t 		pid;
	uint32_t 		SRT_runtime;
	uint32_t		sp_till_deadline;
	unsigned short 	SRT_qlength;
};

SRT_history_t		*history_array;
history_list_header_t* history_list_header;
uint64_t			*allocation_array;

//the following variables are logging related
struct SRT_record *SRT_record[4];
uint64_t	*sp_start_times;
int		*allocator_runtimes;

long long sp_limit;
double alpha;

/**************Allocate memory for logging task computation times**********/
int setup_log_memory(void)
{
	int	t;

    sp_start_times = (uint64_t*)malloc(sizeof(uint64_t)*sp_limit);
    if(sp_start_times == NULL)
    {
	    perror("malloc failed for the sp_limit array");		
	    return -1;
    }

    if(mlock(sp_start_times, (sizeof(uint64_t)*sp_limit)) < 0)
    {
	    perror("mlock failed for the sp_limit array");
	    return -1;
    }

    allocator_runtimes = (int*)malloc(sizeof(int)*sp_limit);
    if(allocator_runtimes == NULL)
    {
	    perror("malloc failed for the allocator_runtime array");
	    return -1;		
    }

    if(mlock(allocator_runtimes, (sizeof(int)*sp_limit)) < 0)
    {
	    perror("mlock failed for the allocator_runtime array");
	    return -1;
    }


    for(t = 1; t < 4; t++)
    {
	    SRT_record[t] = 
            (struct SRT_record*)malloc(sizeof(struct SRT_record)*sp_limit);
	    if(SRT_record[t] == NULL)
	    {
		    perror("malloc failed to allocate "
                   "space for the SRT_record array");
		    return -1;
	    }

	    bzero(SRT_record[t], sizeof(struct SRT_record)*sp_limit);

	    if(mlock(SRT_record[t], sizeof(struct SRT_record)*sp_limit) < 0)
	    {
		    perror("mlock failed for the SRT_record array");
		    return -1;
	    }
    }

    return 0;
}

static void inline log_allocator_dat(long long sp_count)
{
    sp_start_times[sp_count] = 
        (unsigned long long)history_list_header->prev_sp_boundary;
	allocator_runtimes[sp_count] = 
        (int)history_list_header->last_allocator_runtime;
}

static void inline log_SRT_sp_dat(  int task_index,
                                    long long sp_count,
                                    SRT_history_t	*SRT_history_p)
{
    struct SRT_record * SRT_record_p = &((SRT_record[task_index])[sp_count]);

    SRT_record_p->SRT_runtime        = SRT_history_p->current_runtime;
    SRT_record_p->SRT_qlength 	     = SRT_history_p->queue_length;
    SRT_record_p->sp_till_deadline   = SRT_history_p->sp_till_deadline;
    SRT_record_p->pid			     = SRT_history_p->pid;
    SRT_record_p->job_release_time   = SRT_history_p->job_release_time;
    SRT_record_p->SRT_budget         = allocation_array[task_index];
}

/************print and free memory for logging task computation times*********/
void free_log_memory(void)
{
    struct SRT_record *next_record;
    long long sp_count;
    int t;

    for(sp_count = 0; sp_count < sp_limit; sp_count++)
    {
        printf("%lli, %llu, %i, ", sp_count, 
                (unsigned long long)sp_start_times[sp_count], 
                allocator_runtimes[sp_count]);

        for(t = 1; t < 4; t++)
        {
            next_record = &((SRT_record[t])[sp_count]);
            printf("%lu, %llu, %lu, %lu, %u, %llu, ",	
                    (unsigned long)next_record->pid,
                    (unsigned long long)next_record->job_release_time,
                    (unsigned long)next_record->SRT_runtime,
                    (unsigned long)next_record->sp_till_deadline,
                    (unsigned)next_record->SRT_qlength,
                    (unsigned long long)next_record->SRT_budget);
        }
        printf("\n");
    }

    munlockall();

    for(t = 1; t < 4; t++)
    {
        free(SRT_record[t]);
    }

    free(allocator_runtimes);

    free(sp_start_times);
}

int allocator_setup(uint64_t scheduling_period,
                    uint64_t allocator_bandwidth)
{
    int proc_file;
    int retval;

	pid_t  mypid;
	struct sched_param my_sched_params;
	int min_priority;

    /*Need to change the scheduling policy of the task to real-time 
      (SCHED_FIFO)*/

	/*Determine own PID*/
	mypid =  getpid();

	/*Determine the lowest valid priority for the given scheduling policy*/
	min_priority = sched_get_priority_min(SCHED_FIFO);
	
	/*Set the priority*/
	my_sched_params.sched_priority = min_priority+1;

	/*Try to change the scheduling policy and priority*/
    retval = sched_setscheduler(mypid, SCHED_FIFO, &my_sched_params);
	if(retval)
	{
		perror("sched_setscheduler failed");
		return -1;
	}

    /*Perform setup operations needed to interact with the pbs_allocator
      module*/

	proc_file = open("/proc/sched_rt_bw_mgt", O_RDWR);
	if(proc_file == -1)
	{
		perror("Failed to open \"/proc/sched_rt_bw_mgt\"");
		return proc_file;
	}

	//setup the history mapping
	history_array = mmap(NULL, HISTLIST_SIZE, 
                         (PROT_READ), MAP_SHARED, 
                         proc_file, 0);
	if(history_array == MAP_FAILED)
	{
		perror("Failed to map history_array");
		return -1;
	}

	history_list_header = (history_list_header_t*)history_array;

	//setup the allocations mapping
	allocation_array = mmap(NULL, ALLOC_SIZE, 
                            (PROT_READ | PROT_WRITE), MAP_SHARED, 
                            proc_file, HISTLIST_SIZE);
	if(allocation_array == MAP_FAILED)
	{
		perror("Failed to map allocation_array");
		return -1;
	}

	retval = ioctl(proc_file, 0, scheduling_period);
	if(retval) 
    {
        perror("ioctl failed");
        fprintf(stderr, "Failed to set scheduling_period to %lu!\n", 
            scheduling_period);
        return retval;
    }

	retval = ioctl(proc_file, 1, allocator_bandwidth);
	if(retval)
    {
        perror("ioctl failed");
        fprintf(stderr, "Failed to set allocator_bandwidth to %lu!\n",
            allocator_bandwidth);
        return retval;
    }

	retval = ioctl(proc_file, 3, 0);
	if(retval)
    {
        perror("ioctl failed\n");
        fprintf(stderr, "Failed to issue ioctl command 3!\n");
        return retval;
    }

    return proc_file;
}

int allocator_close(int proc_file)
{
	if(munmap(history_array, HISTLIST_SIZE) != 0)
	{
		perror("Failed to unmap history pages!\n");
		return -1;
	}

	if(munmap(allocation_array, ALLOC_SIZE) != 0)
	{
		perror("Failed to unmap history pages!\n");
		return -1;
	}

	if(close(proc_file) != 0)
	{
		perror("Failed to close proc_file!\n");
		return -1;
	}

    return 0;
}

int compute_budget(SRT_history_t *history, uint64_t* budget)
{
	int64_t M2, delta, mean, variance, n;
	double  stdeviation;

	int64_t  offset;

	uint64_t est_tot_cmpt, est_cur_cmpt, est_rem_cmpt;
	int i;

	/*make sure there is sufficient history to compute bandwidth*/
	if( (history->buffer_index < 0) || 
        (history->history_length == 0))
	{
		return -1;
	}

	/*make sure there is computation remaining*/
	if(history->queue_length == 0)
	{
		*budget 		  = 0;
		return 0;
	}

	/*compute the statistics of the history of computation times*/
	n = 0;
	mean = 0;
	M2 = 0;
    
	for(i = 0; i < history->history_length; i++)
	{
		n = n+1;
		delta = history->history[i] - mean;
		mean = mean + (delta/n);
        /*The following expression uses the new value of mean*/
		M2 = M2 + delta*(history->history[i]-mean);
	}
	variance = M2/(n-1);
    
	stdeviation = sqrt((double)variance);

	/*compute the estimated copmutation time of the currently running job, 
	//assuming an exponential distribution with the statistics of
	//previously completed jobs*/

	offset = mean - (uint64_t)stdeviation;

	offset = (history->current_runtime > offset)? 
             history->current_runtime : offset;

	est_cur_cmpt = offset + (uint64_t)(stdeviation * alpha);
    
	/*compute the estimate of the total computation time for
	//all queued jobs and computation remaining*/
	est_tot_cmpt = est_cur_cmpt + (history->queue_length-1)*mean;

	est_rem_cmpt = est_tot_cmpt - history->current_runtime;
	*budget = (est_rem_cmpt / (history->sp_till_deadline));
    
	return 0;
}

void allocator_loop_wlogging(int proc_file)
{
    //enter the main scheduling loop
    long long sp_count = 0;

    int retval;
    int t, task_count, task_index;

    char buffer[10];

    SRT_history_t *next;

    uint64_t budget;

    while(sp_count < sp_limit)
    {
        retval = read(proc_file, buffer, 1);
        if(retval == -1)
        {
            perror("read from pbs_bw_mgt failed:");
            break;
        }

        sp_count++;
        log_allocator_dat(sp_count);

        next = &(history_array[history_list_header->first]);
        task_count = history_list_header->SRT_count;

		for(t = 0; t < task_count; t++)		
		{
			task_index = next - history_array;

            retval = compute_budget(next, &budget);
		    if(retval == 0)
		    {
			    allocation_array[task_index] = budget;
		    }

            log_SRT_sp_dat(task_index, sp_count, next);

			next = &(history_array[next->next]);
		}

	}
    
}

void allocator_loop(int proc_file)
{
    //enter the main scheduling loop
    long long sp_count = 0;
    unsigned char loop_condition = 1;

    int retval;
    int t, task_count, task_index;

    char buffer[10];

    SRT_history_t *next;

    uint64_t budget;

    while(loop_condition)
    {
        retval = read(proc_file, buffer, 1);
        if(retval == -1)
        {
            perror("read from pbs_bw_mgt failed:");
            break;
        }

        next = &(history_array[history_list_header->first]);
        task_count = history_list_header->SRT_count;

		for(t = 0; t < task_count; t++)		
		{
			task_index = next - history_array;

            retval = compute_budget(next, &budget);
		    if(retval == 0)
		    {
			    allocation_array[task_index] = budget;
		    }

            next = &(history_array[next->next]);
		}

        if(sp_limit > 0)
        {
            sp_count++;
            loop_condition = (sp_count < sp_limit);
        }
	}
}

char *options_string = \
"\n\t-f:\trun without prompting\n"\
"\t-a:\tthe PBS alpha parameter (2.0)\n"\
"\t-s:\tthe number of scheduling periods (1)\n"\
"\t-S:\tdo not keep or output a log\n";

int main(int argc, char** argv)
{
	int retval;

    int proc_file;

	/*variables for parsing input arguments*/
	unsigned char	aflag = 0, sflag = 0, fflag=0, Sflag = 0;


	alpha = 2.0;
	sp_limit = 1;

    while((retval = getopt(argc, argv, "fa:s:S")) != -1)
    {
        switch(retval)
        {
            case 'f':
                fflag = 1;
                break;
            case 'a':
                aflag = 1;
                alpha = strtod(optarg, NULL);
                if(alpha < 0)
                {
                    fprintf(stderr, 
                            "Bad alpha value (%f), should be positive!\n",
                            alpha);
                    return -EINVAL;
                }
                break;
            case 's':
                sflag = 1;
                sp_limit = strtoul(optarg, NULL, 10);
                if(sp_limit < 0)
                {
                    fprintf(stderr, 
                            "Bad sp_limit value (%lli), "
                            "should be positive or 0(infinite)!\n", sp_limit);
                    return -EINVAL;
                }
                break;
            case 'S':
                Sflag = 1;
                break;
            default:
                fprintf(stderr, "Usage: %s [Options]\n%s\n", 
                    argv[0], options_string);
                return -EINVAL;
        }
    }

	/*Make sure the number of arguments make sense*/
	if(optind != argc)
	{
		fprintf(stderr, "Usage: %s [Options]\n%s\n", argv[0], options_string);
		return -1;
	}

    if((sp_limit == 0) && (Sflag == 0))
    {
		fprintf(stderr, 
            "WARNING: The number of scheduling periods is not bounded!" 
            "Logging is automatically disabled!\n");
        Sflag = 1;
    }

    fprintf(stderr, "Options: alpha = %f", alpha);

    if(sp_limit == 0)
        fprintf(stderr, ", count = infinity");
    else
        fprintf(stderr, ", count = %lli", sp_limit);

    if(Sflag == 0)
        fprintf(stderr, ", logging to stdout enabled.\n");
    else
        fprintf(stderr, ", logging to stdout disabled.\n");

	if(fflag == 0)
	{
        fprintf(stderr, "Waiting for prompt from user ...\n");
		if(fgetc(stdin) == (int)'q')
		{
			fprintf(stderr, "\nExiting...\n");
			return 0;
		}
        fprintf(stderr, "\nRunning...\n");
	}

    //This should only be done if logging is enabled
    if(Sflag == 0)
    {
	    retval = setup_log_memory();
        if(retval)
        {
            fprintf(stderr, "setup_log_memory failed!\n");
            exit(EXIT_FAILURE);
        }
    }

    //Setup the interface with the pbs_allocator module
    //and scheduling related parameters
    proc_file = allocator_setup(10000, 1000);
    if(proc_file < 0)
    {
        fprintf(stderr, "allocator_setup failed!");
        exit(EXIT_FAILURE);
    }

    if(Sflag == 0)
    {
        allocator_loop_wlogging(proc_file);
    }
    else
    {
        allocator_loop(proc_file);
    }
	
    retval = allocator_close(proc_file);
    if(retval)
    {
        fprintf(stderr, "allocator_close failed!");
        exit(EXIT_FAILURE);
    }

    if(Sflag == 0)
    {
        free_log_memory();
    }

	printf("\n");
	return 0;
}

