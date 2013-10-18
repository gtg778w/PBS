/*****************************************************************************/
//				            Predictor Test
/*****************************************************************************/
/*
    Description: Test accuracy and overhead of predictors from libPredictor.
    
    inputs: 
    
    -I <name>: name of input CSV log file
    -P <name>: name of predictor from libPredictor
    -L <name>: name of output CSV log file
    
    contents of output CSV log file:
    
    actual execution time, predictor output valid, predictor output, predictor overhead
*/

#include <sys/mman.h>

#include <sched.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>

#include <unistd.h>

/*****************************************************************************/
//				            Timing-related Code
/*****************************************************************************/

#include <time.h>

static __inline__ uint64_t getns(void)
{
    struct timespec time;
    int ret;
    uint64_t now_ns;

	ret = clock_gettime(CLOCK_MONOTONIC, &time);
    if(ret == -1)
    {
        perror("clock_gettime failed");
        exit(EXIT_FAILURE);
    }
    

    now_ns = time.tv_sec * 1000000000;
    now_ns = now_ns + time.tv_nsec;

    return now_ns;
}

/*****************************************************************************/
//              Command-line Argument Parsing Related Code
/*****************************************************************************/
/*
    Description: Test accuracy and overhead of predictors from libPredictor.
    
    inputs: 
    
    -I <name>: name of input CSV log file
    -P <name>: name of predictor from libPredictor
    -L <name>: name of output CSV log file
    
    contents of output CSV log file:
    
    actual execution time, predictor output valid, predictor output, predictor overhead
*/

char *usage_string 
	= " [-I <input log file name>] [ -P <predictor name>] [ -L <output log file name>] ";
char *optstring = "I:P:L:r";

static int  parse_commandline_args( int argc, char ** argv,
                                    char **input_logfile_name_pp,
                                    char **predictor_name_pp,
                                    char **output_logfile_name_pp,
                                    int  *realtime_flag_p)
{
    int ret = 0;

    /*Set default values*/
    *input_logfile_name_pp  = "inputlog.csv";
    *predictor_name_pp      = "template";
    *output_logfile_name_pp = "outputlog.csv";
    *realtime_flag_p = 0;

	/* process input arguments */
	while ((ret = getopt(argc, argv, optstring)) != -1)
	{
		switch(ret)
		{
			case 'I':
                *input_logfile_name_pp = optarg;
				break;

            case 'P':
                *predictor_name_pp = optarg;
                break;

            case 'L':
                *output_logfile_name_pp = optarg;
                break;

            case 'r':
                *realtime_flag_p = 1;
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
    else
    {
        ret = 0;
    }

exit0:    
    return ret;
}

/*****************************************************************************/
//				     Read and Parse the input CSV file
/*****************************************************************************/

#define BLOCK_SIZE (1024)

static int parse_input_file(    FILE    *infilep,
                                int64_t **input_array_p,
                                long    *input_array_length_p)
{
    int ret;
    char *line = NULL;
    size_t line_len = 0;
    long long   lineparser_long;

    void    *temp_p;
    int64_t *input_array = NULL;
    long    input_array_length = 0;
    long    i = 0;

    /*Loop through the file reading it line by line.*/
    while((ret = getline(&line, &line_len, infilep)) != -1)
    {
        /*If the index is as large as the array, try to increase the array length by
        at least BLOCK_SIZE*/
        if(i >= input_array_length)
        {
            input_array_length = ((i + (2 * BLOCK_SIZE) - 1)/BLOCK_SIZE) * BLOCK_SIZE;
            temp_p = realloc(input_array, sizeof(int64_t) * input_array_length);
            if(NULL == temp_p)
            {
                fprintf(stderr, "parse_input_file: realloc failed  to allocate %lubytes",
                                (sizeof(int64_t) * input_array_length));
                perror("");
                goto error0;
            }
            else
            {
                input_array = (int64_t*)temp_p;
            }
        }
    
        /*Try to parse the line just read*/
        errno = 0;
        ret = sscanf(line, "%lli, ", &lineparser_long);
        if(1 != ret)
        {
            /*If nothing was read, check if this is the end of file*/
            if(!feof(infilep))
            {
                fprintf(stderr, "parse_input_file: sscanf failed to read element %li", i);
                perror("");
                goto error0;
            }
            else
            {
                break;
            }
        }
        else
        {
            /*Next input read successfully. Store the value and increment the index*/
            input_array[i] = lineparser_long;
            i++;
        }
    }
    
    /*Check if the reason the getline failed was because of an end of file marker,
    which would not consitute an error.*/
    if(!feof(infilep))
    {
        perror("parse_input_file: getline failed");
        goto error0;
    }
    
    /*Set the output variables*/
    *input_array_p = input_array;
    *input_array_length_p = i;
    
    return 0;
    
error0:
    if(NULL != input_array)
    {
        free(input_array);
    }
    
    if(NULL != line)
    {
        free(line);
    }
    
    return -1;
}

/*****************************************************************************/
//		               Allocate Memory for the log
/*****************************************************************************/

static int extend_log_memory(   unsigned long   array_length,
                                int64_t         **input_array_p,
                                int64_t         **prediction_array_p,
                                int64_t         **prediction_valid_array_p,
                                int64_t         **overhead_array_p)
{
    int ret = 0;
    int64_t *realloced_memory_p;
    
    realloced_memory_p = (int64_t*)realloc( *input_array_p, 
                                            sizeof(int64_t)*(4*array_length));
    if(NULL == realloced_memory_p)
    {
        perror("extend_log_memory: realloc failed");
        ret = -1;
    }
    else
    {
        *input_array_p      = realloced_memory_p;
        *prediction_array_p = &((*input_array_p)[array_length]);
        *prediction_valid_array_p   = &((*prediction_array_p)[array_length]);
        *overhead_array_p   = &((*prediction_valid_array_p)[array_length]);
    }
    
    return ret;
}

/*****************************************************************************/
//			Setup and cleanup real-time properties to reduce interference
//          to timing
/*****************************************************************************/

static int set_realtime(void)
{
    int ret;

    pid_t my_pid;
    int max_priority;
    struct sched_param sched_param;
    
    /*Lock all available memory*/
    ret = mlockall(MCL_CURRENT | MCL_FUTURE);
    if(0 != ret)
    {
        perror("set_realtime: mlockall failed");
        goto error0;
    }
    
    /*Set the scheduler to real-time FIFIO and maximum priority*/
    my_pid = getpid();
    
    max_priority = sched_get_priority_max(SCHED_FIFO);
    if(max_priority == -1)        
    {
        perror("set_realtime: sched_get_priority_max failed");
        goto error1;
    }

    sched_param.sched_priority = max_priority;
    ret = sched_setscheduler(my_pid, SCHED_FIFO, &sched_param);
    if(ret == -1)
    {
        perror("set_realtime:: sched_setsceduler failed");
        goto error1;
    }
    
    return 0;
    
error1:
    munlockall();
error0:
    return -1;
}

static int reset_realtime(void)
{
    int ret = 0, ret2;

    pid_t my_pid;
    struct sched_param sched_param;
    
    /*Set the scheduler to real-time FIFIO and maximum priority*/
    my_pid = getpid();
    
    sched_param.sched_priority = 0;
    ret = sched_setscheduler(my_pid, SCHED_OTHER, &sched_param);
    if(ret == -1)
    {
        perror("set_realtime: sched_setsceduler failed");
    }    
    
    /*Unlock all memory*/
    ret2 = munlockall();
    if(ret2 == -1)
    {
        perror("set_realtime: munlockall failed");
        ret = -1;
    }

    return ret;
}

/*****************************************************************************/
//				       Go Through the Prediction Loop
/*****************************************************************************/

#include "libPredictor.h"

static void prediction_loop(libPredictor_t  *predictor_p,
                            unsigned long   array_length,
                            int64_t         *input_array,
                            int64_t         *prediction_array,
                            int64_t         *prediction_valid_array,
                            int64_t         *overhead_array)
{
    int ret;
    int i;
    int64_t start_time, end_time;
    int64_t std_c0, u_cl, std_cl;
    
    for(i = 0; i < array_length; i++)
    {
        
        start_time = getns();
        ret = predictor_p->update(  predictor_p->state, input_array[i], 
                                    &(prediction_array[i]), &std_c0, &u_cl, &std_cl);
        end_time   = getns();
        
        prediction_valid_array[i] = (ret == -1)? 0: 1;
        overhead_array[i] = end_time - start_time;
    }
}

/*****************************************************************************/
//				       Write to the output CSV file
/*****************************************************************************/

static int output_log(  FILE*           outfilep,
                        unsigned long   array_length,
                        int64_t         *input_array,
                        int64_t         *prediction_array,
                        int64_t         *prediction_valid_array,
                        int64_t         *overhead_array)
{
    int ret;
    int i;
    
    for(i = 0; i < array_length; i++)
    {
        ret = fprintf(outfilep, "%lli, %lli, %lli, %lli\n", 
                                (long long int)input_array[i],
                                (long long int)prediction_valid_array[i],
                                (long long int)prediction_array[i],
                                (long long int)overhead_array[i]);
        if(ret < 0)
        {
            perror("output_log: fprintf failed");
            return -1;
        }
    }
    
    return 0;
}

/*****************************************************************************/

char *clear_line =  "\r         "
                    "          "
                    "          "
                    "          "
                    "          "
                    "          "
                    "          "
                    "          "
                    "          "
                    "          "
                    "          "
                    "          ";

int main(int argc, char ** argv)
{
    int ret;
    
    char *infile_name;
    FILE *infilep;
    
    char *outfile_name;
    FILE *outfilep;    
    
    char *predictor_name;
    libPredictor_t predictor;
    
    int64_t *input_array;
    long     input_length;
    int64_t *prediction_valid_array;
    int64_t *prediction_array;
    int64_t *overhead_array;
    
    int realtime_flag = 0;
    
    /*Parse command-line arguments*/
    printf("\n%s: Parsing command-line arguments ...", argv[0]);
    
    ret = parse_commandline_args( argc, argv,
                                  &infile_name,
                                  &predictor_name,
                                  &outfile_name,
                                  &realtime_flag);
    if(ret != 0)
    {
        fprintf(stderr, "\nmain: parse_commandline_args failed!\n");
        goto error0;
    }
    
    /*Setup file handles and the predictor*/
    printf("%s", clear_line);
    printf("\r%s: Setting up file handles and the predictor ...", argv[0]);
    fflush(stdout);
    
    infilep = fopen(infile_name, "r");
    if(NULL == infilep)
    {
        fprintf(stderr, "\nFailed to open %s for reading!\n", infile_name);
        perror("main: fopen failed");
        goto error0;
    }
    
    outfilep = fopen(outfile_name, "w");
    if(NULL == outfilep)
    {
        fprintf(stderr, "\nFailed to open %s for writing!\n", outfile_name);
        perror("main: fopen failed");
        goto error1;
    }
    
    ret = libPredictor_getPredictor(&predictor, predictor_name);
    if(ret != 0)
    {
        fprintf(stderr, "\nmain: libPredictor_getPredictor failed\n");
        goto error2;
    }
    
    /*Parse the input file*/
    printf("%s", clear_line);
    printf("\r%s: Parsing the input file ...", argv[0]);
    fflush(stdout);
        
    ret = parse_input_file( infilep,
                            &input_array,
                            &input_length);
    if(ret != 0)
    {
        fprintf(stderr, "\nmain: parse_input_file failed\n");
        goto error3;
    }

    /*Extend the log memory*/
    printf("%s", clear_line);
    printf("\r%s: Allocating log memory ...", argv[0]);
    fflush(stdout);
    
    ret = extend_log_memory(    input_length,
                                &input_array,
                                &prediction_array,
                                &prediction_valid_array,
                                &overhead_array);
    if(ret != 0)
    {
        fprintf(stderr, "\nmain: extend_log_memory failed!\n");
        goto error4;
    }

    /*Setup real-time parameters*/
    if(realtime_flag != 0)
    {
        printf("\r%s: Setting up real-time properties for timing ...", argv[0]);
        ret = set_realtime();
        if(ret != 0)
        {
            fprintf(stderr, "\nWARNING: main: set_realtime failed\n");
            realtime_flag = 0;
        }
    }
    
    /*Process the input*/
    printf("%s", clear_line);
    printf("\r%s: Processing the input file ...", argv[0]);
    fflush(stdout);
    
    prediction_loop(    &predictor,
                        input_length,
                        input_array,
                        prediction_array,
                        prediction_valid_array,
                        overhead_array);
                            
    /*Remove real-time properties*/
    if(realtime_flag != 0)
    {
        printf("\r%s: Resetting real-time properties ...", argv[0]);
        ret = reset_realtime();
        if(ret != 0)
        {
            fprintf(stderr, "\nWARNING: main: reset_realtime failed\n");
        }
    }
        
    /*Writing to output log*/
    printf("%s", clear_line);
    printf("\r%s: Writing to output file ...", argv[0]);
    fflush(stdout);
    
    ret = output_log(   outfilep,
                        input_length,
                        input_array,
                        prediction_array,
                        prediction_valid_array,
                        overhead_array);
    if(ret != 0)
    {
        fprintf(stderr, "main: output_log failed!\n");
        goto error4;
    }
    
    /*Free allocated memory, predictor state and close files*/
    printf("%s", clear_line);
    printf("\r%s: Cleaning up ...", argv[0]);
    fflush(stdout);
    
    free(input_array);
    libPredictor_freePredictor((&predictor));
    fclose(outfilep);
    fclose(infilep);
    
    /*Free allocated memory and close files*/
    printf("%s", clear_line);
    printf("\r%s: Done!\n\n", argv[0]);
    fflush(stdout);
        
    return 0;

error4:
    free(input_array);
error3:
    libPredictor_freePredictor((&predictor));
error2:
    fclose(outfilep);
error1:
    fclose(infilep);
error0:
    fprintf(stderr, "%s: main failed!\n\n", argv[0]);
    return -1;
}

