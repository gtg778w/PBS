#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <time.h>

#include "cpufreq_helper.h"

char *usage =   "[Run duration (s)] [Oscillation Period (s) ] "
                "[Min speed (0.0-1.0)] [Max speed (0.0-1.0)]";

int main(int argc, char ** argv)
{
    int ret;
    double duration = 10.0;
    double oscillation_period = 1.0;
    double min_speed = 0.0;
    double max_speed = 1.0;
    
    long max_index, min_index;
    
    long i, outer_iterations;
    
    struct  timespec sleep_argument, rem_argument;
    time_t  tv_sec;
    long    tv_nsec;  
    
    char *endptr;
    
    /*Parse input arguments*/
    switch(argc)
    {
        case 5:
            errno = 0;
            max_speed =  strtod(argv[4], &endptr);
            if((errno != 0) || (endptr == argv[4]))
            {
                fprintf(stderr, "main: strtod failed to parse argument 4");
                goto error0;
            }
            
            if((max_speed > 1.0) || (max_speed < 0.0))
            {
                fprintf(stderr, "main: argument 4 must be a value between 0.0 and 1.0");
                goto error0;
            }
        
        case 4:
            errno = 0;
            min_speed =  strtod(argv[3], &endptr);
            if((errno != 0) || (endptr == argv[3]))
            {
                fprintf(stderr, "main: strtod failed to parse argument 3");
                goto error0;
            }
            
            if((min_speed > 1.0) || (min_speed < 0.0))
            {
                fprintf(stderr, "main: argument 3 must be a value between 0.0 and 1.0");
                goto error0;
            }
            
        case 3:
            errno = 0;
            oscillation_period = strtod(argv[2], &endptr);
            if((errno != 0) || (endptr == argv[2]))
            {
                fprintf(stderr, "main: strtod failed to parse argument 2");
                goto error0;
            }
        
            if(oscillation_period <= 0.0)
            {
                fprintf(stderr, "main: argument 2 must be a strictly positive value");
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
            
            if(duration < 0.0)
            {
                fprintf(stderr, "main: argument 1 must be a non-negative value");
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
    
    /*Compute the max index and min index*/
    max_index = (freq_count-1) - (long)(max_speed * (double)(freq_count-1));
    min_index = (freq_count-1) - (long)(min_speed * (double)(freq_count-1));
    
    outer_iterations = (long)round(duration/oscillation_period);
    
    /*Setup the nanosleep argument*/
    tv_sec = (time_t)floor(oscillation_period/2.0);
    tv_nsec= (long)round(    ((oscillation_period/2.0) - (double)tv_sec)
                             * 1000000000.0);
    
    for(i = 0; i < outer_iterations; i++)
    {
        /*Set the high frequency and sleep*/
        ret = cpufreq_change_frequency(max_index);
        if(ret < 0)
        {
            fprintf(stderr, "main: cpufreq_change_frequency failed\n");
            goto error1;
        }
        
        sleep_argument.tv_sec   = tv_sec;
        sleep_argument.tv_nsec  = tv_nsec;
        do
        {
            ret = nanosleep(&sleep_argument, &rem_argument);
            sleep_argument = rem_argument;
        }while(ret < 0);
        
        /*Set the low frequency and sleep*/
        ret = cpufreq_change_frequency(min_index);
        if(ret < 0)
        {
            fprintf(stderr, "main: cpufreq_change_frequency failed\n");
            goto error1;
        }
        
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

