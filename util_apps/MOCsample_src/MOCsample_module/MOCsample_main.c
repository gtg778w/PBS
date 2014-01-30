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
copy_to_user
*/

#include <asm/current.h>
/*
current
*/

#include "MOCsample.h"
/*
disable_allocator
allocator_state
*/

MODULE_AUTHOR("Safayet N Ahmed");
MODULE_LICENSE("Dual BSD/GPL");

/**********************************************************************

Function headers and structure deffinitions for the sched_alloc_active file.

***********************************************************************/
void MOCsample_sched_in(struct preempt_notifier *notifier, int cpu);
void MOCsample_sched_out(struct preempt_notifier *notifier, struct task_struct *next);
        
struct preempt_ops MOCsample_pin_ops =  {
                                            .sched_in   = MOCsample_sched_in,
                                            .sched_out  = MOCsample_sched_out,
                                        };

void MOCsample_sched_in(struct preempt_notifier *notifier, int cpu)
{
    MOCsample_t *MOCsample_p;
    
    MOCsample_p = container_of(notifier, MOCsample_t, preempt_notifier);
    MOCsample_p->last_sample = MOCsample_p->read(MOCsample_p);
}

void MOCsample_sched_out(struct preempt_notifier *notifier, struct task_struct *next)
{
    MOCsample_t *MOCsample_p;
    u64         last_sample, next_sample, diff;
    u64         running_total;
    
    MOCsample_p = container_of(notifier, MOCsample_t, preempt_notifier);
    
    last_sample = MOCsample_p->last_sample;
    next_sample = MOCsample_p->read(MOCsample_p);
    diff        = next_sample - last_sample;
    
    running_total   = MOCsample_p->running_total + diff;
    MOCsample_p->running_total  = running_total;
}

/**********************************************************************

Function headers and structure deffinitions for the sched_alloc_active file.

***********************************************************************/

int MOCsample_inst_open(struct inode *inode, struct file *file);
int MOCsample_userinst_open(struct inode *inode, struct file *file);
int MOCsample_cycl_open(struct inode *inode, struct file *file);
int MOCsample_nsec_open(struct inode *inode, struct file *file);
int MOCsample_VIC_open(struct inode *inode, struct file *file);
ssize_t MOCsample_read(struct file* file, char __user *dst, size_t count, loff_t *offset);
int MOCsample_release(struct inode *inode, struct file *filp);

struct file_operations  MOCsample_inst_fops = 
{
    .owner  =   THIS_MODULE,
    .read   =   MOCsample_read,
    .open   =   MOCsample_inst_open,
    .release=   MOCsample_release
};

struct file_operations  MOCsample_userinst_fops = 
{
    .owner  =   THIS_MODULE,
    .read   =   MOCsample_read,
    .open   =   MOCsample_userinst_open,
    .release=   MOCsample_release
};

struct file_operations  MOCsample_cycl_fops = 
{
    .owner  =   THIS_MODULE,
    .read   =   MOCsample_read,
    .open   =   MOCsample_cycl_open,
    .release=   MOCsample_release
};

struct file_operations  MOCsample_nsec_fops = 
{
    .owner  =   THIS_MODULE,
    .read   =   MOCsample_read,
    .open   =   MOCsample_nsec_open,
    .release=   MOCsample_release
};

struct file_operations  MOCsample_VIC_fops = 
{
    .owner  =   THIS_MODULE,
    .read   =   MOCsample_read,
    .open   =   MOCsample_VIC_open,
    .release=   MOCsample_release
};

/*The procfs file that is accessed by the bandwidth allocation process*/
char*                   MOCsample_inst_file_name = "MOCsample_inst";
struct proc_dir_entry*  MOCsample_inst_file;
char*                   MOCsample_userinst_file_name = "MOCsample_userinst";
struct proc_dir_entry*  MOCsample_userinst_file;
char*                   MOCsample_cycl_file_name = "MOCsample_cycl";
struct proc_dir_entry*  MOCsample_cycl_file;
char*                   MOCsample_nsec_file_name = "MOCsample_nsec";
struct proc_dir_entry*  MOCsample_nsec_file;
char*                   MOCsample_VIC_file_name = "MOCsample_VIC";
struct proc_dir_entry*  MOCsample_VIC_file;

int __init MOCsample_init(void)
{
    int ret = 0;
    
    /*Initialize the slab cache for MOCsample_t objects.*/
    ret = MOCsample_alloc_init();
    if(0 != ret)
    {
        goto error0;
    }

    /*Setup the files in the procfs interface*/
    MOCsample_inst_file = create_proc_entry(MOCsample_inst_file_name, 0000, NULL);
    if(MOCsample_inst_file == NULL)
    {
        ret = -ENOMEM;
        goto error1;
    }
    MOCsample_inst_file->proc_fops = &MOCsample_inst_fops;

    MOCsample_userinst_file = create_proc_entry(MOCsample_userinst_file_name, 0000, NULL);
    if(MOCsample_userinst_file == NULL)
    {
        ret = -ENOMEM;
        goto error2;
    }
    MOCsample_userinst_file->proc_fops = &MOCsample_userinst_fops;
        
    MOCsample_cycl_file = create_proc_entry(MOCsample_cycl_file_name, 0000, NULL);
    if(MOCsample_cycl_file == NULL)
    {
        ret = -ENOMEM;
        goto error3;
    }
    MOCsample_cycl_file->proc_fops = &MOCsample_cycl_fops;
    
    MOCsample_nsec_file = create_proc_entry(MOCsample_nsec_file_name, 0000, NULL);
    if(MOCsample_nsec_file == NULL)
    {
        ret = -ENOMEM;
        goto error4;
    }
    MOCsample_nsec_file->proc_fops = &MOCsample_nsec_fops;

    MOCsample_VIC_file = create_proc_entry(MOCsample_VIC_file_name, 0000, NULL);
    if(MOCsample_VIC_file == NULL)
    {
        ret = -ENOMEM;
        goto error5;
    }
    MOCsample_VIC_file->proc_fops = &MOCsample_VIC_fops;
    
    /*Once all files are setup, enable the permissions*/
    MOCsample_inst_file->mode = 0444;
    MOCsample_userinst_file->mode = 0444;
    MOCsample_cycl_file->mode = 0444;
    MOCsample_nsec_file->mode = 0444;
    MOCsample_VIC_file->mode = 0444;
    
    return 0;

error5:
    remove_proc_entry(MOCsample_nsec_file_name, NULL);
error4:
    remove_proc_entry(MOCsample_cycl_file_name, NULL);
error3:
    remove_proc_entry(MOCsample_userinst_file_name, NULL);
error2:
    remove_proc_entry(MOCsample_inst_file_name, NULL);
error1:
    MOCsample_alloc_uninit();
error0:
    return ret;
}

void MOCsample_uninit(void)
{
    remove_proc_entry(MOCsample_VIC_file_name, NULL);
    remove_proc_entry(MOCsample_nsec_file_name, NULL);
    remove_proc_entry(MOCsample_cycl_file_name, NULL);
    remove_proc_entry(MOCsample_userinst_file_name, NULL);
    remove_proc_entry(MOCsample_inst_file_name, NULL);
    
    MOCsample_alloc_uninit();
}

module_init(MOCsample_init);
module_exit(MOCsample_uninit);

int MOCsample_inst_open(struct inode *inode, struct file *file)
{
    int ret = 0;
    MOCsample_t *MOCsample_p; 
    u64 first_sample;
    
    ret = MOCsample_alloc( &MOCsample_inst_template,
                           &(MOCsample_p));
    if(0 != ret)
    {
        goto error0;
    }

    preempt_notifier_init(  &(MOCsample_p->preempt_notifier),
                            &MOCsample_pin_ops);
    preempt_notifier_register(  &(MOCsample_p->preempt_notifier));

    first_sample = MOCsample_p->read(MOCsample_p);
    MOCsample_p->last_sample    = first_sample;
    MOCsample_p->running_total  = 0;

    file->private_data = MOCsample_p;
error0:
    return ret;
}

int MOCsample_userinst_open(struct inode *inode, struct file *file)
{
    int ret = 0;
    MOCsample_t *MOCsample_p; 
    u64 first_sample;
    
    ret = MOCsample_alloc( &MOCsample_userinst_template,
                           &(MOCsample_p));
    if(0 != ret)
    {
        goto error0;
    }

    preempt_notifier_init(  &(MOCsample_p->preempt_notifier),
                            &MOCsample_pin_ops);
    preempt_notifier_register(  &(MOCsample_p->preempt_notifier));

    first_sample = MOCsample_p->read(MOCsample_p);
    MOCsample_p->last_sample    = first_sample;
    MOCsample_p->running_total  = 0;

    file->private_data = MOCsample_p;
error0:
    return ret;
}

int MOCsample_cycl_open(struct inode *inode, struct file *file)
{
    int ret = 0;
    MOCsample_t *MOCsample_p;
    u64 first_sample;
    
    ret = MOCsample_alloc(  &MOCsample_cycl_template, 
                            &(MOCsample_p));
    if(0 != ret)
    {
        goto error0;
    }

    preempt_notifier_init(  &(MOCsample_p->preempt_notifier),
                            &MOCsample_pin_ops);
    preempt_notifier_register(  &(MOCsample_p->preempt_notifier));

    first_sample = MOCsample_p->read(MOCsample_p);
    MOCsample_p->last_sample    = first_sample;
    MOCsample_p->running_total  = 0;

    file->private_data = MOCsample_p;

error0:
    return ret;
}

int MOCsample_nsec_open(struct inode *inode, struct file *file)
{
    int ret = 0;
    MOCsample_t *MOCsample_p;
    u64 first_sample;
    
    ret = MOCsample_alloc(  &MOCsample_nsec_template, 
                            &(MOCsample_p));
    if(0 != ret)
    {
        goto error0;
    }

    preempt_notifier_init(  &(MOCsample_p->preempt_notifier),
                            &MOCsample_pin_ops);
    preempt_notifier_register(  &(MOCsample_p->preempt_notifier));

    first_sample = MOCsample_p->read(MOCsample_p);
    MOCsample_p->last_sample    = first_sample;
    MOCsample_p->running_total  = 0;

    file->private_data = MOCsample_p;

error0:
    return ret;
}

int MOCsample_VIC_open(struct inode *inode, struct file *file)
{
    int ret = 0;
    MOCsample_t *MOCsample_p;
    u64 first_sample;
    
    ret = MOCsample_alloc(  &MOCsample_VIC_template, 
                            &(MOCsample_p));
    if(0 != ret)
    {
        goto error0;
    }

    preempt_notifier_init(  &(MOCsample_p->preempt_notifier),
                            &MOCsample_pin_ops);
    preempt_notifier_register(  &(MOCsample_p->preempt_notifier));

    first_sample = MOCsample_p->read(MOCsample_p);
    MOCsample_p->last_sample    = first_sample;
    MOCsample_p->running_total  = 0;

    file->private_data = MOCsample_p;

error0:
    return ret;
}


int MOCsample_release(struct inode *inode, struct file *file)
{
    MOCsample_t *MOCsample_p= file->private_data;

    preempt_notifier_unregister(&(MOCsample_p->preempt_notifier));
    
    MOCsample_free(MOCsample_p);
    return 0;
}

static char LUT_u64tochar[16] =    
            {   '0', '1', '2', '3',
                '4', '5', '6', '7',
                '8', '9', 'a', 'b',
                'c', 'd', 'e', 'f' };

static inline void convertu64toChar(char buffer[18], u64 convertible)
{
    int i;
    int next_four;
    
    for(i = 0; i < 16; i++)
    {
        next_four   = (convertible >> ((15-i) << 2)) & 0xf;
        buffer[i] = LUT_u64tochar[next_four];
    }
    buffer[16] = '\n';
    buffer[17] = '\0';
}

ssize_t MOCsample_read(struct file* file, char __user *dst, size_t count, loff_t *offset)
{
    MOCsample_t *MOCsample_p;
    u64         last_sample, next_sample, diff;
    u64         running_total;
    char        readable[18];
    int         ret;
    
    if(*offset >= 18)
    {
        count = 0;
    }
    else
    {
        count = (count > (18-*offset))? (18-*offset) : count;
    }

    MOCsample_p = (MOCsample_t *)file->private_data;

    last_sample = MOCsample_p->last_sample;
    next_sample = MOCsample_p->read(MOCsample_p);
    diff        = next_sample - last_sample;
    MOCsample_p->last_sample    = next_sample;
    
    running_total = MOCsample_p->running_total + diff;
    MOCsample_p->running_total  = running_total;
    
    convertu64toChar(readable, running_total);
    if(count > 0)
    {
        ret = copy_to_user(dst, (readable+*offset), count);
        if(ret != 0)
        {
            return ret;
        }
    }

    return count;
}

