#include <stdio.h>
#include <string.h>
#include <unistd.h>
/*
read
*/

#include "loadgen.h"
#include "pbspoll_helper.h"

/*
    The purpose of this code is to pole sched_pbs_actv at a reasonably low rate
    to see if the pbs_ul_allocator has stopped running. Additionally, this task can also
    serve as a really low priority background task.
*/

int main(int argc, char** argv)
{
    int ret;

    char *busyloop_enable_flag;
    int busyloop_enable = 0;
    
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
            goto error0;
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
        /*Check if the allocator task is active*/
        ret = pbspoll_check_active();
        if(ret < 0)
        {
            fprintf(stderr, "main: pbspoll_check_active failed!\n");
            goto error0;
        }

        /*If the allocator task is no longer active, break out of the loop*/
        if(0 == ret)
        {
            break;
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

    return 0;
error0:
    fprintf(stderr, "%s: main failed!", argv[0]);
    return -1;
}

