// workload_timing
// 
// a generic program to create a log of job execution times for the PeSoRTA workloads

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <string.h>

#include <stdint.h>

#include <errno.h>

#include <sched.h>

#include <time.h>

#include <sys/io.h>

#include <sys/mman.h>

#include <sys/types.h>

#include <sys/stat.h>

#include <fcntl.h>

/*************************************************************************/
//				MOCsample related Code
/*************************************************************************/

#include "MOCsample_timer_user.h"

int MOCsample_fd;

int MOCsample_open(char *MOC_type)
{
    int ret = 0;
    int arg_len;
    char filename_buffer[64];
    
    arg_len = strnlen(MOC_type, 47);
    if(47 == arg_len)
    {
        ret = -1;
        goto exit0;
    }
    
    snprintf(filename_buffer, 64, "/proc/MOCsample_%s", MOC_type);
    
    MOCsample_fd = open(filename_buffer, O_WRONLY);
    if(-1 == MOCsample_fd)
    {
        perror("open failed in MOCsample_setup");
        ret = -1;
        goto exit0;
    }
    
exit0:
    return ret;
}

void MOCsample_close(void)
{
    close(MOCsample_fd);
}

/*****************************************************************************/


#include "PeSoRTA.h"

char *usage_string 
	= "[-j <maxjobs>] [-r [-p <real-time priority>]] [ -R <workload root directory>] [-C <config file>] [-L <output file>] [-M <measure of computation>] [-P <sampling period>] [-N <sample count>]";
char *optstring = "j:rp:R:C:L:M:P:N:";

int main (int argc, char * const * argv)
{
	int ret;

    /*variables for parsing options*/
	unsigned char jflag = 0;
	long maxjobs = 0;
    unsigned char rflag = 0;
    unsigned char pflag = 0;
    double desired_priority = 1.0;
    
    char *MOC_name = "inst";
    char *workload_root_dir = "./";
    char *config_file = "config";
    char *logfile_name = "MOCsamples.csv";

    /*arguments related to timed sampling*/
    u64 period = 3000000; /*in nanoseconds*/
    u64 samples= 20000;     

    /*working directory*/
    void    *buffer_p = NULL;
    char    *cwd_name_buffer = NULL;
    char    *cwd_name = NULL;
    int     cwd_name_length = 0;
    
    /*the workload state*/
    void *workload_state = NULL;	
    
	/*variables for the scheduler*/
    pid_t my_pid;
    double max_priority;
    double min_priority;
    int set_priority;
    struct sched_param sched_param;

    /*job traversal*/
    long possiblejobs;
	long jobi;
	
	/*Sampling log memory*/
    MOCsample_timed_sample_t *log_mem; 
    u64 log_mem_len;
    u64 s_i;

    /*logfile*/
    FILE *logfile_h;

	/* process input arguments */
	while ((ret = getopt(argc, argv, optstring)) != -1)
	{
		switch(ret)
		{
			case 'j':
				errno = 0;
				maxjobs = strtol(optarg, NULL, 10);
				if(errno)
				{
					perror("main: Failed to parse the j option");
					goto exit0;
				}
				jflag = 1;
				break;

            case 'r':
                rflag = 1;
                break;
            
            case 'p':
                pflag = 1;
                errno = 0;
                desired_priority = strtod(optarg, NULL);
                if( (errno == 0))
                {
                    if( (desired_priority >= 0.0) && 
                        (desired_priority <= 1.0) )
                    {
                        break;
                    }
                    else
                    {
                        fprintf(stderr, "main: Desired priority must be between "
                                        "0.0 (minimum priority) and 1.0 "
                                        "(maximum priority).\n");
                        goto exit0;
                    }
                }
                else
                {
                    perror("main: Failed to parse the p option");
                    goto exit0;
                }
                break;

            case 'R':
                workload_root_dir = optarg;
                break;

            case 'C':
                config_file = optarg;
                break;

            case 'L':
                logfile_name = optarg;
                break;
            
            case 'M':
                MOC_name = optarg;
                break;
            
            case 'P':
                errno = 0;
                period = (u64)strtoull(optarg, NULL, 10);
                if(errno != 0)
                {
                    perror("main: Failed to parse the P option");
                    goto exit0;                    
                }
                break;
                
            case 'N':
                errno = 0;
                samples = (u64)strtoull(optarg, NULL, 10);
                if(errno != 0)
                {
                    perror("main: Failed to parse the N option");
                    goto exit0;                    
                }
                break;
			default:
				fprintf(stderr, "main: Bad option %c!\nUsage %s %s!\n", 
				                (char)ret, argv[0], usage_string);
				ret = -EINVAL;
				goto exit0;
		}
	}

	if(optind != argc)
	{
		fprintf(stderr, "main: Usage %s %s!\n", argv[0], usage_string);
		ret = -EINVAL;
		goto exit0;
	}

    /*Save the current working directory*/
    cwd_name_length = 512;
    do
    {
        buffer_p = realloc(cwd_name_buffer, sizeof(char)*cwd_name_length);
        if(NULL == buffer_p)
        {
            perror( "main: realloc failed to allocate memory for "
                    "the current directory name buffer ");
            ret = -1;
            goto exit1;
        }
        else
        {
            cwd_name_buffer = (char*)buffer_p;
        }
        
        errno = 0;
        cwd_name = getcwd(cwd_name_buffer, cwd_name_length);
        if(NULL == cwd_name)
        {
            if(ERANGE == errno)
            {
                cwd_name_length = cwd_name_length + 512;
            }
            else
            {
                perror("main: getcwd failed");
                ret = -1;
                goto exit0;
            }
        }
    }while(NULL == cwd_name);
    
    /*Cheange to the desired working directory*/
    ret = chdir(workload_root_dir);
    if(ret < 0)
    {
        fprintf(stderr, "main: failed to change the current working "
                "directory to the desired directory \"%s\"", 
                workload_root_dir);
        perror("main: chdir failed");
        ret = -1;
        goto exit0;
    }
    printf("\t\tWorkload root directory: %s\n", workload_root_dir);
    
    /*Open the MOCsample_* file*/
    ret = MOCsample_open(MOC_name);
    if(-1 == ret)
    {
        fprintf(stderr, "main: MOCsample_open failed for MOC name \"%s\".\n",
                MOC_name);
        ret = -1;
        goto exit0;
    }
    
    /*Initialize the workload*/
    ret = workload_init(config_file, &workload_state, &possiblejobs);
    if(ret < 0)
    {
        fprintf(stderr, "main: workload_init failed\n");
        goto exit0b;
    }
    printf("\t\tWorkload config file: %s\n", config_file);
        
    /*Check if the workload returned a valid possiblejobs*/
    if(possiblejobs < 0)
    {
        /*Set it to an arbitrarily high value*/
        possiblejobs = 10000;
    }
    
    /*Check the number of jobs*/
    if(jflag == 0)
    {
        maxjobs = possiblejobs;
    }
    printf("\t\tNumber of jobs: %li\n", possiblejobs);

	/*Allocate space for the sampling log*/
    log_mem = (MOCsample_timed_sample_t*)malloc(samples * sizeof(MOCsample_timed_sample_t));
    if(NULL == log_mem)
    {
        fprintf(stderr, "main: failed to allocate memory for the sampling log\n");
        perror("main: malloc failed");
        goto exit1;
    }
    log_mem_len = samples;

    if(rflag == 1)
    {
        ret = mlockall(MCL_CURRENT);
        if(ret == -1)
        {
            perror("main: mlock failed");
            goto exit2;
        }

        my_pid = getpid();
        
        max_priority = (double)sched_get_priority_max(SCHED_FIFO);
        if(max_priority == -1.0)        
        {
            perror("main: sched_get_priority_max failed");
            goto exit3;
        }

        min_priority = (double)sched_get_priority_min(SCHED_FIFO);
        if(min_priority == -1.0)        
        {
            perror("main: sched_get_priority_min failed");
            goto exit3;
        }

        set_priority = (int)(   (desired_priority * max_priority)  +
                                ((1.0 - desired_priority) * min_priority));

        sched_param.sched_priority = set_priority;
        ret = sched_setscheduler(my_pid, SCHED_FIFO, &sched_param);
        if(ret == -1)
        {
            fprintf(stderr, "main: failed to set real-time priority!\n");
            perror("main: sched_setsceduler failed");
            goto exit3;
        }
        printf("\t\tReal-time priority: %i\n", set_priority);
        fflush(stdout);
    }
    else
    {
        if(pflag == 1)
        {
            fprintf(stderr, "WARNING: rflag is not set. pflag is ignored!\n");
        }
    }

    /*Start the timed sampling.*/
    ret = MOCsample_timer_start(MOCsample_fd,
                                period,    
                                log_mem_len);
    if(ret < 0)
    {
        fprintf(stderr, "main: MOCsample_timer_start failed\n");
        goto exit3;
    }

	/* the main job loop */
	for(jobi = 0; jobi < maxjobs; jobi++)
	{
	    /*run and time the next job*/
		ret = perform_job(workload_state);

        /*make sure the job didn't incur any errors*/
		if(ret < 0)
		{
            fprintf(stderr, "main: perform_job failed for job %li\n", jobi);
            /*correct the number of executed jobs*/
            maxjobs = jobi;
			break;
		}
		else if(ret == 1)
		{
		    /*no more jobs to perform*/
		    maxjobs = jobi;
		}
	}

    /*Stop the timed sampling.*/
    ret = MOCsample_timer_stop( MOCsample_fd,
                                log_mem_len,
                                log_mem,
                                &samples);
    if(ret < 0)
    {
        fprintf(stderr, "main: MOCsample_timer_stop failed\n");
        goto exit3;
    }
    
    /*Cheange back to the original working directory*/
    ret = chdir(cwd_name);
    if(ret < 0)
    {
        fprintf(stderr, "main: getcwd failed to change the current working "
                        "directory to the desired directory \"%s\" ", cwd_name);
        perror("");
        ret = -1;
        goto exit3;
    }

    /*open the log file*/
    logfile_h = fopen(logfile_name, "w");
    if(NULL == logfile_h)
    {
        fprintf(stderr, "main: Failed to open log file \"%s\"!\n", logfile_name);
        perror("main: fopen failed");
        goto exit3;
    }
    
    /*write the log_mem out to file*/
    for(s_i = 0; s_i < s_i; s_i++)
    {
        ret = fprintf(logfile_h,    "%lu, %lu\n", 
                                    log_mem[jobi].time_stamp, 
                                    log_mem[jobi].MOCsample);
        if(ret < 0)
        {
            fprintf(stderr, "main: Failed to write log index %li to log file!\n", jobi);
            perror("main: fprintf failed");
            goto exit4;
        }
    }

    /*undo everything*/
exit4:
    fclose(logfile_h);
exit3:
    if(rflag == 1)
    {
        munlockall();
    }
exit2:
	free(log_mem);
exit1:
    workload_uninit(workload_state);
exit0b:
    MOCsample_close();
exit0:
    if(NULL != cwd_name_buffer)
    {
        free(cwd_name_buffer);
    }

    fprintf(stderr, "%s: main failed\n", argv[0]);

	return 0;
}

