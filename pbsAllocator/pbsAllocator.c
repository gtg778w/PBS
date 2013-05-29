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
"\t-s:\tthe number of scheduling periods (1)\n"\
"\t-S:\tdo not keep or output a log\n";

int main(int argc, char** argv)
{
	int retval;

    int proc_file;

	/*variables for parsing input arguments*/
	unsigned char   fflag=0, Sflag = 0;

	sp_limit = 1;

    while((retval = getopt(argc, argv, "fs:S")) != -1)
    {
        switch(retval)
        {
            case 'f':
                fflag = 1;
                break;
            case 's':
                sp_limit = strtoul(optarg, NULL, 10);
                if(sp_limit < 0)
                {
                    fprintf(stderr, "Bad sp_limit value (%lli), should be positive or "
                                    "0(infinite)!\n", 
                                    (long long int)sp_limit);
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

    if(sp_limit == 0)
        fprintf(stderr, ", count = infinity");
    else
        fprintf(stderr, ", count = %lli", (long long int)sp_limit);

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

