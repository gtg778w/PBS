#include "jb_mgt.h"
#include "bw_mgt.h"
#include "pbs_ioctl.h"
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
	free_histentry(freeable->history);

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

	initable->history = alloc_histentry();
	if(initable->history == NULL)
	{
		kmem_cache_free(SRT_struct_slab_cache, initable);
		return NULL;
	}

	(initable->history)->pid = current->pid;

	initable->allocation_index = initable->history - history_array;
	allocation_array[initable->allocation_index] = 0;

	ptiming_struct 	= &(initable->timing_struct);
	INIT_TIMING_STRUCT(ptiming_struct);

	initable->task	= current;

	initable->queue_length = 0;

	initable->overuse_count = 0;

	initable->state 	= SRT_OPEN;
	initable->init_mask 	= 0;

	increment_SRT_count();

	return initable;
}

/**********************************************************************

Global variables and functions associated with the sched_rt_jb_mgt file

***********************************************************************/
int jb_mgt_open(struct inode *inode, struct file *filp);
long jb_mgt_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
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
	.unlocked_ioctl	=	jb_mgt_ioctl,
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

#define SRT_PERIOD_INIT_MASK 1
#define SRT_RUNTIME_INIT_MASK 2
#define SRT_HISTLEN_INIT_MASK 4
#define SRT_INITED(strct) (strct->init_mask == (SRT_PERIOD_INIT_MASK | SRT_RUNTIME_INIT_MASK | SRT_HISTLEN_INIT_MASK))

/*FIXME: The entire ioctl command should eventually be removed*/
long jb_mgt_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	unsigned int ret = 0;
	u64 temp;

	struct SRT_struct *SRT_struct;

	printk(KERN_INFO "jb_mgt_ioctl(%d) called by process %d.\n", cmd, current->pid);

	SRT_struct = (struct SRT_struct *)filp->private_data;

	if((SRT_struct->state != SRT_OPEN) && (SRT_struct->state != SRT_CONFIGURED)) 
	{
	    return -EBUSY;
    }

	switch(cmd)
	{
		case PBS_IOCTL_JBMGT_PERIOD:
			printk(KERN_INFO "Setting SRT period to %li.\n", arg);

			//Check that the requested period is a multiple of the scheduling period
			temp = (u64)arg*1000;
			if((temp != 0) && ( (temp % scheduling_period_ns.tv64) == 0))
			{
				SRT_struct->timing_struct.task_period.tv64 = temp;
				temp = temp / scheduling_period_ns.tv64;

				//Check that the task period is sufficiently small
				if( (temp & ((s64)(-1) << 32)) == 0)
				{
					(SRT_struct->history)->sp_per_tp = temp;

					SRT_struct->init_mask = SRT_struct->init_mask | SRT_PERIOD_INIT_MASK;
					break;
				}
				else
				{
					printk(KERN_INFO "Trying to set SRT task period to %li! Too Large!\n", arg);
					ret = -EINVAL;
				}
			}
			else
			{
				printk(KERN_INFO "Trying to set SRT period to %li, not a positive multiple of root period (%li)!\n", 
					 (arg/1000), (unsigned long)(scheduling_period_ns.tv64/1000));
				ret = -EINVAL;
			}
			
			printk(KERN_INFO "SRT period is invalid.\n");
			
			break;

		case PBS_IOCTL_JBMGT_SRT_RUNTIME:
			printk(KERN_INFO "Setting SRT runtime to %li.\n", arg);

			//Check that the runtime is less than the task_period
			temp = (u64)arg*1000;
			temp = temp/((SRT_struct->history)->sp_per_tp);

			if((temp < SRT_struct->timing_struct.task_period.tv64) && (temp > 0))
			{
				allocation_array[SRT_struct->allocation_index] = temp;
				SRT_struct->init_mask = SRT_struct->init_mask | SRT_RUNTIME_INIT_MASK;
				break;
			}
			printk(KERN_INFO "SRT runtime is invalid.\n");
			ret = -EINVAL;
			break;

		case PBS_IOCTL_JBMGT_SRT_HISTLEN:
			//Check that the history length is sufficiently small
			if(arg <= 120)
			{
				printk(KERN_INFO "Setting SRT history length to %li.\n", arg);
				(SRT_struct->history)->history_length = (char)arg;
				SRT_struct->init_mask = SRT_struct->init_mask | SRT_HISTLEN_INIT_MASK;
				break;
			}
			
			printk(KERN_INFO "history length must be less then 121. Got %li\n", arg);			
			ret = -EINVAL;
			break;

		case PBS_IOCTL_JBMGT_START:
			printk(KERN_INFO "Creating cgroup for SRT_task.\n");

			if( SRT_INITED(SRT_struct) )
			{
				SRT_struct->state = SRT_STARTED;
			}
			else
			{
				printk(KERN_INFO "Trying to create cgroup without setting up parameters!\n");
				ret = -EINVAL;
			}
			break;

		default:
			printk(KERN_INFO "Bad ioctl command for sched_rt_job_mgt!!\n");
			ret = -EINVAL;
	}

	return ret;	
}

/*Return the number of PBS tasks, for which information is returned*/
ssize_t jb_mgt_read(struct file* filp, char __user *dst, size_t count, loff_t *offset)
{
	struct SRT_struct *SRT_struct;
	int ret = count;

	SRT_struct = (struct SRT_struct*)(filp->private_data);

    pba_nextjob2(SRT_struct);

    if(sizeof(struct SRT_job_log) == count)
    {
	    if(copy_to_user(dst, &(SRT_struct->log), sizeof(struct SRT_job_log)))
		{
			ret = -EFAULT;
		}
		/*else everything is going smoothely*/ 
	}
	else
	{
	    ret = -EINVAL;
	}

	return ret;
}

ssize_t jb_mgt_write(   struct file *filep, 
                        const char __user *data, 
                        size_t count, 
                        loff_t *offset)
{
    job_mgt_cmd_t cmd;
    ssize_t ret = count;
    
	struct SRT_struct *SRT_struct = (struct SRT_struct*)(filep->private_data);
	
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
            printk(KERN_INFO    "jb_mgt_write: PBS_JBMGT_CMD_SETUP, %lli, %llu.%llu",
                                cmd.args[0],
                                ((u64)(cmd.args[1]) >> 16),
                                ((u64)(cmd.args[1]) & 0xffff));
            
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
            task_period = (u64)cmd.args[0] * 1000;
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

            /*FIXME: remove this*/
		    if( SRT_INITED(SRT_struct) == 0)
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
            /*Insert the command arguments into the history data structure.
            This is done to allow the allocator task access to the execution-time
            prediction performed by the SRT task.*/
            SRT_struct->history->u_c0   = cmd.args[0];
            SRT_struct->history->std_c0 = cmd.args[1];
            SRT_struct->history->u_cl   = cmd.args[2];
            SRT_struct->history->std_cl = cmd.args[3];
            
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
            /*FIXME: Reset the SRT_struct to its state after the last valid call to 
            PBS_JBMGT_CMD_SETUP*/
            SRT_struct->state = SRT_CLOSED;
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

