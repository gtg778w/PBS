#include <stdlib.h>
/*
strtoul
NULL
*/
#include <stdio.h>
/*
fprintf
stderr
*/
#include <stdint.h>
/*
int32_t
int64_t
uint32_t
uint64_t
*/

#include <string.h>
/*
memset
*/

#include <sys/mman.h>
/*
mlock
*/

#include "pbsAllocator.h"

struct SRT_record
{
	uint64_t 		SRT_budget;
    uint64_t        job_release_time;
	int64_t         u_c0;
	int64_t         var_c0;
	int64_t         u_cl;
	int64_t         var_cl;
	uint32_t 		pid;
	uint32_t 		SRT_runtime;
	uint32_t		sp_till_deadline;
	unsigned short 	SRT_qlength;
};

//the following variables are logging related
struct SRT_record   *SRT_record[4];
uint64_t	*sp_start_times;
int		    *allocator_runtimes;
int64_t     sp_limit;

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

	    memset(SRT_record[t], 0, sizeof(struct SRT_record)*sp_limit);

	    if(mlock(SRT_record[t], sizeof(struct SRT_record)*sp_limit) < 0)
	    {
		    perror("mlock failed for the SRT_record array");
		    return -1;
	    }
    }

    return 0;
}

void log_allocator_dat(long long sp_count)
{
    sp_start_times[sp_count] = 
        (unsigned long long)loaddata_list_header->prev_sp_boundary;
	allocator_runtimes[sp_count] = 
        (int)loaddata_list_header->last_allocator_runtime;
}

void log_SRT_sp_dat(int task_index,
                    long long sp_count,
                    SRT_loaddata_t	*SRT_loaddata_p,
                    uint64_t SRT_budget2)
{
    struct SRT_record * SRT_record_p = &((SRT_record[task_index])[sp_count]);

    SRT_record_p->SRT_runtime       = SRT_loaddata_p->current_runtime;
    SRT_record_p->SRT_qlength 	    = SRT_loaddata_p->queue_length;
    SRT_record_p->sp_till_deadline  = SRT_loaddata_p->sp_till_deadline;
    SRT_record_p->pid			    = SRT_loaddata_p->pid;
    SRT_record_p->job_release_time  = SRT_loaddata_p->job_release_time;
    SRT_record_p->u_c0              = SRT_loaddata_p->u_c0;
	SRT_record_p->var_c0            = SRT_loaddata_p->var_c0;
	SRT_record_p->u_cl              = SRT_loaddata_p->u_cl;
	SRT_record_p->var_cl            = SRT_loaddata_p->var_cl;
    SRT_record_p->SRT_budget        = allocation_array[task_index];
}

/************print and free memory for logging task computation times*********/
void free_log_memory(void)
{
    struct SRT_record *next_record;
    long long sp_count;
    int t;

    for(sp_count = 0; sp_count < sp_limit; sp_count++)
    {
        printf("\n%lli, %llu, %i, |", sp_count, 
                (unsigned long long)sp_start_times[sp_count], 
                allocator_runtimes[sp_count]);

        for(t = 1; t < 4; t++)
        {
            next_record = &((SRT_record[t])[sp_count]);
            printf("| %lu, %llu), %lu, [%lu, %u], {%llu, %lli, %lli, %lli}, (%lli) |",
                    (unsigned long)next_record->pid,
                    (unsigned long long)next_record->job_release_time,
                    (unsigned long)next_record->SRT_runtime,
                    (unsigned long)next_record->sp_till_deadline,
                    (unsigned)next_record->SRT_qlength,
                    (unsigned long long)next_record->u_c0,
                    (unsigned long long)next_record->var_c0,
                    (unsigned long long)next_record->u_cl,
                    (unsigned long long)next_record->var_cl,
                    (unsigned long long)next_record->SRT_budget);
        }
        printf("|\n");
    }

    munlockall();

    for(t = 1; t < 4; t++)
    {
        free(SRT_record[t]);
    }

    free(allocator_runtimes);

    free(sp_start_times);
}

