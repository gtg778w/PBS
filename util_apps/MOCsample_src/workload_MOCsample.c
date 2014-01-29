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
//				Timing-related Code
/*************************************************************************/

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
    
    MOCsample_fd = open(filename_buffer, O_RDONLY);
    if(-1 == MOCsample_fd)
    {
        perror("open failed in MOCsample_setup");
        ret = -1;
        goto exit0;
    }
    
exit0:
    return ret;
}

static char LUT_hextobyte[256] =    
            /*   0   1   2   3   4   5   6   7   8   9   a   b   c   d   e   f*/
            {   -1, -1, -1, -1, -1, -1, -1, -1, -1, 32, 32, 32, 32, 32, -1, -1, /* 0 */
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 1 */
                32, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 2 */
                 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1, -1, -1, -1, -1, -1, /* 3 */
                -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 4 */
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 5 */
                -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 6 */
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 7 */
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 8 */
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 9 */
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* a */
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* b */
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* c */
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* d */
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* e */
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* f */};

static __inline__ uint64_t convertu64toChar(char *hexstring)
{    
    uint64_t value;
    char digit, *digit_p;
    
    /*Unnecessary code in critical path.*/
    /*if( (hexstring[0] == '0') || (hexstring[1] == 'x') )
    {
        hexstring = &hexstring[2];
    }*/
    
    /*Parse the rest of the string*/
    value = 0;
    for(digit_p = hexstring;
        ((digit = LUT_hextobyte[(int)(*digit_p)]) != -1);
        digit_p++)
    {
        value = (value << 4) | digit;
    }

    return value;
}

static __inline__ uint64_t MOCsample_read(void)
{
    uint64_t MOCsample_now;
    char read_buf[18];

    read(MOCsample_fd, read_buf, 18);
    MOCsample_now = convertu64toChar(read_buf);

    return MOCsample_now;
}

void MOCsample_close(void)
{
    close(MOCsample_fd);
}

/*****************************************************************************/


#include "PeSoRTA.h"

char *usage_string 
	= "[-j <maxjobs>] [-r [-p <real-time priority>]] [ -R <workload root directory>] [-C <config file>] [-L <output file>] [-M <measure of computation>]";
char *optstring = "j:rp:R:C:L:M:";

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
    char *logfile_name = "timing.csv";

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

    /*timing log*/
    long possiblejobs;
	long jobi;
    uint64_t *log_mem; 

    /*logfile*/
    FILE *logfile_h;

	uint64_t	MOCsample_start, MOCsample_end, MOCsample_diff;

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
					perror("Failed to parse the j option");
					exit(EXIT_FAILURE);
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
                        fprintf(stderr, "Desired priority must be between "
                                        "0.0 (minimum priority) and 1.0 "
                                        "(maximum priority).\n");
                        exit(EXIT_FAILURE);
                    }
                }
                else
                {
                    perror("Failed to parse the p option");
                    exit(EXIT_FAILURE);
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
            
			default:
				fprintf(stderr, "ERROR: Bad option %c!\nUsage %s %s!\n", 
				                (char)ret, argv[0], usage_string);
				ret = -EINVAL;
				goto exit0;
		}
	}

	if(optind != argc)
	{
		fprintf(stderr, "ERROR: Usage %s %s!\n", argv[0], usage_string);
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
            fprintf(stderr, "ERROR: (%s) main)  realloc failed to allocate memory for "
                            "the current directory name buffer ", workload_name());
            perror("");
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
                fprintf(stderr, "ERROR: (%s) main)  getcwd failed ", workload_name());
                perror("");
                ret = -1;
                goto exit0;
            }
        }
    }while(NULL == cwd_name);
    
    /*Cheange to the desired working directory*/
    ret = chdir(workload_root_dir);
    if(ret < 0)
    {
        fprintf(stderr, "ERROR: (%s) main)  getcwd failed to change the current working "
                        "directory to the desired directory \"%s\" ", 
                        workload_name(), workload_root_dir);
        perror("");
        ret = -1;
        goto exit0;
    }
    printf("\t\tWorkload root directory: %s\n", workload_root_dir);
    
    /*Open the MOCsample_* file*/
    ret = MOCsample_open(MOC_name);
    if(-1 == ret)
    {
        fprintf(stderr, "ERROR: (%s) main) MOCsample_open failed for MOC name \"%s\".\n",
                        workload_name(), MOC_name);
        ret = -1;
        goto exit0;
    }
    
    /*Initialize the workload*/
    ret = workload_init(config_file, &workload_state, &possiblejobs);
    if(ret < 0)
    {
        fprintf(stderr, "ERROR: (%s) main) workload_init failed\n", workload_name());
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

	/*Allocate space for the timing log_mem*/
    log_mem = (uint64_t*)malloc(maxjobs * sizeof(uint64_t));
    if(NULL == log_mem)
    {
        fprintf(stderr, "ERROR: failed to allocate memory for timing log_mem\n");
        perror("ERROR: malloc failed in main");
        goto exit1;
    }

    if(rflag == 1)
    {
        ret = mlockall(MCL_CURRENT);
        if(ret == -1)
        {
            perror("ERROR: mlock failed in main");
            goto exit2;
        }

        my_pid = getpid();
        
        max_priority = (double)sched_get_priority_max(SCHED_FIFO);
        if(max_priority == -1.0)        
        {
            perror("ERROR: sched_get_priority_max failed in main");
            goto exit3;
        }

        min_priority = (double)sched_get_priority_min(SCHED_FIFO);
        if(min_priority == -1.0)        
        {
            perror("ERROR: sched_get_priority_min failed in main");
            goto exit3;
        }

        set_priority = (int)(   (desired_priority * max_priority)  +
                                ((1.0 - desired_priority) * min_priority));

        sched_param.sched_priority = set_priority;
        ret = sched_setscheduler(my_pid, SCHED_FIFO, &sched_param);
        if(ret == -1)
        {
            fprintf(stderr, "ERROR: failed to set real-time priority!\n");
            perror("ERROR: sched_setsceduler failed in main");
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

	/* the main job loop */
	for(jobi = 0; jobi < maxjobs; jobi++)
	{
	    /*run and time the next job*/
		MOCsample_start = MOCsample_read();
		ret = perform_job(workload_state);
    	MOCsample_end = MOCsample_read();

        /*make sure the job didn't incur any errors*/
		if(ret < 0)
		{
            fprintf(stderr, "ERROR: main) (%s) perform_job returned -1 for job %li\n", 
                workload_name(), jobi);
            /*correct the number of executed jobs*/
            maxjobs = jobi;
			break;
		}
		else if(ret == 1)
		{
		    /*no more jobs to perform*/
		    maxjobs = jobi;
		}

        /*log_mem the time*/
        MOCsample_diff = MOCsample_end - MOCsample_start;

        log_mem[jobi] = MOCsample_diff;
	}

    /*Cheange back to the original working directory*/
    ret = chdir(cwd_name);
    if(ret < 0)
    {
        fprintf(stderr, "ERROR: (%s) main)  getcwd failed to change the current working "
                        "directory to the desired directory \"%s\" ", 
                        workload_name(), cwd_name);
        perror("");
        ret = -1;
        goto exit3;
    }

    /*open the log file*/
    logfile_h = fopen(logfile_name, "w");
    if(NULL == logfile_h)
    {
        fprintf(stderr, "ERROR: Failed to open log file \"%s\"!\n", logfile_name);
        perror("ERROR: fopen failed in main");
        goto exit3;
    }
    
    /*write the log_mem out to file*/
    for(jobi = 0; jobi < maxjobs; jobi++)
    {
        ret = fprintf(logfile_h, "%lu,\n", log_mem[jobi]);
        if(ret < 0)
        {
            fprintf(stderr, "ERROR: Failed to write log index %li to log file!\n", jobi);
            perror("ERROR: fprintf failed in main");
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

	return 0;
}

