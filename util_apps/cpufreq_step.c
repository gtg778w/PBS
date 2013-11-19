#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <time.h>

#include "cpufreq_helper.h"

char *usage = "[Run duration (s)] [Stepping time (s) ] [Initial steping index]";

int main(int argc, char ** argv)
{
    int ret;
    double duration = 10.0;
    double stepping_time = 1.0;
    long initial_stepping_index = 0;
    long stepping_index;
    
    long i, outer_iterations;
    
    struct  timespec sleep_argument, rem_argument;
    time_t  tv_sec;
    long    tv_nsec;  
    
    char *endptr;
    
    /*Parse input arguments*/
    switch(argc)
    {
        case 4:
            errno = 0;
            initial_stepping_index =  strtol(argv[3], &endptr, 0);
            if((errno != 0) || (endptr == argv[3]))
            {
                fprintf(stderr, "main: strtol failed to parse argument 3");
                goto error0;
            }
            
        case 3:
            errno = 0;
            stepping_time = strtod(argv[2], &endptr);
            if((errno != 0) || (endptr == argv[2]))
            {
                fprintf(stderr, "main: strtod failed to parse argument 2");
                goto error0;
            }
        
        case 2:
            errno = 0;
            duration = strtod(argv[1], &endptr);
            if((errno != 0) || (endptr == argv[1]))
            {
                fprintf(stderr, "main: strtod failed to parse argument 2");
                goto error0;
            }
            
        case 1:
            break;
        
        default:
            fprintf(stderr, "Usage: %s %s\n", argv[0], usage);
            goto error0;
        
    }
    
    /*Get the list of available frequencies on the current system*/
    ret = cpufreq_get_available_frequencies();
    if(ret < 0)
    {
        fprintf(stderr, "main: cpufreq_get_available_frequencies failed\n");
        goto error0;
    }
    
    /*Verify and correct the initial stepping index argument*/
    if(initial_stepping_index > freq_count)
    {
        fprintf(stderr, "main: (WARNING) the system only has %i detected speed settings. "
                        "Setting initial stepping index to %i\n", 
                        freq_count, (freq_count - 1));
                        
        initial_stepping_index = freq_count - 1;
    }
    else
    {
        if(initial_stepping_index < 0)
        {
            fprintf(stderr, "main: the second argument must be a posotive integer\n");
            goto error1;
        }
    }
    
    outer_iterations = (long)round(duration/stepping_time);
    
    /*Setup the nanosleep argument*/
    tv_sec = (time_t)floor(stepping_time);
    tv_nsec= (long)round(    (stepping_time - (double)tv_sec)
                                            * 1000000000.0);
    
    stepping_index = initial_stepping_index;
    for(i = 0; i < outer_iterations; i++)
    {
        ret = cpufreq_change_frequency(stepping_index);
        if(ret < 0)
        {
            fprintf(stderr, "main: cpufreq_change_frequency failed\n");
            goto error1;
        }
        
        stepping_index = (stepping_index + 1) % freq_count;
        
        sleep_argument.tv_sec   = tv_sec;
        sleep_argument.tv_nsec  = tv_nsec;
        do
        {
            ret = nanosleep(&sleep_argument, &rem_argument);
            sleep_argument = rem_argument;
        }while(ret < 0);
    }
    
    /*Free the memory used to store the list of available frequencies*/
    cpufreq_free();
    
    return 0;

error1:
    /*Free the memory used to store the list of available frequencies*/
    cpufreq_free();
error0:
    fprintf(stderr, "%s: main failed\n", argv[0]);
    return -1;
}

