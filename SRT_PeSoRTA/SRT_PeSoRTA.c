#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <stdint.h>

#include <errno.h>

#include <sched.h>

#include <fcntl.h>

#include <sys/stat.h>

#include <sys/mman.h>

#include <sys/types.h>

#include <sys/io.h>

#include <time.h>

#include "SRT.h"
#include "libPredictor.h"
#include "PeSoRTA.h"

/**************************************************************************
                        workload-related functions
**************************************************************************/
int setup_workload( char *workload_configurtaion_file,
                    char *workload_root_directory,
                    int *p_fd_cwd, void** p_workload_state, long *p_possiblejobs)
{
    int ret;
    
    int fd_cwd;
    
    void *workload_state = NULL;
    
    long possiblejobs;
    
    /*Open the current working directory (to be able to return to it)*/
    fd_cwd = open(".", O_RDONLY);
    if(-1 == fd_cwd)
    {
        perror("setup_workload: open failed for \".\"");
        goto error0;
    }
    
    /*Cheange to the desired working directory*/
    ret = chdir(workload_root_directory);
    if(ret < 0)
    {
        fprintf(stderr, "setup_workload:  chdir failed to change the current working "
                        "directory to the desired directory \"%s\" ", 
                        workload_root_directory);
        perror("");
        goto error1;
    }

    /*Initialize the workload*/
    ret = workload_init(    workload_configurtaion_file, 
                            &workload_state, &possiblejobs);
    if(ret < 0)
    {
        fprintf(stderr, "setup_workload: workload_init failed\n");
        goto error2;
    }

    /*Store the return values and return*/
    *p_possiblejobs = possiblejobs;
    *p_workload_state = workload_state;
    *p_fd_cwd = fd_cwd;
    return 0;

error2:
    /*Change back to the original working directory*/
    fchdir(fd_cwd);
error1:
    /*Close the file descriptor for the original working directory*/    
    close(fd_cwd);
error0:
    return -1;
}

void cleanup_workload(int fd_cwd, void* workload_state)
{
    /*Perform any cleanup operation associated with the workload*/
    workload_uninit(workload_state);
    /*Change back to the original working directory*/
    fchdir(fd_cwd);
    /*Close the file descriptor for the original working directory*/
    close(fd_cwd);
}

/*****************************************************************************/
//                main function
/*************************************************************************/

char *usage_string = "[options]\n"\
        "-W: workload configuration file\n"\
        "-D: workload root directory\n"
        "-J: maximum number of jobs to run\n"
        "\n"\
        "-f: do not propmpt before proceeding\n"\
        "\n"\
        "-A: prediction algorithm (\"mabank\" by default)"
        "-p: scheduling period\n"\
        "-c: initial exec-time prediction\n"\
        "-a: alpha parameter\n"\
        "-L: logging level\n"\
        "-R: name of log file (\"log.csv\" by default)\n\n";

char* optstring
    = "W:D:J:fA:p:c:a:L:R:";

int main (int argc, char * const * argv)
{
    int ret;

    //SRT related variables
    SRT_handle handle;
    uint64_t period = 40000000;
    uint64_t estimated_mean_exectime = 20000000;
    double alpha = 1.0;
    unsigned char cflag = 0;
    unsigned char fflag = 0;
    //logging related variables
    int loglevel = 0;
    char* logfilename = "log.csv";

    //execution-time predictor related variables
    SRT_Predictor_t predictor;
    char *predictor_name = "template";

    //workload related variables
    char *workload_configname= NULL;
    char *workload_root_directory = NULL;
    int  fd_cwd;
    void *workload_state = NULL;
    
    //variables related to job count
    long possiblejobs;
    unsigned long maxjobs = 10000;
    long j;
    
    //process input arguments
    while ((ret = getopt(argc, argv, optstring)) != -1)
    {
        switch(ret)
        {
            case 'W':
                workload_configname = optarg;
                break;
            
            case 'D':
                workload_root_directory = optarg;
                break;
        
            case 'J':
                errno = 0;
                maxjobs = strtoul(optarg, NULL, 10);
                if(errno)
                {
                    perror("main: Failed to parse the J option");
                    ret = -EINVAL;
                    goto exit0;
                }
                break;
        
            case 'f':
                fflag = 1;
                break;
            
            case 'A':
                predictor_name = optarg;
                break;

            case 'p':
                errno = 0;
                period = (uint64_t)strtoul(optarg, NULL, 10);
                if(errno)
                {
                    perror("main: Failed to parse the p option");
                    ret = -EINVAL;
                    goto exit0;
                }

                break;
                
            case 'c':
                errno = 0;
                estimated_mean_exectime = (uint64_t)strtoul(optarg, NULL, 10);
                if(errno)
                {
                    perror("main: Failed to parse the b option");
                    ret = -EINVAL;
                    goto exit0;
                }
                cflag = 1;
                break;

            case 'a':
                errno = 0;
                alpha = strtod(optarg, NULL);
                if(errno)
                {
                    perror("main: Failed to parse the a option");
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
                    perror("main: Failed to parse the L option");
                    ret = -EINVAL;
                    goto exit0;
                }
                
                if((loglevel < SRT_LOGLEVEL_MIN) || (loglevel > SRT_LOGLEVEL_MAX))
                {
                    fprintf(stderr, "main: The log level option (-L) , must have an "
                                    "integer argument between %i and %i inclusive.\n",
                                    SRT_LOGLEVEL_MIN, SRT_LOGLEVEL_MAX);
                    ret = -EINVAL;
                    goto exit0;
                }
                break;
            
            case 'R':
                logfilename = optarg;
                break;

            default:
                fprintf(stderr, "\nUsage: %s %s\n", argv[0], usage_string);
                ret = -EINVAL;
                goto exit0;
        }
    }

    /*Check that there are no additional unrecognized input arguments*/
    if(optind != argc)
    {
        fprintf(stderr, "Usage %s %s\n", argv[0], usage_string);
        ret = -EINVAL;
        goto exit0;
    }

    /*Check that estimated mean execution time and task period is schedulable*/
    if(cflag)
    {
        if( (estimated_mean_exectime > period) || 
            (estimated_mean_exectime == 0)  )
        {
            fprintf(stderr, "main: Invalid estimate of mean (%lu). Must be strictly"
                            " positive and less than the period! \n", 
                            estimated_mean_exectime);
            ret = -EINVAL;
            goto exit0;
        }
    }
    else
    {
        estimated_mean_exectime = (period >> 2); //0.25
    }

    /*Setup the predictor*/
    ret = libPredictor_getPredictor(&predictor, predictor_name);
    if(-1 == ret)
    {
        fprintf(stderr, "main: libPredictor_getPredictor failed with argument \"%s\"\n",
                        predictor_name);
        ret = -1;
        goto exit0;
    }
    
    /*Setup the scheduler*/
    ret = SRT_setup(period, estimated_mean_exectime, alpha, &predictor, &handle, 
                    loglevel, maxjobs, logfilename);
    if(ret)
    {
        fprintf(stderr, "main: SRT_setup failed!\n");
        ret = -1;
        goto exit1;
    }
    
    /*Setup the workload*/
    ret = setup_workload(   workload_configname,
                            workload_root_directory,
                            &fd_cwd, &workload_state, &possiblejobs);
    if(0 != ret)
    {
        fprintf(stderr, "main: setup_workload failed\n");
        workload_state = NULL;
        goto exit2;
    }

    /*Ensure that max jobs is no greater than possiblejobs*/
    maxjobs =   ((possiblejobs > 0) && (possiblejobs < maxjobs))? 
                possiblejobs : maxjobs;

    
    /*Print the configuration parameters and check that the user wants to continue*/
    printf("\nWorkload parameters:\n");
    printf("\tworkload name:\t\t%s\n", workload_name());
    printf("\tworkload root directory:\t%s\n", workload_root_directory);
    printf("\tconfiguration file name:\t%s\n", workload_configname);
    printf("\tmaximum jobs:\t\t%lu\n", maxjobs);

    printf("\nSchedulng Parameters:\n");
    printf("\tscheduling period:\t\t%luus\n", period);
    printf("\testimated mean execution time:\t%lu\n", estimated_mean_exectime);
    printf("\talpha:\t\t\t%f\n", alpha);
    printf("\tbudget_type: \t\t%s\n", (handle.budget_type == 0)? "VIC" : "time");
    printf("\tpredictor name:\t\t%s\n", predictor_name);

    printf("\nLogging Parameters:\n");
    printf("\tLogging level:\t%i\n", loglevel);
    printf("\tLog file:\t\t%s\n\n", logfilename);
    
    if(fflag == 0)
    {
        printf("Type \"q\" to quit or \"Enter\" to continue.\n");
        if(fgetc(stdin) == (int)'q')
        {
            printf("Exiting...\n");
            ret = 0;
            goto exit2;
        }
    }

    //lock memory
    ret = mlockall(MCL_CURRENT);
    if(ret)
    {
        perror("main: mlockall failed");
        ret = -1;
        goto exit2;
    }

    printf("Running ... \n");
    fflush(stdout);

    /*Sleep until the next scheduling-period boundary,
    the first task-period boundary*/
    ret = SRT_sleepTillFirstJob(&handle);
    if(ret)
    {
        fprintf(stderr, "main: SRT_sleepTillFirstJob failed!\n");
        ret = -1;
        goto exit2;
    }
    
    /*The main job loop*/
    for(j = 0; j < maxjobs; j++)
    {
        /*Perform a single job*/
        ret = perform_job(workload_state);
        if(ret < 0)
        {
            fprintf(stderr, "main: perform_job failed for job %li.\n", j);
			break;
        }

        /*Sleep until the next task-period boundary*/
        ret = SRT_sleepTillNextJob(&handle);
        if(ret)
        {
            fprintf(stderr, "main: SRT_sleepTillNextJob failed for job %li.\n", j);
            ret = -1;
            goto exit2;
        }
    }

exit2:
    /*Notify the PBSS module of the end of the SRT task*/
    SRT_close(&handle);
    printf("Closing ... \n");
    
    /*Freeing the workload state is done out of order since the pbsSRT handle should
    really be closed first*/
    if(NULL != workload_state)
    {
        /*Free the workload*/
        workload_uninit(workload_state);
    }
    
exit1:
    /*Free the predictor*/
    libPredictor_freePredictor((&predictor));
    
exit0:
    if(0 != ret)
    {
        fprintf(stderr, "%s: main failed!\n", argv[0]);
    }
    
    return ret;
}

