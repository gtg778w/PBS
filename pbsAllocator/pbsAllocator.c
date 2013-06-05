#include <stdio.h>
/*
fprintf
stderr
perror
*/

#include <stdlib.h>
/*
strtoul
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
strlen
*/

#include <errno.h>
/*
errno
*/

#include <unistd.h>
/*
getopt
write
*/

#include "pbsAllocator.h"

void allocator_loop_wlogging(int proc_file)
{
    bw_mgt_cmd_t cmd;
    
    long long sp_count = 0;

    int retval;
    int t, task_count, task_index;

    SRT_loaddata_t *next;

    uint64_t budget;

    while(sp_count < sp_limit)
    {
    
        cmd.cmd = PBS_BWMGT_CMD_NEXTJOB;
        retval = write(proc_file, &cmd, sizeof(cmd));
        if(retval != sizeof(cmd))
        {
            perror( "allocator_loop_wlogging: Failed to write PBS_BWMGT_CMD_NEXTJOB "
                    "command.");
            break;
        }

        sp_count++;
        log_allocator_dat(sp_count);

        next = &(loaddata_array[loaddata_list_header->first]);
        task_count = loaddata_list_header->SRT_count;

        for(t = 0; t < task_count; t++)
        {
            task_index = next - loaddata_array;

            compute_budget(next, &budget);
            allocation_array[task_index] = budget;
            
            log_SRT_sp_dat(task_index, sp_count, next, budget);

            next = &(loaddata_array[next->next]);
        }

    }
    
}

void allocator_loop(int proc_file)
{
    bw_mgt_cmd_t cmd;

    long long sp_count = 0;
    unsigned char loop_condition = 1;

    int retval;
    int t, task_count, task_index;

    SRT_loaddata_t *next;

    uint64_t budget;

    while(loop_condition)
    {
        cmd.cmd = PBS_BWMGT_CMD_NEXTJOB;
        retval = write(proc_file, &cmd, sizeof(cmd));
        if(retval != sizeof(cmd))
        {
            perror( "allocator_loop: Failed to write PBS_BWMGT_CMD_NEXTJOB "
                    "command.");
            break;
        }

        next = &(loaddata_array[loaddata_list_header->first]);
        task_count = loaddata_list_header->SRT_count;

        for(t = 0; t < task_count; t++)
        {
            task_index = next - loaddata_array;

            compute_budget(next, &budget);
            allocation_array[task_index] = budget;

            next = &(loaddata_array[next->next]);
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
"\t-P:\tThe length of a scheduling period. (ns)\n"\
"\t-B:\tThe budget allocated to the scheduler over a scheduling period. (ns)\n"\
"\t-s:\tthe number of scheduling periods (1)\n"\
"\t-S:\tdo not keep or output a log\n";

int main(int argc, char** argv)
{
    int retval;

    int proc_file;

    uint64_t scheduling_period = 10000000;
    uint64_t allocator_budget = 1000000;

    /*variables for parsing input arguments*/
    unsigned char   fflag=0, Sflag = 0;

    sp_limit = 1;

    while((retval = getopt(argc, argv, "fP:B:s:S")) != -1)
    {
        switch(retval)
        {
            case 'f':
                fflag = 1;
                break;
                
            case 'P':
                errno = 0;
                scheduling_period = strtoul(optarg, NULL, 10);
                if(errno)
                {
                    perror("Failed to parse the P option");
                    retval = -EINVAL;
                    goto exit0;
                }

                if(0 == scheduling_period)
                {
                    fprintf(stderr, "The scheduling period must be strictly positive!\n");
                    retval = -EINVAL;
                    goto exit0;
                }

                break;
                
            case 'B':
                errno = 0;
                allocator_budget = strtoul(optarg, NULL, 10);
                if(errno)
                {
                    perror("Failed to parse the B option");
                    retval = -EINVAL;
                    goto exit0;
                }
                
                if(0 == allocator_budget)
                {
                    fprintf(stderr, "The allocator budget must be strictly positive!\n");
                    retval = -EINVAL;
                    goto exit0;
                }
                
                break;
                
            case 's':
                errno = 0;
                sp_limit = strtoul(optarg, NULL, 10);
                if(errno)
                {
                    perror("Failed to parse the s option");
                    retval = -EINVAL;
                    goto exit0;
                }
                break;
                
            case 'S':
                Sflag = 1;
                break;
                
            default:
                fprintf(stderr, "Usage: %s [Options]\n%s\n", 
                                argv[0], options_string);
                retval = -EINVAL;
                goto exit0;
        }
    }

    /*Make sure the number of arguments make sense*/
    if(optind != argc)
    {
        fprintf(stderr, "Usage: %s [Options]\n%s\n", argv[0], options_string);
        retval = -EINVAL;
        goto exit0;
    }

    /*Make sure the scheduling period and allocator budget make sense*/
    if(scheduling_period < allocator_budget)
    {
        fprintf(stderr, "Invalid parameters for scheduling period and allocator budget:\n"
                        "(T, Q) = (%llu, %llu) \n"
                        "The allocator budget must be less than the scheduling period.\n",
                        (long long unsigned int)scheduling_period, 
                        (long long unsigned int)allocator_budget);
        retval = EINVAL;
        goto exit0;
    }
    else
    {
        fprintf(stderr, "(T, Q) = (%llu, %llu)",
                        (long long unsigned int)scheduling_period, 
                        (long long unsigned int)allocator_budget);
    }

    /*If no bound is imposed on the number of reservation periods,
      make sure logging is disabled*/
    if((sp_limit == 0) && (Sflag == 0))
    {
        fprintf(stderr, 
            "WARNING: The number of scheduling periods is not bounded!" 
            "Logging is automatically disabled!\n");
        Sflag = 1;
    }

    /*Display the number of reservation periods*/
    if(sp_limit == 0)
    {
        fprintf(stderr, ", count = infinity");
    }
    else
    {
        fprintf(stderr, ", count = %lli", (long long int)sp_limit);
    }
    
    /*Check if logging is enabled*/
    if(Sflag == 0)
    {
        fprintf(stderr, ", logging to stdout enabled.\n");
    }
    else
    {
        fprintf(stderr, ", logging to stdout disabled.\n");
    }
    
    /*Check if the user will be prompted before proceeding*/
    if(fflag == 0)
    {
        fprintf(stderr, "Waiting for prompt from user ...\n");
        if(fgetc(stdin) == (int)'q')
        {
            fprintf(stderr, "\nExiting...\n");
            retval = 0;
            goto exit0;
        }
        fprintf(stderr, "\nRunning...\n");
    }

    //If logging is enabled, setup the log memory
    if(Sflag == 0)
    {
        retval = setup_log_memory();
        if(retval)
        {
            fprintf(stderr, "setup_log_memory failed!\n");
            goto exit0;
        }
    }

    //Setup the interface with the pbs_allocator module
    //and scheduling related parameters
    proc_file = allocator_setup(scheduling_period, allocator_budget);
    if(proc_file < 0)
    {
        fprintf(stderr, "allocator_setup failed!");
        retval = proc_file;
        goto exit0;
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
        goto exit0;
    }

    if(Sflag == 0)
    {
        free_log_memory();
    }

    printf("\n");
exit0:
    return 0;
}

