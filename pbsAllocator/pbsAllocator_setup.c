#include <stdlib.h>
#include <stdio.h>
/*
fprintf
stderr
perror
*/

#include <stdint.h>
/*
int32_t
int64_t
uint32_t
uint64_t
*/

#include <fcntl.h>
/*
open
write
*/

#include <sys/types.h>
#include <unistd.h>
/*
read
getpid
*/

#include <sched.h>

#include <sys/mman.h>
/*
memory mapping stuff
*/

#include "pbsAllocator.h"

SRT_loaddata_t          *loaddata_array;
loaddata_list_header_t  *loaddata_list_header;
uint64_t                *allocation_array;

int allocator_setup(uint64_t scheduling_period,
                    uint64_t allocator_bandwidth)
{
    int proc_file;
    int retval;

    pid_t  mypid;
    struct sched_param my_sched_params;
    int min_priority;

    bw_mgt_cmd_t cmd;

    /*Setup the memory for storing budget allocations before saturation. The space 
    required is the same as that of the allocation mapping pages*/
    presaturation_budget_array = (double*)malloc(ALLOC_SIZE);
    if(NULL == presaturation_budget_array)
    {
        perror("allocator_setup: malloc failed for the presaturation_budget_array!");
        retval = -1;
        goto exit0;
    }
    
    if(mlock(presaturation_budget_array, ALLOC_SIZE) < 0)
    {
        perror("allocator_setup: mlock failed for the presaturation_budget_array!");
        free(presaturation_budget_array);
        presaturation_budget_array = NULL;
        retval = -1;
        goto exit0;
    }

    /*Need to change the scheduling policy of the task to real-time 
      (SCHED_FIFO)*/

    /*Determine own PID*/
    mypid =  getpid();

    /*Determine the lowest valid priority for the given scheduling policy*/
    min_priority = sched_get_priority_min(SCHED_FIFO);
    
    /*Set the priority*/
    my_sched_params.sched_priority = min_priority+1;

    /*Try to change the scheduling policy and priority*/
    retval = sched_setscheduler(mypid, SCHED_FIFO, &my_sched_params);
    if(retval)
    {
        perror("allocator_setup: sched_setscheduler failed");
        retval = -1;
        goto exit0;
    }

    /*Perform setup operations needed to interact with the pbs_allocator
      module*/
    proc_file = open("/proc/sched_rt_bw_mgt", O_RDWR);
    if(proc_file == -1)
    {
        perror("allocator_setup: Failed to open \"/proc/sched_rt_bw_mgt\"");
        retval = proc_file;
        goto exit0;
    }

    //setup the loaddata mapping
    loaddata_array = mmap(NULL, LOADDATALIST_SIZE, 
                         (PROT_READ), MAP_SHARED, 
                         proc_file, (LOADDATALIST_PAGEOFFSET * PAGE_SIZE));
    if(loaddata_array == MAP_FAILED)
    {
        perror("allocator_setup: Failed to map loaddata_array");
        retval = -1;
        goto exit0;
    }

    loaddata_list_header = (loaddata_list_header_t*)loaddata_array;

    //setup the model adapters
    
    //setup the allocations mapping
    allocation_array = mmap(NULL, ALLOC_SIZE, 
                            (PROT_READ | PROT_WRITE), MAP_SHARED, 
                            proc_file, (ALLOC_PAGEOFFSET * PAGE_SIZE));
    if(allocation_array == MAP_FAILED)
    {
        perror("allocator_setup: Failed to map allocation_array");
        retval = -1;
        goto exit0;
    }

    cmd.cmd = PBS_BWMGT_CMD_SETUP_START;
    cmd.args[0] = budget_type;
    cmd.args[1] = scheduling_period;
    cmd.args[2] = allocator_bandwidth;
    retval = write(proc_file, &cmd, sizeof(cmd));
    if(retval != sizeof(cmd))
    {
        perror("allocator_setup: Failed to write PBS_BWMGT_CMD_SETUP_START command.");
        goto exit0;
    }

    retval = proc_file;
    
exit0:
    return retval;
}

int allocator_close(int proc_file)
{
    bw_mgt_cmd_t cmd;
    int retval;

    cmd.cmd = PBS_BWMGT_CMD_STOP;
    retval = write(proc_file, &cmd, sizeof(cmd));
    if(retval != sizeof(cmd))
    {
        perror("allocator_close: Failed to write PBS_BWMGT_CMD_STOP command.");
        goto exit0;
    }

    if(munmap(loaddata_array, LOADDATALIST_SIZE) != 0)
    {
        perror("Failed to unmap loaddata pages!\n");
        retval = -1;
        goto exit0;
    }

    if(munmap(allocation_array, ALLOC_SIZE) != 0)
    {
        perror("Failed to unmap allocation pages!\n");
        retval = -1;
        goto exit0;
    }

    if(close(proc_file) != 0)
    {
        perror("Failed to close proc_file!\n");
        retval = -1;
        goto exit0;
    }

    retval = 0;
exit0:
    return 0;
}

