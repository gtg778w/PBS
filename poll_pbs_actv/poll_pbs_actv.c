#include <stdio.h>
#include <string.h>
#include <unistd.h>
/*
read
*/

#include "loadgen.h"


/*
    The purpose of this code is to pole sched_pbs_actv at a reasonably low rate
    to see if the pbs_ul_allocator has stopped running. Additionally, this task can also
    serve as a really low priority background task.
*/

int main(int argc, char** argv)
{
    int ret;
    FILE* file_p;

    char *busyloop_enable_flag;
    int busyloop_enable = 0;
    
    unsigned char read_buffer[3] = {' ', '\n', '\0'};
    
    unsigned long workload_loopcount;
    int32_t workload_state = 0;

    /*Check command line arguments if application is setup to busy loop*/
    if(argc > 1)
    {
        busyloop_enable_flag = argv[1];
        if(strcmp(busyloop_enable_flag, "-I") == 0)
        {
            busyloop_enable = 1;
        }
        else
        {
            fprintf(stderr, "Usage: %s [-I]\n\t-I: busy loop when idle\n\n", argv[0]);
        }
    }

    /*If the application is setup to busyloop, callibrate the busy loop*/
    if(1 == busyloop_enable)
    {
        /*Callibrate the busy looping code*/
        callibrate(20000000);
        
        workload_loopcount = ipms * 500;
    }

    for(;;)
    {
        /*
            open the status file
        */
        file_p = fopen("/proc/sched_pbs_actv", "r");
        if(file_p == NULL)
        {
            perror("Failed to open \"/proc/sched_pbs_actv\"");
            return -1;
        }

        /*
            read the file
        */
        ret = fread(read_buffer, sizeof(char), 2, file_p);
        if(ret < 2)
        {
            if(ferror(file_p))
                perror("fread failed");
            else
                fprintf(stderr, "fread read only %i characters!\n", ret);

            fclose(file_p);
            return -1;    
        }

        /*
            close the file
        */
        fclose(file_p);

        /*
            check if the allocator is inactive, then exit
        */
        if(read_buffer[0] == '0')
        {
            return 0;
        }

        /*
            sleep for half a second
        */
        if(0 != busyloop_enable)
        {
            workload_state = work_function(workload_loopcount, workload_state);
        }
        else
        {
            usleep(500000);
        }
    }

    /*
        control should never reach here
        this is a canary
    */
    fprintf(stderr, "control reached after the loop!");
    return -1;
}

