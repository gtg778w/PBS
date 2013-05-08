#ifndef BW_MGT_INCLUDE
#define BW_MGT_INCLUDE

#include <linux/module.h>
/*
THIS_MODULE
*/

#include <linux/proc_fs.h>
/*
struct proc_dir_entry
create_proc_entry
remove_proc_entry
*/

#include <linux/sched.h>
/*
task_is_dead
*/

#include <linux/preempt.h>
/*
preempt_disable
preempt_enable
*/

#include <linux/semaphore.h>
/*
semaphore related stuff
*/

#include <linux/init.h>
/*
__init
*/

#include <linux/kernel.h>	
/* 
printk() 
*/

#include <linux/errno.h>	
/* 
error codes 
*/

#include <linux/err.h>
/*
PTR_ERR
*/

#include <asm/uaccess.h>
/*
copy_from_user
*/

#include <asm/current.h>
/*
current
*/

enum 
{
	MODULE_LOADED, 
	ALLOCATOR_OPEN, 
	ALLOCATOR_START, 
	ALLOCATOR_LOOP, 
	ALLOCATOR_PRECLOSING, 
	ALLOCATOR_CLOSING,
	ALLOCATOR_STATE_COUNT
};

//FIXME
/*
static const char *a_state_strings[ALLOCATOR_STATE_COUNT] = 
{
	"MODULE_LOADED", 
	"ALLOCATOR_OPEN", 
	"ALLOCATOR_START", 
	"ALLOCATOR_LOOP", 
	"ALLOCATOR_PRECLOSING", 
	"ALLOCATOR_CLOSING"
};
*/

extern unsigned char allocator_state;

int disable_allocator(void);
void reset_SRT_count(void);
void increment_SRT_count(void);
void decrement_SRT_count(void);

int init_bw_mgt(void);
int uninit_bw_mgt(void);

#endif
