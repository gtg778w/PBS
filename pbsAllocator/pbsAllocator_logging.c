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

struct SRT_record_s
{
    uint64_t        SRT_budget;
    uint64_t        job_release_time;
    int64_t         u_c0;
    int64_t         var_c0;
    int64_t         u_cl;
    int64_t         var_cl;
    uint32_t        pid;
    uint32_t        SRT_runtime;
    uint32_t        sp_till_deadline;
    unsigned short  SRT_qlength;
};

//the following variables are logging related
#define ALLOCATOR_RECORD_COUNT (4)
struct SRT_record_s   *SRT_record[ALLOCATOR_RECORD_COUNT];

#define ALLOCATOR_LOG_MOI_MAX (16)
struct allocator_record_s
{
    uint64_t    sp_start_time;      //8
    uint64_t    allocator_runtime;  //8
    uint64_t    energy_last_sp;     //8
    uint64_t    energy_total;       //8
    uint64_t    icount_last_sp;     //8
    double      estimated_icount;   //8
    double      estimated_energy;   //8
    uint64_t    mocount;            //8
    uint64_t    mostat[ALLOCATOR_LOG_MOI_MAX]; //128
    double      perf_coeffs[ALLOCATOR_LOG_MOI_MAX];//128
    double      powr_coeffs[ALLOCATOR_LOG_MOI_MAX];//128
};

struct allocator_record_s *allocator_record;
int64_t     sp_limit;

/**************Allocate memory for logging task computation times**********/
int setup_log_memory(void)
{
    int t;

    allocator_record = 
        (struct allocator_record_s*)malloc(sizeof(struct allocator_record_s)*sp_limit);
    if(NULL == allocator_record)
    {
        perror("malloc failed for the allocator_record array");
        return -1;
    }

    if(mlock(allocator_record, (sizeof(struct allocator_record_s)*sp_limit)) < 0)
    {
        perror("mlock failed for the allocator_record array");
        return -1;
    }
    
    memset(allocator_record, 0, (sizeof(struct allocator_record_s)*sp_limit));

    for(t = 0; t < ALLOCATOR_RECORD_COUNT; t++)
    {
        SRT_record[t] = 
            (struct SRT_record_s*)malloc(sizeof(struct SRT_record_s)*sp_limit);
        if(SRT_record[t] == NULL)
        {
            perror("malloc failed to allocate "
                   "space for the SRT_record array");
            return -1;
        }

        memset(SRT_record[t], 0, sizeof(struct SRT_record_s)*sp_limit);

        if(mlock(SRT_record[t], sizeof(struct SRT_record_s)*sp_limit) < 0)
        {
            perror("mlock failed for the SRT_record array");
            return -1;
        }
    }

    return 0;
}

void log_allocator_dat( long long sp_count, 
                        double est_icount,
                        double est_energy)
{
    int moi, mocount;
    
    allocator_record[sp_count].sp_start_time
                        = loaddata_list_header->prev_sp_boundary;
    allocator_record[sp_count].allocator_runtime 
                        = loaddata_list_header->last_allocator_runtime;

    allocator_record[sp_count].energy_last_sp
                        = loaddata_list_header->energy_last_sp;
    allocator_record[sp_count].energy_total
                        = loaddata_list_header->energy_total;
    allocator_record[sp_count].icount_last_sp
                        = loaddata_list_header->icount_last_sp;
    
    allocator_record[sp_count].estimated_icount = est_icount;
    allocator_record[sp_count].estimated_energy = est_energy;
    
    allocator_record[sp_count].mocount
                        = loaddata_list_header->mo_count;
    
    mocount = allocator_record[sp_count].mocount;
    mocount = (mocount > ALLOCATOR_LOG_MOI_MAX)? ALLOCATOR_LOG_MOI_MAX : mocount;
    for(moi = 0; moi < mocount; moi++)
    {
        allocator_record[sp_count].mostat[moi]
                        = loaddata_list_header->mostat_last_sp[moi];
        allocator_record[sp_count].perf_coeffs[moi] 
                        = perf_model_coeffs_double[moi];
        allocator_record[sp_count].powr_coeffs[moi] 
                        = power_model_coeffs_double[moi];
    }
}

void log_SRT_sp_dat(int task_index,
                    long long sp_count,
                    SRT_loaddata_t  *SRT_loaddata_p,
                    uint64_t SRT_budget2)
{
    if(task_index < ALLOCATOR_RECORD_COUNT)
    {
        struct SRT_record_s * SRT_record_p = &((SRT_record[task_index])[sp_count]);

        SRT_record_p->SRT_runtime       = SRT_loaddata_p->current_runtime;
        SRT_record_p->SRT_qlength       = SRT_loaddata_p->queue_length;
        SRT_record_p->sp_till_deadline  = SRT_loaddata_p->sp_till_deadline;
        SRT_record_p->pid               = SRT_loaddata_p->pid;
        SRT_record_p->job_release_time  = SRT_loaddata_p->job_release_time;
        SRT_record_p->u_c0              = SRT_loaddata_p->u_c0;
        SRT_record_p->var_c0            = SRT_loaddata_p->var_c0;
        SRT_record_p->u_cl              = SRT_loaddata_p->u_cl;
        SRT_record_p->var_cl            = SRT_loaddata_p->var_cl;
        SRT_record_p->SRT_budget        = allocation_array[task_index];
    }
}

/************print and free memory for logging task computation times*********/
void free_log_memory(void)
{
    struct SRT_record_s *next_record;
    long long sp_count;
    int moi, mocount;
    int t;

    for(sp_count = 0; sp_count < sp_limit; sp_count++)
    {
        printf( "\n%llu\t)%llu>, %lluns, %lluinst, [%e inst est] "
                "%lluuJ, [%e uJ est] , %lluuJ total\n", 
                (long long unsigned int)sp_count,
                (long long unsigned int)allocator_record[sp_count].sp_start_time, 
                (long long unsigned int)allocator_record[sp_count].allocator_runtime,
                (long long unsigned int)allocator_record[sp_count].icount_last_sp,
                                        allocator_record[sp_count].estimated_icount,
                (long long unsigned int)allocator_record[sp_count].energy_last_sp,
                                        allocator_record[sp_count].estimated_energy,
                (long long unsigned int)allocator_record[sp_count].energy_total);
        
        mocount = allocator_record[sp_count].mocount;
        printf("\t[%llu] { ", (long long unsigned int)mocount);
        mocount = (mocount > ALLOCATOR_LOG_MOI_MAX)? ALLOCATOR_LOG_MOI_MAX : mocount;
        for(moi = 0; moi < (mocount-1); moi++)
        {
            printf( "%llu, ", 
                    (long long unsigned int)allocator_record[sp_count].mostat[moi]);
        }
        if(mocount > 0)
        {
            printf( "%llu }\n", 
                    (long long unsigned int)allocator_record[sp_count].mostat[moi]);
        }
        else
        {
            printf("}\n");
        }
        printf("\tperf\t{ ");
        for(moi = 0; moi < (mocount-1); moi++)
        {
            printf( "%e, ", allocator_record[sp_count].perf_coeffs[moi]);
        }
        if(mocount > 0)
        {
            printf( "%e }\n", allocator_record[sp_count].perf_coeffs[moi]);
        }
        else
        {
            printf("}\n");
        }
        printf("\tpower\t{ ");
        for(moi = 0; moi < (mocount-1); moi++)
        {
            printf( "%e, ", allocator_record[sp_count].powr_coeffs[moi]);
        }
        if(mocount > 0)
        {
            printf( "%e }\n", allocator_record[sp_count].powr_coeffs[moi]);
        }
        else
        {
            printf("}\n");
        }
        
        for(t = 0; t < ALLOCATOR_RECORD_COUNT; t++)
        {
            next_record = &((SRT_record[t])[sp_count]);
            printf("\t\t%lu, %llu), %lu, [%lu, %u], {%llu, %lli, %lli, %lli}, (%lli) \n",
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
        printf("\n");
    }

    munlockall();

    for(t = 0; t < ALLOCATOR_RECORD_COUNT; t++)
    {
        free(SRT_record[t]);
    }

    free(allocator_record);
}

