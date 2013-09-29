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

void allocator_loop(int proc_file, long log_level)
{
    bw_mgt_cmd_t cmd;

    long long sp_count = 0;
    unsigned char loop_condition = 1;

    double inst_count, energy_count;

    int retval;
    int t, task_count, task_index;

    SRT_loaddata_t *next;

    double   budget_double, budget_sum, saturation_multiplier;
    uint64_t budget_uint64_t;
    unsigned char overload;

    /*  The main allocator loop.  */
    while(loop_condition)
    {
        /*  Sleep until the beginning of the next reservation period.   */
        cmd.cmd = PBS_BWMGT_CMD_NEXTJOB;
        retval = write(proc_file, &cmd, sizeof(cmd));
        if(retval != sizeof(cmd))
        {
            perror( "allocator_loop: Failed to write PBS_BWMGT_CMD_NEXTJOB "
                    "command.");
            break;
        }

        /*  Update the power and performance model coefficients.    */
        pbsAllocator_modeladapters_adapt(&inst_count, &energy_count);
        if(log_level > 0)
        {
            log_allocator_summary();
            
            if((log_level > 1) && (sp_count < sp_limit))
            {
                /*  Log energy and instruction count  */
                log_allocator_dat(sp_count, inst_count, energy_count);
            }
        }
        
        /*  Compute the amount of CPU budget available to allocate. */
        compute_max_CPU_budget();
        
        /*  The budget allocated over all SRT tasks will be summed, initialize the sum 
        to zero.    */
        budget_sum = 0.0;
        
        /*  Determine the number of SRT tasks in the system from the loaddata_list_header
        that is shared with the kernel. */
        task_count = loaddata_list_header->SRT_count;
        
        /*  Get the loaddata structure of the first task in the linked list   */
        next = &(loaddata_array[loaddata_list_header->first]);
        
        /*  Loop over all the tasks in the linked list    */
        for(t = 0; t < task_count; t++)
        {
            task_index = next - loaddata_array;

            /*  Compute the budget allocation before any saturation */
            compute_budget(next, &budget_double);
            presaturation_budget_array[task_index] = budget_double;
            
            budget_sum = budget_sum + budget_double;
            
            next = &(loaddata_array[next->next]);
        }

        /*  Check if the sum of the budget allocated is too high and if saturation is 
            needed  */
        if( budget_sum > maximum_available_CPU_budget   )
        {
            /*  Set the overload flag  */
            overload = 1;
            
            /*  Compute the saturation multiplier   */
            saturation_multiplier = maximum_available_CPU_budget / budget_sum;
        }
        else
        {
            /*  Reset the overload flag  */
            overload = 0;
        }

        /*  Loop over all tasks in the linked list to perform the saturation    */
        next = &(loaddata_array[loaddata_list_header->first]);
        for(t = 0; t < task_count; t++)
        {
            task_index = next - loaddata_array;
            
            if(overload)
            {
                budget_double = saturation_multiplier * 
                                presaturation_budget_array[task_index];
            }
            else
            {
                budget_double = presaturation_budget_array[task_index];
            }
                        
            /*  Write the saturated budget out to the memory-mapped region
                containing all the allocation.  */
            budget_uint64_t = (uint64_t)budget_double;
            allocation_array[task_index] = budget_uint64_t;
            
            /*  Log the amount of budget allocated. */
            if((log_level > 1) && (sp_count < sp_limit))
            {
                log_SRT_sp_dat(t, sp_count, next, budget_uint64_t);
            }

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
"\t-VIC:\tUse VIC-based budget.\n"\
"\t-s:\tthe number of scheduling periods (1)\n"\
"\t-L:\tThe log level (0, 1, or 2)\n";

int main(int argc, char** argv)
{
    int retval;

    int proc_file;

    uint64_t scheduling_period = 10000000;
    uint64_t allocator_budget  = 1000000;

    /*variables for parsing input arguments*/
    unsigned char   fflag=0;
    
    long int log_level = 0;

    sp_limit = 1;

    while((retval = getopt(argc, argv, "fP:V:B:s:L:")) != -1)
    {
        switch(retval)
        {
            case 'f':
                fflag = 1;
                break;
                
            case 'P':
                errno = 0;
                scheduling_period = strtoul(optarg, NULL, 10);
                if(errno != 0)
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
                if(errno != 0)
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
                if(errno != 0)
                {
                    perror("Failed to parse the s option");
                    retval = -EINVAL;
                    goto exit0;
                }
                break;
                
            case 'L':
                errno = 0;
                log_level = strtol(optarg, NULL, 10);
                if(errno != 0)
                {
                    perror("Failed to parse the L option");
                    retval = -EINVAL;
                    goto exit0;
                }
                break;
            
            case 'V':
                retval = strncmp("IC", optarg, 2);
                if(0 == retval)
                {
                    budget_type = PBS_BUDGET_VIC;
                    break;
                }
                /*else, fall through to the default condition*/
                
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
        
        maximum_available_CPU_time =(double)scheduling_period - 
                                    (double)allocator_budget;
    }

    if(log_level >= 2)
    {
        log_level = 2;
    }
    else
    {
        if(log_level < 0)
        {
            log_level = 0;
        }
    }

    /*If no bound is imposed on the number of reservation periods,
      make sure logging is disabled*/
    if((sp_limit == 0) && (log_level == 2))
    {
        fprintf(stderr, 
            "\n\nWARNING: The number of scheduling periods is not bounded!" 
            "Logging is automatically disabled!\n\n");
        log_level = 1;
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
    
    /*Display the type of budget being used*/
    if(PBS_BUDGET_VIC == budget_type)
    {
        fprintf(stderr, ", VIC-based budget");
    }
    else if(PBS_BUDGET_ns == budget_type)
    {
        fprintf(stderr, ", time-based budget");
    }
    else
    {
        fprintf(stderr, "\n\nERROR: budget-type set to unknown value!\n");
        goto exit0;
    }
    
    /*Check if logging is enabled*/
    fprintf(stderr, ", log_level = %li\n", log_level);
    
    /*Check if the user will be prompted before proceeding*/
    if(fflag == 0)
    {
        fprintf(stderr, "\n\nWaiting for prompt from user ...\n");
        if(fgetc(stdin) == (int)'q')
        {
            fprintf(stderr, "\nExiting...\n");
            retval = 0;
            goto exit0;
        }
        fprintf(stderr, "\nRunning...\n");
    }

    //If logging is enabled, setup the log memory
    retval = setup_log_memory(log_level);
    if(retval)
    {
        fprintf(stderr, "setup_log_memory failed!\n");
        goto exit0;
    }

    //Setup the interface with the pbs_allocator module
    //and scheduling related parameters
    proc_file = allocator_setup(scheduling_period, allocator_budget);
    if(proc_file < 0)
    {
        fprintf(stderr, "allocator_setup failed!\n");
        retval = proc_file;
        goto exit0;
    }

    /*Set the mocount field in the log summary*/
    log_summary_setmocount();

    //Setup the model adapters
    retval = pbsAllocator_modeladapters_init(proc_file);
    if(0 != retval)
    {
        fprintf(stderr, "pbsAllocator_modeladapters_init failed!\n");
        goto exit1;
    }

    allocator_loop(proc_file, log_level);
    
exit1:
    /*Indicate to the kernel module that the allocator is closing*/
    retval = allocator_close(proc_file);
    if(retval)
    {
        fprintf(stderr, "allocator_close failed!");
        goto exit0;
    }

    /*free the modeladapters*/    
    pbsAllocator_modeladapters_free(proc_file);

    /*free the log level*/
    free_log_memory(log_level);

    printf("\n");
exit0:
    return 0;
}

