#include <stdio.h>
#include <unistd.h>

/*
    The purpose of this code is to pole sched_pbs_actv at a reasonably low rate
    to see if the pbs_ul_allocator has stopped running.
*/

int main(void)
{
    int ret;
    FILE* file_p;

    unsigned char read_buffer[3] = {' ', '\n', '\0'};

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
        usleep(500000);
    }

    /*
        control should never reach here
        this is a canary
    */
    fprintf(stderr, "control reached after the loop!");
    return -1;
}

