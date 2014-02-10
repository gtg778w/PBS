#ifndef MOCsample_TIMER_USER_INCLUDE
#define MOCsample_TIMER_USER_INCLUDE
    
    typedef uint64_t u64;
    #include "MOCsample_timer_command.h"
    
    static int MOCsample_timer_start(   int fd,
                                        u64 period,
                                        u64 samples)
    {
        MOCsample_timer_command_t command = (MOCsample_timer_command_t){0};
        ssize_t ret;
        
        command.command = MOCsample_TIMER_COMMAND_START;
        command.arguments[MOCsample_TIMER_COMMAND_START_PERIOD] = period;
        command.arguments[MOCsample_TIMER_COMMAND_START_COUNT]  = samples;
        
        ret = write(fd, &command, sizeof(command));
        if(ret < 0)
        {
            perror("MOCsample_timer_start: write failed");
        }
        else
        {
            ret = 0;
        }
        
        return ret;
    }
    
    static int MOCsample_timer_stop(    int fd,
                                        u64 buffer_length,
                                        MOCsample_timed_sample_t *buffer,
                                        u64 *samples_p)
    {
        MOCsample_timer_command_t command = (MOCsample_timer_command_t){0};
        ssize_t ret;
        
        command.command = MOCsample_TIMER_COMMAND_STOP;
        command.arguments[MOCsample_TIMER_COMMAND_STOP_BUFFSIZ] = buffer_length;
        command.arguments[MOCsample_TIMER_COMMAND_STOP_VLDELMS] = 0;
        command.arguments[MOCsample_TIMER_COMMAND_STOP_BUFFPTR] = (u64)buffer;
        ret = write(fd, &command, sizeof(command));
        if(ret < 0)
        {
            perror("MOCsample_timer_start: write failed");
        }
        else
        {
            *samples_p = command.arguments[MOCsample_TIMER_COMMAND_STOP_VLDELMS];
            ret = 0;
        }
        
        return ret;  
    }
    
#endif
