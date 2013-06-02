#include "jb_mgt.h"
#include "bw_mgt.h"
#include "pba.h"
/*
pba_nextjob2
*/

/**********************************************************************

Various global variables and functions

***********************************************************************/

struct	kmem_cache *SRT_struct_slab_cache = NULL;

/**********************************************************************
Functions for initializing various data structures
***********************************************************************/

static void free_SRT_struct(struct SRT_struct *freeable)
{
	free_loaddata(freeable->loaddata);

    /*FIXME*/
    printk(KERN_INFO    "free_SRT_struct called by task (%d), "
                        "cumulative budget %llins, "
                        "cumulative budget before saturation %llins, "
                        "total consumed budget %llins, "
                        "total misses %llu\n", 
                        freeable->task->pid, 
                        freeable->summary.cumulative_budget,
                        freeable->summary.cumulative_budget_sat,
                        freeable->summary.consumed_budget,
                        freeable->summary.total_misses);

	kmem_cache_free(SRT_struct_slab_cache, freeable);

	decrement_SRT_count();
}

struct SRT_struct *allocate_SRT_struct(void)
{
	struct SRT_struct *initable;
	struct SRT_timing_struct *ptiming_struct;

	initable= kmem_cache_alloc(SRT_struct_slab_cache, GFP_KERNEL);
	if(initable == NULL)
	{
		printk(KERN_INFO "Failed to allocate memory for jb_mgt_struct_t in jb_mgt_open!\n");
		return NULL;
	}

    /*zero out the summary structure*/
    initable->summary = (struct SRT_summary_s){0, 0, 0, 0};

	initable->loaddata = alloc_loaddata();
	if(initable->loaddata == NULL)
	{
		kmem_cache_free(SRT_struct_slab_cache, initable);
		return NULL;
	}

	(initable->loaddata)->pid = current->pid;

	initable->allocation_index = initable->loaddata - loaddata_array;
	allocation_array[initable->allocation_index] = 0;

	ptiming_struct 	= &(initable->timing_struct);
	INIT_TIMING_STRUCT(ptiming_struct);

	initable->task	= current;

	initable->queue_length = 0;

	initable->overuse_count = 0;

	initable->state 	= SRT_OPEN;

	increment_SRT_count();

	return initable;
}

/**********************************************************************

Global variables and functions associated with the sched_rt_jb_mgt file

***********************************************************************/
int jb_mgt_open(struct inode *inode, struct file *filp);
ssize_t jb_mgt_read(struct file* filp, char __user *dst, size_t count, loff_t *offset);
ssize_t jb_mgt_write(   struct file *fileh, 
                    const char __user *data, 
                    size_t count, 
                    loff_t *offset);
int jb_mgt_release(struct inode *inode, struct file *filp);

/*The procfs file that is accessed by the bandwidth allocation process*/
char*  jb_mgt_file_name = "sched_rt_jb_mgt";
struct proc_dir_entry* p_jb_mgt_file;
struct file_operations jb_mgt_fops = {
	.owner		=	THIS_MODULE,
	.read 		=	jb_mgt_read,
	.write      =   jb_mgt_write,
	.open 		=	jb_mgt_open,
	.release	=	jb_mgt_release	
};

int jb_mgt_open(struct inode *inode, struct file *filp)
{
	int ret = 0;

	//initialize an allocator cache for the pbs_srt_struct
	printk(KERN_INFO "jb_mgt_open called by process %d.\n", current->pid);

	if(allocator_state != ALLOCATOR_LOOP)
		return -EBUSY;

	// Allocate the per job structure
	filp->private_data = allocate_SRT_struct();
	if(filp->private_data == NULL)
	{
		ret = -ENOMEM;
	}

	return ret;
}

/*Return the number of PBS tasks, for which information is returned*/
ssize_t jb_mgt_read(struct file* filp, char __user *dst, size_t count, loff_t *offset)
{
	struct SRT_struct *SRT_struct;

	SRT_struct = (struct SRT_struct*)(filp->private_data);

    pba_nextjob2(SRT_struct);

    count = (count > sizeof(struct SRT_job_log))?
            sizeof(struct SRT_job_log) : count;
    
    if(copy_to_user(dst, &(SRT_struct->log), count))
	{
		count = -EFAULT;
	}

	return count;
}

ssize_t jb_mgt_write(   struct file *filep, 
                        const char __user *data, 
                        size_t count, 
                        loff_t *offset)
{
    job_mgt_cmd_t cmd;
    ssize_t ret = count;
    
	struct SRT_struct *SRT_struct = (struct SRT_struct*)(filep->private_data);
	struct SRT_summary __user *summary_p = NULL;
	
	u64 task_period;
    u64 sp_per_tp; /*reservation periods in a task period*/
    
    if(sizeof(job_mgt_cmd_t) != count)
    {
        printk(KERN_INFO    "The argument written through jb_mgt_write by process %d "
                            "is not of valid size.", current->pid);
        ret = -EINVAL;
        goto exit0;
    }

    if( copy_from_user( &cmd, data, sizeof(job_mgt_cmd_t)))
    {
        ret = -EFAULT;
        goto exit0;
    }
    
    switch(cmd.cmd)
    {
        case PBS_JBMGT_CMD_SETUP:
            printk(KERN_INFO    "jb_mgt_write: PBS_JBMGT_CMD_SETUP, %lli",
                                cmd.args[0]);
            
            /*The PBS_JBMGT_CMD_SETUP command should only be 
            issued in the SRT_OPEN state or SRT_CONFIGURED state*/
            if((SRT_OPEN != SRT_struct->state) && (SRT_CONFIGURED != SRT_struct->state))
            {
                printk(KERN_INFO "Invalid attempt to issue PBS_JBMGT_CMD_SETUP command "
                                 "while in a state other than \"SRT_OPEN\" or "
                                 "\"SRT_CONFIGURED\" by process %d.\n", current->pid);
                ret = -EBUSY;
                goto exit0;
            }

            /*Check that the task period actually makes sense*/
            task_period = (u64)cmd.args[0];
            sp_per_tp   = task_period / scheduling_period_ns.tv64;
            
            if( (task_period < 0) || ((task_period % scheduling_period_ns.tv64) != 0))
            {
                printk(KERN_INFO    "Invalid value passed to PBS_JBMGT_CMD_SETUP "
                                    "command for the task period by process %d. "
                                    "The task period must be a strictly positive"
                                    "multiple of the reservation period.", 
                                    current->pid);
                ret = -EINVAL;
                goto exit0;
            }
                
            /*Check that the task period is sufficiently small*/
			if( (sp_per_tp & ((s64)(-1) << 32)) != 0)
			{
                printk(KERN_INFO    "Invalid value passed to PBS_JBMGT_CMD_SETUP "
                                    "command for the task period by process %d. "
                                    "The passed value is too large.", 
                                    current->pid);
			    ret = -EINVAL;
			    goto exit0;
			}
			
			SRT_struct->timing_struct.task_period.tv64 = task_period;
			(SRT_struct->loaddata)->sp_per_tp = sp_per_tp;
			/*The conditoins and passed values are valid*/
			SRT_struct->state = SRT_CONFIGURED;
            break;
        
        case PBS_JBMGT_CMD_START:
            printk(KERN_INFO    "jb_mgt_write: PBS_JBMGT_CMD_START");
            
            /*The PBS_JBMGT_CMD_START command should only be 
            issued in the SRT_START state*/
            if(SRT_CONFIGURED != SRT_struct->state)
            {
			    printk(KERN_INFO    "Attempt to issue PBS_JBMGT_CMD_START command "
			                        "without setting up parameters by process %d.", 
			                        current->pid);
                ret = -EINVAL;
                goto exit0;
            }
		    
		    SRT_struct->state = SRT_STARTED;
            break;
            
        case PBS_JBMGT_CMD_NEXTJOB:
            /*Insert the command arguments into the loaddata structure.
            This is done to allow the allocator task access to the execution-time
            prediction performed by the SRT task.*/
            SRT_struct->loaddata->u_c0   = cmd.args[0];
            SRT_struct->loaddata->var_c0 = cmd.args[1];
            SRT_struct->loaddata->u_cl   = cmd.args[2];
            SRT_struct->loaddata->var_cl = cmd.args[3];
            
            if(SRT_STARTED == SRT_struct->state)
            {                                                
                printk(KERN_INFO    "The PBS_JBMGT_CMD_NEXTJOB command issued for the "
                                    "first time by process %d.", current->pid);
                preempt_disable();
			    ret = first_sleep_till_next_period(&(SRT_struct->timing_struct));
			    preempt_enable();
			    if(ret==0)
			    {
				    SRT_struct->state = SRT_LOOP;
				    ret = count;
			    }
			    else
			    {
			        printk(KERN_INFO    "\"first_sleep_till_next_period\" failed for "
                                        "process %d.", current->pid);
                    /*ret has already been set by the fialing function*/
                    goto exit0;
			    }
            }
            else
            {
                if(SRT_LOOP == SRT_struct->state)
                {
			        preempt_disable();
			        ret = sleep_till_next_period(&(SRT_struct->timing_struct));
                    preempt_enable();
                    if(ret==0)
			        {
			            ret = count;
			        }
    			    else
			        {
			            printk(KERN_INFO    "\"sleep_till_next_period\" failed for "
                                            "process %d.", current->pid);
                        /*ret has already been set by the fialing function*/
                        goto exit0;
			        }
                }
                else
                {
                    printk(KERN_INFO "Invalid Attempt to issue PBS_JBMGT_CMD_NEXTJOB "
                                     "command by process %d. PBS_JBMGT_CMD_NEXTJOB "
                                     "command can only be issued from the "
                                     "\"SRT_STARTED\" state or SRT_LOOP state.\n", 
                                     current->pid);
                    ret = -EINVAL;
                    goto exit0;
                }
            }
            break;
        
        case PBS_JBMGT_CMD_STOP:
            printk(KERN_INFO    "jb_mgt_write: PBS_JBMGT_CMD_STOP");
	        if(SRT_LOOP == SRT_struct->state)
	        {
		        preempt_disable();
		        //remove from the period timer list and stop budget enforcement
		        remove_from_timing_queue(&(SRT_struct->timing_struct));
		        SRT_struct->state = SRT_CLOSED;
		        preempt_enable();
            }
            
            SRT_struct->state = SRT_CLOSED;
            break;
        
        case PBS_JBMGT_CMD_GETSUMMARY:
            printk(KERN_INFO    "jb_mgt_write: PBS_JBMGT_CMD_GETSUMMARY");
            summary_p = (struct SRT_summary __user *)cmd.args[0];
            if( copy_to_user(   summary_p, &(SRT_struct->summary), 
                                sizeof(struct SRT_summary_s)) )
		    {
			    ret = -EFAULT;
			    goto exit0;
		    }
            break;
            
        default:
            printk(KERN_INFO    "Invalid cmd code in job_mgt_cmd_t structure passed to "
                                "jb_mgt_write by process %d.", current->pid);
            ret = -EINVAL;
            goto exit0;
    }

exit0:
    return ret;
}

int jb_mgt_release(struct inode *inode, struct file *filp)
{
	struct SRT_struct *freeable;

	freeable = filp->private_data;

	printk(KERN_EMERG "jb_mgt_release called by process %d.\n", current->pid);

	switch(freeable->state)
	{
		case SRT_LOOP:

			preempt_disable();
			//remove from the period timer list
			remove_from_timing_queue(&(freeable->timing_struct));
			freeable->state = SRT_CLOSED;
			preempt_enable();

		case SRT_STARTED:
		case SRT_CONFIGURED:
		case SRT_OPEN:
		case SRT_CLOSED:
			free_SRT_struct(freeable);
	}

	return 0;
}

/**********************************************************************

(Un)initialization code associated with the sched_rt_jb_mgt file

***********************************************************************/

int init_jb_mgt(void)
{
	int returnable = 0;

	/*Create the file in the root of the profcs directory, initially with no permissions for others*/
	p_jb_mgt_file = create_proc_entry(jb_mgt_file_name, 0600, NULL);
	if(p_jb_mgt_file == NULL)
	{
		returnable = -ENOMEM;
		goto error;
	}
	p_jb_mgt_file->data = NULL;
	p_jb_mgt_file->proc_fops = &jb_mgt_fops;
	p_jb_mgt_file->mode = 0666;

	/*Setup a lookaside cache for the sleep_queue_entry_t*/
	SRT_struct_slab_cache = KMEM_CACHE(SRT_struct, SLAB_HWCACHE_ALIGN);
	if(SRT_struct_slab_cache == NULL)
		returnable = -ENOMEM;

	reset_SRT_count();
error:
	return returnable;
}

int uninit_jb_mgt(void)
{
	int returnable = 0;
	remove_proc_entry(jb_mgt_file_name, NULL);

	/*Release the slab cache allocator*/
	kmem_cache_destroy(SRT_struct_slab_cache);

	return returnable;
}

