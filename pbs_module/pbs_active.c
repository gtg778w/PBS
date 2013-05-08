#include <linux/module.h>
/*
THIS_MODULE
*/

#include <linux/proc_fs.h>
/*
struct proc_dir_entry
create_proc_entry
*/

#include <linux/kernel.h>	
/* 
printk() 
*/

#include <linux/errno.h>	
/* 
error codes 
*/

#include <asm/uaccess.h>
/*
copy_from_user
*/

#include <asm/current.h>
/*
current
*/

#include "bw_mgt.h"
/*
disable_allocator
allocator_state
*/

/**********************************************************************

Function headers and structure deffinitions for the sched_alloc_active file.

***********************************************************************/

ssize_t pbs_actv_read(struct file* file, char __user *dst, size_t count, loff_t *offset);
ssize_t pbs_actv_write(struct file* file, const char __user *src, size_t count, loff_t *offset);
int pbs_actv_open(struct inode *inode, struct file *file);
int pbs_actv_release(struct inode *inode, struct file *filp);

/*The procfs file that is accessed by the bandwidth allocation process*/
char*  pbs_actv_file_name = "sched_pbs_actv";
struct proc_dir_entry* p_pbs_actv_file;
struct file_operations pbs_actv_fops = {
	.owner	=	THIS_MODULE,
	.read 	=	pbs_actv_read,
	.write	=	pbs_actv_write,
	.open 	=	pbs_actv_open,
	.release	=	pbs_actv_release	
};

int init_pbs_actv(void)
{
	/*Create the file in the root of the profcs directory, initially with no permissions for others*/
	p_pbs_actv_file = create_proc_entry(pbs_actv_file_name, 0600, NULL);
	if(p_pbs_actv_file == NULL)
	{
		return -ENOMEM;
	}
	p_pbs_actv_file->data = NULL;
	p_pbs_actv_file->proc_fops = &pbs_actv_fops;
	p_pbs_actv_file->mode = 0666;

	return 0;
}

void uninit_pbs_actv(void)
{
	remove_proc_entry(pbs_actv_file_name, NULL);
}

int pbs_actv_open(struct inode *inode, struct file *file)
{
	return 0;
}

int pbs_actv_release(struct inode *inode, struct file *filp)
{
	return 0;
}

ssize_t pbs_actv_read(struct file* file, char __user *dst, size_t count, loff_t *offset)
{
	char	*zero = "0\n";
	char	*one  = "1\n";
	char	*readable;	
	int	ret;

	if(*offset > 2)
	{
		count = 0;
	}
	else
	{
		count = (count > (2-*offset))? (2-*offset) : count;
	}

	readable = (allocator_state == ALLOCATOR_LOOP)? one : zero;

	if(count > 0)
	{
		ret = copy_to_user(dst, (readable+*offset), count);
		if(ret != 0)
		{
			return ret;		
		}

		*offset += count;
	}

	return count;
}

ssize_t pbs_actv_write(struct file* file, const char __user *src, size_t count, loff_t *offset)
{
	char	buffer[64], *nonwhite;
	int	remaining = count;
	int	tmp;
	int	returnable = count;

	nonwhite = buffer;
	nonwhite[0] = '\0';

	//strip white space
	while(remaining > 0)
	{
		tmp = (remaining > 63)? 63 : remaining;
		/*Copy data into kernel space*/
		if(copy_from_user(buffer, src, tmp))
		{
			printk(KERN_INFO "failed to copy from user!\n");
			returnable = -EFAULT;
			goto streight_exit;
		}

		/*Make sure the string is NULL terminated*/
		buffer[tmp] = '\0';

		/*Shed trailing and leading white space*/
		nonwhite = strstrip(buffer);
		if(nonwhite[0] != '\0')
			break;
	}

	//Check if the allocator needs to be disabled
	if(nonwhite[0] == '0')
	{
		if(disable_allocator() == 0)
		{
			printk(KERN_INFO "Allocator disabled by %d through \"proc/sched_pbs_actv\"!\n", current->pid);
		}
		else
		{
			printk(KERN_INFO "Invalid write to \"proc/sched_pbs_actv\" by %d!\n", current->pid);
			returnable = -EINVAL;
		}
	}
	else
	{
		if(nonwhite[0] != '\0')
		{
			returnable = -EINVAL;
		}
	}

streight_exit:
	return returnable;
}

