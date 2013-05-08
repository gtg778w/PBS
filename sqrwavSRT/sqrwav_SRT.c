/*
gcc -O3 -o sqrwavSRT  sqrwav_SRT.c pbsuser.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <stdint.h>

#include <errno.h>

#include <sched.h>

#include <sys/mman.h>

#include <sys/types.h>

#include <sys/io.h>

#include <time.h>

#include "sqrwav.h"
#include "pbsuser.h"
#include "loadgen.h"

/*****************************************************************************/
//				Workload generation related Code
/*************************************************************************/

uint64_t job(struct sqrwav_struct *sqrwav_p)
{
    uint64_t i;
    uint64_t job_length;
    static uint64_t state = 0;

    job_length = sqrwav_next(sqrwav_p);

    state = work_function(job_length);

    return state;
}

/*****************************************************************************/
//				main function
/*************************************************************************/

char *usage_string = "[options]\n"\
        "-j: number of jobs\n"\
        "-P: period of sqare-wave\n"\
        "-D: duty cycle\n"\
        "-d: initial value of index. (must be positive)\n"\
        "-M: Maximum nominal value\n"\
        "-m: minimum nominal value\n"\
        "-N: noise ratio\n"\
        "\n"\
        "-f: do not propmpt before proceeding\n"\
        "\n"\
        "-p: scheduling period\n"\
        "-b: starting scheduling bandwidth\n"\
        "-l: job history length\n"\
        "-L: name of log file\n\n";

char* optstring
    = "j:P:D:d:M:m:N:p:b:l:L:f";

int main (int argc, char * const * argv)
{
	int ret, dummy_state;

	//variables for the scheduler
	unsigned long maxjobs = 10000;
    unsigned long j;

    //real-time attribute related variables
	unsigned char rflag = 0;
    pid_t my_pid;
    int max_priority;
    struct sched_param sched_param;

    //pbs related variables
	SRT_handle handle;
	unsigned long period = 40000, bandwidth, hlength = 0;
   	char* logfilename = NULL;
	unsigned char pflag = 0, bflag = 0, lflag = 0, jflag = 0;
    unsigned char fflag = 0, Lflag = 0;

    //square-wave generator
    struct      sqrwav_struct sqrwav;
    sqrwav.period = 10000;
    sqrwav.duty_cycle = (1.0/2.0);
    sqrwav.minimum_nominal_value = 1000000;
    sqrwav.maximum_nominal_value = 5000000;    
    sqrwav.noise_ratio = 0.2;
    sqrwav.index = 0;
    sqrwav.rangen_state = 0;
	uint64_t	ns_start, ns_end, ns_diff;

	//process input arguments
	while ((ret = getopt(argc, argv, optstring)) != -1)
	{
		switch(ret)
		{
			case 'j':
				errno = 0;
				maxjobs = strtoul(optarg, NULL, 10);
				if(errno)
				{
					perror("Failed to parse the j option");
					exit(EXIT_FAILURE);
				}
				break;

            case 'r':
                rflag = 1;
                break;

            case 'P':
                errno = 0;
                sqrwav.period = (uint64_t)strtoul(optarg, NULL, 10);
				if(errno)
				{
					perror("Failed to parse the P option");
					exit(EXIT_FAILURE);
				}
                break;

            case 'D':
                errno = 0;
                sqrwav.duty_cycle = (double)strtod(optarg, NULL);
				if(errno)
				{
					perror("Failed to parse the D option");
					exit(EXIT_FAILURE);
				}
                break;

            case 'd':
                errno = 0;
                sqrwav.index = (uint64_t)strtoul(optarg, NULL, 10);
				if(errno)
				{
					perror("Failed to parse the d option");
					exit(EXIT_FAILURE);
				}
                break;
                errno = 0;

            case 'M':
                errno = 0;
                sqrwav.maximum_nominal_value 
                    = (uint64_t)strtoul(optarg, NULL, 10);
				if(errno)
				{
					perror("Failed to parse the M option");
					exit(EXIT_FAILURE);
				}
                break;

            case 'm':
                errno = 0;
                sqrwav.minimum_nominal_value 
                    = (uint64_t)strtoul(optarg, NULL, 10);
				if(errno)
				{
					perror("Failed to parse the m option");
					exit(EXIT_FAILURE);
				}
                break;

            case 'N':
                errno = 0;
                sqrwav.noise_ratio = strtod(optarg, NULL);
				if(errno)
				{
					perror("Failed to parse the N option");
					exit(EXIT_FAILURE);
				}
                break;

            case 'p':
                errno = 0;
				period = strtoul(optarg, NULL, 10);
				if(errno)
				{
					perror("Failed to parse the p option");
					exit(EXIT_FAILURE);
				}

                if(period < 10)
		        {
			        fprintf(stderr, "The period is too small! got %lu\n", 
                        period);
			        return -EINVAL;
		        }

				pflag = 1;
                break;
                
            case 'b':
                errno = 0;
				bandwidth = strtoul(optarg, NULL, 10);
				if(errno)
				{
					perror("Failed to parse the b option");
					exit(EXIT_FAILURE);
				}
				bflag = 1;
                break;

            case 'l':
                errno = 0;
				hlength = strtoul(optarg, NULL, 10);
				if(errno)
				{
					perror("Failed to parse the p option");
					exit(EXIT_FAILURE);
				}

                if(hlength > 120)
	            {
		            fprintf(stderr, "The history length can be at most 120! got %lu\n", hlength);
		            return -EINVAL;
	            }
				lflag = 1;
                break;

            case 'L':
                Lflag = 1;
				logfilename = optarg;
                break;

            case 'f':
                fflag = 1;
                break;

			default:
				fprintf(stderr, "\nUsage: %s %s\n", argv[0], usage_string);
				return -EINVAL;
		}
	}

	if(optind != argc)
	{
		fprintf(stderr, "Usage %s %s\n", argv[0], usage_string);
		return -EINVAL;
	}

	if(bflag)
	{
		if((bandwidth > period) || (bandwidth == 0))
		{
			fprintf(stderr, "The bandwidth must be strictly positive and less than the period! got %lu\n", bandwidth);
			return -EINVAL;
		}
	}
	else
	{
		bandwidth = (period >> 2); //0.25
	}

	//check that the user wants to continue
	printf("\n\nGot the following options:\n");

    printf("\t\njobs:\t%lu\n", maxjobs);		 

    printf("\nSquare Wave Parameters:\n");
    printf("\tsquare-wave period:\t%lu jobs\n", sqrwav.period);
    printf("\tduty cycle:\t\t%f\n", sqrwav.duty_cycle);
    printf("\tmaximum nominal value:\t%lu iterations\n", 
        sqrwav.maximum_nominal_value);
    printf("\tminimum nominal value:\t%lu iterations\n",
        sqrwav.minimum_nominal_value);
    printf("\tnoise ratio:\t\t%f\n", sqrwav.noise_ratio);
    printf("\tinitial index:\t\t%lu\n", sqrwav.index);

    printf("\nSchedulng Parameters:\n");
	printf("\tscheduling period:\t%luus\n", period);
    printf("\tbandwidth:\t\t%lu\n", bandwidth);
    printf("\thlength:\t\t%lu\n", hlength);
	if(Lflag == 1)
	{
		printf("\tLogging Enabled:\t%s\n\n", logfilename);
	}

	if(fflag == 0)
	{
		if(fgetc(stdin) == (int)'q')
		{
			printf("Exiting...\n");
			goto clean_close;
		}
	}

	//setup the scheduler
	ret = pbs_SRT_setup(period, bandwidth, hlength, &handle, Lflag, 10000, logfilename);
	if(ret)
	{
		fprintf(stderr, "Failed to setup scheduler!\n");
		return ret;
	}

	//lock memory
	ret = mlockall(MCL_CURRENT);
	if(ret)
	{
		fprintf(stderr, "Failed to lock memory!\n");
		return ret;
	}

	printf("Running ... ");


	//the main job loop
	for(j = 0; j < maxjobs; j++)
	{        
   		ret = pbs_begin_SRT_job(&handle);
		if(ret)
		{
			break;
		}

		dummy_state ^= (int)job(&sqrwav);
	}

    if(ret == 0)
    {
        ret = pbs_begin_SRT_job(&handle);
    }


clean_close:
	//close cleanly
	pbs_SRT_close(&handle);
    printf("\rCompleted ... %i\n", dummy_state);
	return 0;
}

