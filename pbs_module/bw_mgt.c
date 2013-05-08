/**********************************************************************

Global variables and functions associated with the sched_rt_bw_mgt file
and allocator task.

***********************************************************************/

#include "bw_mgt.h"
#include "jb_mgt.h"
#include "pbs_ioctl.h"
#include "pbs_mmap.h"

/**********************************************************************

Global variables and functions associated with the allocator task.

***********************************************************************/

unsigned char	allocator_state = MODULE_LOADED;


/**********************************************************************

Function headers and structure deffinitions for the sched_bw_mgt file.

***********************************************************************/

ssize_t bw_mgt_first_read(struct file* file, char __user *dst, size_t count, loff_t *offset);
ssize_t bw_mgt_read(struct file* file, char __user *dst, size_t count, loff_t *offset);
int bw_mgt_mmap(struct file *file, struct vm_area_struct *vmas);
long bw_mgt_ioctl(struct file * file, unsigned int cmd, unsigned long arg); //struct inode * inode,
int bw_mgt_open(struct inode *inode, struct file *file);
int bw_mgt_release(struct inode *inode, struct file *filp);

/*The procfs file that is accessed by the bandwidth allocation process*/
char*  bw_mgt_file_name = "sched_rt_bw_mgt";
struct proc_dir_entry* p_bw_mgt_file;
struct file_operations bw_mgt_fops = {
	.owner		=	THIS_MODULE,
	.read 		=	bw_mgt_first_read,
	.mmap			=	bw_mgt_mmap,
	.open 		=	bw_mgt_open,
	.unlocked_ioctl	=	bw_mgt_ioctl,
	.release		=	bw_mgt_release	
};

/**********************************************************************

File handling code associated with the sched_rt_bw_mgt file

***********************************************************************/

void allocator_sleep(void)
{
	set_current_state(TASK_UNINTERRUPTIBLE);
	preempt_enable_no_resched();
	schedule();
	preempt_disable();
}

/*Return the number of PBS tasks, for which information is returned*/
ssize_t bw_mgt_first_read(struct file* file, char __user *dst, size_t count, loff_t *offset)
{
	int ret;

	if(allocator_state != ALLOCATOR_START)
	{
		printk(KERN_INFO "Trying to start allocator loop, without setting up allocator!\n");
		return -ECANCELED;
	}
	else
	{
		printk(KERN_INFO "This is a call to first read by process %d.\n", current->pid);
	}

	//from this point forward, donot allow other tasks to preempt the allocator task
	preempt_disable();
	ret = start_pbs_timing();
	if(ret)
	{
		preempt_enable();
		return ret;
	}

	//Set the read pointer to the regular read
	bw_mgt_fops.read = bw_mgt_read;

	//state is changed to ALLOCATOR LOOP after the first execution of the function "sp_timer_func"
	//this is done to ensure that expires_next and expires_prev are initialized by it
	allocator_state = ALLOCATOR_LOOP;

	allocator_sleep();
	
	return count;	
}

ssize_t bw_mgt_read(struct file* file, char __user *dst, size_t count, loff_t *offset)
{
	//check before going to sleep
	if(allocator_state != ALLOCATOR_LOOP)
	{
		return -ECANCELED;
	}

	//assign new bandwidths
	assign_bandwidths();

	//sleep until the next scheduling period
	allocator_sleep();

	//check after going to sleep
	if(allocator_state != ALLOCATOR_LOOP)
	{
		return -ECANCELED;
	}

	//wakeup all appropriate SRT tasks and refresh their bandwidths
	sched_period_tick();

	return count;
}

long bw_mgt_ioctl(struct file * file, unsigned int cmd, unsigned long arg) //struct inode * inode,
{
	static s64	allocator_runtime = 500;
	static s64	allocator_period = 10000;
	int ret = 0;

	if(allocator_state != ALLOCATOR_OPEN)
	{
        //FIXME
		//printk(KERN_INFO "Process %d calling ioctl while allocator state = %s!\n", current->pid, a_state_strings[allocator_state]);
        printk(KERN_INFO "Process %d calling ioctl while allocator state = %i!\n", current->pid, allocator_state);
		return -EINVAL;
	}

	printk(KERN_INFO "bw_mgt_ioctl called by process %d.\n", current->pid);

	switch(cmd)
	{
		case PBS_IOCTL_BWMGT_PERIOD:
			if(arg == 0)
			{
				printk(KERN_INFO "Allocator period too small! Got %li", arg);
				ret = -EINVAL;
			}
			else
			{
				printk(KERN_INFO "Setting period to %li.\n", arg);
				allocator_period = arg;
			}
			break;
		case PBS_IOCTL_BWMGT_ALLOC_RUNTIME:
			if((arg == 0) || (arg  > allocator_period))
			{
				printk(KERN_INFO "Received invalid value for bandwidth! Period = %lli. Got %li.", (s64)allocator_period, arg);
				ret = -EINVAL;				
			}
			else
			{
				printk(KERN_INFO "Setting allocator runtime to %li.\n", arg);			
				allocator_runtime = arg;
			}
			break;
		case PBS_IOCTL_BWMGT_START:
			ret = setup_allocator(allocator_period, allocator_runtime);
			if(ret == 0)
			{
				allocator_state = ALLOCATOR_START;
				//restore the default values of runtime and period
				//since these are static variables, this is exacly where it has to be done
				allocator_runtime = 500;
				allocator_period = 10000;
			}
			else
				printk(KERN_INFO "Failed to assign period/runtime to allocator cgroup! (%d)", ret);
			break;
		default:
			printk(KERN_INFO "Control should not have reached this point!!\n");
			ret = -EINVAL;
	}

	return ret;
}

int bw_mgt_mmap(struct file *file, struct vm_area_struct *vmas)
{
	int ret;

	if((allocator_state == ALLOCATOR_OPEN) || (allocator_state == ALLOCATOR_START))
	{
		ret =  do_pbs_mmap(vmas);
	}
	else
	{
        //FIXME
		//printk(KERN_INFO "Process %d calling mmap while allocator state = %s!\n", current->pid, a_state_strings[allocator_state]);
        printk(KERN_INFO "Process %d calling mmap while allocator state = %i!\n", current->pid, allocator_state);
		ret = -EINVAL;		
	}

	return ret;
}

int bw_mgt_open(struct inode *inode, struct file *file)
{
	int ret = 0;

	if(allocator_state != MODULE_LOADED)
	{
		return -EBUSY;
	}

	printk(KERN_INFO "*********************************\n");
	printk(KERN_INFO "bw_mgt_open called by process %d.\n", current->pid);

	ret = init_histlist();
	if(ret != 0)
	{
		return ret;
	}

	bw_mgt_fops.read = bw_mgt_first_read;

	allocator_state = ALLOCATOR_OPEN;

	return ret;
}

static atomic_t SRT_count;

void reset_SRT_count(void)
{
	atomic_set(&SRT_count, 0);
}

void increment_SRT_count(void)
{
	atomic_inc(&SRT_count);
}

void decrement_SRT_count(void)
{
	int local_count;

	preempt_disable();

	local_count = atomic_dec_return(&SRT_count);

	if(local_count == 0)
	{
		if(allocator_state == ALLOCATOR_CLOSING)
		{
			allocator_state = MODULE_LOADED;
		}
	}

	preempt_enable();
}

int disable_allocator(void)
{
	int ret = 0;

	preempt_disable();
	if(allocator_state == ALLOCATOR_LOOP)
	{
		ret = stop_pbs_timing(1);
		if(ret == 0)
		{
			allocator_state = ALLOCATOR_PRECLOSING;
		}
	}
	else
	{
		ret = -EBUSY;
	}
	preempt_enable();

	return ret;
}

int bw_mgt_release(struct inode *inode, struct file *filp)
{
	int local_count;
	printk(KERN_INFO "bw_mgt_release called by process %d.\n", current->pid);

	switch(allocator_state)
	{
		case ALLOCATOR_LOOP:

			stop_pbs_timing(0);

		case ALLOCATOR_PRECLOSING:

			allocator_state = ALLOCATOR_CLOSING;
			local_count = atomic_read(&SRT_count);
			if(local_count == 0)
			{
				allocator_state = MODULE_LOADED;
			}
			else
			{
				printk(KERN_INFO "Exiting with %i tasks in system.\n", local_count);		
			}

			preempt_enable();
			
			break; 		
	}


		
	return 0;
}

/**********************************************************************

Initialization/Uninitialization code associated with the sched_rt_bw_mgt file

***********************************************************************/

int init_pbs_actv(void);
void uninit_pbs_actv(void);

int init_bw_mgt(void)
{
	int returnable = 0;

	/*Allocate pages for mapping*/
	if(allocate_mapping_pages() != 0)
	{
		returnable = -ENOMEM;
		goto error;
	}

	returnable = init_pbs_actv();
	if(returnable != 0)
	{
		printk(KERN_INFO "Failed to initialize \"sched_pbs_actv\"!\n");
		return returnable;
	}

	/*Create the file in the root of the profcs directory, initially with no permissions for others*/
	p_bw_mgt_file = create_proc_entry(bw_mgt_file_name, 0600, NULL);
	if(p_bw_mgt_file == NULL)
	{
		returnable = -ENOMEM;
		goto error;
	}
	p_bw_mgt_file->data = NULL;
	p_bw_mgt_file->proc_fops = &bw_mgt_fops;
	p_bw_mgt_file->mode = 0666;

	allocator_state = MODULE_LOADED;
error:
	return returnable;
}

int uninit_bw_mgt(void)
{
	int returnable = 0;

	free_mapping_pages();

	remove_proc_entry(bw_mgt_file_name, NULL);

	uninit_pbs_actv();

	return returnable;
}

