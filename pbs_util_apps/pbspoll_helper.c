#include <stdlib.h>
#include <stdio.h>

int pbspoll_check_active(void)
{
    int ret;
    FILE *file_p;    
    unsigned char read_buffer[3] = {' ', '\n', '\0'};

    /*
        open the status file
    */
    file_p = fopen("/proc/sched_pbs_actv", "r");
    if(file_p == NULL)
    {
        perror("pbspoll_check_active: fopen failed for \"/proc/sched_pbs_actv\"");
        goto error0;
    }

    /*
        read the file
    */
    ret = fread(read_buffer, sizeof(char), 2, file_p);
    if(ret < 2)
    {
        if(ferror(file_p))
        {
            perror("pbspoll_check_active: fread failed");
        }
        else
        {
            fprintf(stderr, "pbspoll_check_active: fread read only %i characters!\n", ret);
        }
        
        goto error1;    
    }

    /*
        close the file
    */
    fclose(file_p);

    /*
        reutrn 1 if '1' was read, otherwise return 0
    */
    return (read_buffer[0] == '1');

error1:
    fclose(file_p);
error0:
    return -1;
}

