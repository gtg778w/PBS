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
        "-A: prediction algorithm (\"mabank\" by default)"
        "-p: scheduling period\n"\
        "-b: initial exec-time prediction\n"\
        "-a: alpha parameter\n"\
        "-L: logging level\n"\
        "-R: name of log file (\"log.csv\" by default)\n\n";

char* optstring
    = "j:P:D:d:M:m:N:A:p:b:a:L:R:f";

int main (int argc, char * const * argv)
{
	int ret, dummy_state;

	//variables for the scheduler
	unsigned long maxjobs = 10000;
    unsigned long j;

    //pbs related variables
	SRT_handle handle;
	unsigned long period = 40000, bandwidth;
	double alpha = 1.0;
	unsigned char bflag = 0;
    unsigned char fflag = 0;
    //logging related variables
    int loglevel = 0;
  	char* logfilename = "log.csv";

    //execution-time predictor related variables
    pbsSRT_predictor_t predictor;
    char *predictor_name = "template";

    //square-wave generator
    struct      sqrwav_struct sqrwav;
    sqrwav.period = 10000;
    sqrwav.duty_cycle = (1.0/2.0);
    sqrwav.minimum_nominal_value = 1000000;
    sqrwav.maximum_nominal_value = 5000000;    
    sqrwav.noise_ratio = 0.2;
    sqrwav.index = 0;
    sqrwav.rangen_state = 0;

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
		            ret = -EINVAL;
		            goto exit0;
				}
				break;

            case 'P':
                errno = 0;
                sqrwav.period = (uint64_t)strtoul(optarg, NULL, 10);
				if(errno)
				{
					perror("Failed to parse the P option");
		            ret = -EINVAL;
		            goto exit0;
				}
                break;

            case 'D':
                errno = 0;
                sqrwav.duty_cycle = (double)strtod(optarg, NULL);
				if(errno)
				{
					perror("Failed to parse the D option");
		            ret = -EINVAL;
		            goto exit0;
				}
                break;

            case 'd':
                errno = 0;
                sqrwav.index = (uint64_t)strtoul(optarg, NULL, 10);
				if(errno)
				{
					perror("Failed to parse the d option");
		            ret = -EINVAL;
		            goto exit0;
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
		            ret = -EINVAL;
		            goto exit0;
				}
                break;

            case 'm':
                errno = 0;
                sqrwav.minimum_nominal_value 
                    = (uint64_t)strtoul(optarg, NULL, 10);
				if(errno)
				{
					perror("Failed to parse the m option");
		            ret = -EINVAL;
		            goto exit0;
				}
                break;

            case 'N':
                errno = 0;
                sqrwav.noise_ratio = strtod(optarg, NULL);
				if(errno)
				{
					perror("Failed to parse the N option");
		            ret = -EINVAL;
		            goto exit0;
				}
                break;

            case 'A':
                predictor_name = optarg;
                break;

            case 'p':
                errno = 0;
				period = strtoul(optarg, NULL, 10);
				if(errno)
				{
					perror("Failed to parse the p option");
		            ret = -EINVAL;
		            goto exit0;
				}

                if(period < 10)
		        {
			        fprintf(stderr, "The period is too small! got %lu\n", 
                        period);
		            ret = -EINVAL;
		            goto exit0;
		        }

                break;
                
            case 'b':
                errno = 0;
				bandwidth = strtoul(optarg, NULL, 10);
				if(errno)
				{
					perror("Failed to parse the b option");
		            ret = -EINVAL;
		            goto exit0;
				}
				bflag = 1;
                break;

            case 'a':
                errno = 0;
				alpha = strtod(optarg, NULL);
				if(errno)
				{
					perror("Failed to parse the a option");
		            ret = -EINVAL;
		            goto exit0;
				}

                if(alpha < 0)
	            {
		            fprintf(stderr, "The alpha parameter must be non-negative. "
		                            "Prased -a %f.\n", alpha);
		            ret = -EINVAL;
		            goto exit0;
	            }
                break;

            case 'L':
                errno = 0;
                loglevel = strtol(optarg, NULL, 0);
                if(errno)
                {
                    perror("Failed to parse the L option");
                    ret = -EINVAL;
                    goto exit0;
                }
                
                if((loglevel < pbsSRT_LOGLEVEL_MIN) || (loglevel > pbsSRT_LOGLEVEL_MAX))
                {
                    fprintf(stderr, "The log level option (-L) , must have an integer "
                                    "argument between %i and %i inclusive.\n",
                                    pbsSRT_LOGLEVEL_MIN, pbsSRT_LOGLEVEL_MAX);
                    ret = -EINVAL;
                    goto exit0;
                }
                break;
            
            case 'R':
                logfilename = optarg;
                break;
                
            case 'f':
                fflag = 1;
                break;

			default:
				fprintf(stderr, "\nUsage: %s %s\n", argv[0], usage_string);
				ret = -EINVAL;
    			goto exit0;
		}
	}

	if(optind != argc)
	{
		fprintf(stderr, "Usage %s %s\n", argv[0], usage_string);
		ret = -EINVAL;
		goto exit0;	
	}

	if(bflag)
	{
		if((bandwidth > period) || (bandwidth == 0))
		{
			fprintf(stderr, "The bandwidth must be strictly positive and less than the period! got %lu\n", bandwidth);
			ret = -EINVAL;
			goto exit0;
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
    printf("\talpha:\t\t%f\n", alpha);

	printf("\tLogging level:\t%i\n", loglevel);
	printf("\tLog file:\t\t%s\n\n", logfilename);
	
	if(fflag == 0)
	{
		if(fgetc(stdin) == (int)'q')
		{
			printf("Exiting...\n");
			ret = 0;
			goto exit0;
		}
	}

    //setup the predictor
    ret = pbsSRT_getPredictor(&predictor, predictor_name);
    if(-1 == ret)
    {
        fprintf(stderr, "pbsSRT_getPredictor with argument \"%s\" failed in main",
                        predictor_name);
        ret = -1;
        goto exit0;
    }

	//setup the scheduler
	ret = pbsSRT_setup(period, bandwidth, alpha, &predictor, &handle, 
	                   loglevel, maxjobs, logfilename);
	if(ret)
	{
		fprintf(stderr, "Failed to setup scheduler!\n");
		ret = -1;
		goto exit1;
	}

	//lock memory
	ret = mlockall(MCL_CURRENT);
	if(ret)
	{
		fprintf(stderr, "Failed to lock memory!\n");
		ret = -1;
		goto exit1;
	}

	printf("Running ... ");

    //Sleep until the next scheduling-period boundary
	ret = pbsSRT_sleepTillFirstJob(&handle);
	if(ret)
	{
	    fprintf(stderr, "main: pbsSRT_sleepTillFirstJob failed!\n");
		ret = -1;
		goto exit1;
	}
	
	//The main job loop	
	for(j = 0; j < maxjobs; j++)
	{
	    //The main workload for the job
		dummy_state ^= (int)job(&sqrwav);
		
		//Sleep until the next task-period boundary
   		ret = pbsSRT_sleepTillNextJob(&handle);
		if(ret)
		{
		    fprintf(stderr, "main: pbsSRT_sleepTillNextJob failed!\n");
		    ret = -1;
			goto exit1;
		}
	}
	
exit1:
    //Notify the end of the SRT task
	pbsSRT_close(&handle);
    printf("\rCompleted ... %i\n", dummy_state);
exit0:
	return 0;
}

