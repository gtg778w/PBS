#include "pbs_mmap.h"
#include "pbs_timing.h"

struct  page		*histlist_pages = NULL;

struct  page		*allocation_pages = NULL;
u64				*allocation_array = NULL;

int do_pbs_mmap(struct vm_area_struct *vmas)
{
	int		size;
	struct page	*mappable;
	pgprot_t	protection_flag;

	//FIXME: check if pages have already been mapped
	//only map pages if none have already been mapped

	size = vmas->vm_end - vmas->vm_start;
	protection_flag = vmas->vm_page_prot;
	//protection_flag.pgprot |= VM_SHARED;
	if(vmas->vm_pgoff == 0)
	{
		if(size == HISTLIST_SIZE)
		{
			mappable = histlist_pages;
			if((protection_flag.pgprot & VM_WRITE) != 0)
			{
				printk(KERN_INFO "pbs_mmap: mapped pages with offset 0 are not granted write permission!\n");				
				return -EPERM;
			}
			
		}
		else
		{
			printk(KERN_INFO "pbs_mmap: Invalid size for offset = 0! Can only be %ib!\n", HISTLIST_SIZE);
			return -EINVAL;
		}
	}
	else if(vmas->vm_pgoff == (HISTLIST_SIZE>>PAGE_SHIFT))
	{
		if(size == ALLOC_SIZE)
		{
			mappable = allocation_pages;
		}
		else
		{
			printk(KERN_INFO "pbs_mmap: Invalid size for offset = %i! Can only be %ib!\n", HISTLIST_SIZE, ALLOC_SIZE);
			return -EINVAL;
		}
	}
	else
	{
		printk(KERN_INFO "pbs_mmap: Invalid offset. Can only be 0 or %i!\n", HISTLIST_SIZE);
		return -EINVAL;
	}

	printk(KERN_INFO "pbs_mmap: Mapping offset %lx with size %d and permission %lx\n", vmas->vm_pgoff, size, protection_flag.pgprot);

	//map the page into the virtual address
	if(remap_pfn_range(vmas, vmas->vm_start, page_to_pfn(mappable), size, protection_flag))
	{
		printk(KERN_INFO "Failed to map memory!\n");
		return -EAGAIN;
	}

	return 0;

}

history_list_header_t	*history_list_header;

SRT_history_t		*history_array;

SRT_history_t		*history_freelist;
struct mutex		freelist_lock;

int init_histlist(void)
{
	int hist_index;

	if(history_array == NULL)	
		return -EBUSY;

	history_list_header = (history_list_header_t*)history_array;

	history_list_header->prev_sp_boundary  = 0;
	history_list_header->scheduling_period = scheduling_period_ns.tv64;

	history_list_header->max_allocator_runtime = 0;
	history_list_header->last_allocator_runtime = 0;

	history_list_header->SRT_count = 0;
	history_list_header->first = 0;
	

	history_freelist = &(history_array[1]);
	for(hist_index = 1; hist_index < (HISTLIST_SIZE/sizeof(SRT_history_t)); hist_index++)
	{
        history_array[hist_index].job_release_time = 0;
		history_array[hist_index].pid			= 0;
		history_array[hist_index].current_runtime = 0;
		history_array[hist_index].sp_till_deadline= 1;
		history_array[hist_index].sp_per_tp		= 1;
		history_array[hist_index].queue_length	= 0;
		history_array[hist_index].next		= (hist_index+1);
		history_array[hist_index].prev		= (hist_index-1);
		history_array[hist_index].history_length	= 0;	// 1 bytes
        history_array[hist_index].buffer_index	= 0;
	}
	history_array[(HISTLIST_SIZE/sizeof(SRT_history_t))-1].next = 0;

	return 0;
}

int allocate_mapping_pages(void)
{
	int returnable;

	//allocate pages (not in high memory)
	allocation_pages = alloc_pages((__GFP_WAIT | __GFP_REPEAT |  __GFP_ZERO), ALLOC_ORDER);
	if(allocation_pages == NULL)
		return -ENOMEM;

	if(page_address(allocation_pages) == NULL)
	{
		//the allocated memory is in high memory, which is bad for us
		printk(KERN_INFO "The allocated memory is in high memory!\n");
		returnable = -ENOMEM;
		goto error_free_ap;
	}

	allocation_array = (u64*)page_address(allocation_pages);

	histlist_pages = alloc_pages((__GFP_WAIT | __GFP_REPEAT), HISTLIST_ORDER);
	if(histlist_pages == NULL)
	{
		returnable = -ENOMEM;
		goto error_free_ap;
	}

	if(page_address(histlist_pages) == NULL)
	{
		//the allocated memory is in high memory, which is bad for us
		printk(KERN_INFO "The allocated memory is in high memory!\n");
		returnable = -ENOMEM;
		goto error_free_hp;
	}

	history_array = (SRT_history_t*)page_address(histlist_pages);

	printk(KERN_INFO "pages allocated!\n");

	//initialize the locks
	mutex_init(&freelist_lock);

	//call function to initialize history list;
	return init_histlist();

error_free_hp:
	__free_pages(histlist_pages, HISTLIST_ORDER);
	histlist_pages = NULL;
	history_list_header = NULL;
	history_array = NULL;

error_free_ap:
	__free_pages(allocation_pages, ALLOC_ORDER);
	allocation_pages = NULL;

	return returnable;

}

void free_mapping_pages(void)
{
	//FIXME: make sure the pages arn't mapped
	//do_munmap

	preempt_disable();

	__free_pages(histlist_pages, HISTLIST_ORDER);
	histlist_pages = NULL;

	__free_pages(allocation_pages, ALLOC_ORDER);
	allocation_pages = NULL;
	
	//enable preemption before leaving
	preempt_enable();
}

SRT_history_t* alloc_histentry(void)
{
	SRT_history_t	*new_entry;

	//remove an entry from the head of the  free list
	mutex_lock(&freelist_lock);

	new_entry = history_freelist;
	if(new_entry == history_array)
	{
		new_entry = NULL;
		goto unlock_exit;
	}		

	history_freelist = &(history_array[new_entry->next]);
	if(history_freelist != history_array)
	{
		history_freelist->prev = 0;
	}

	//initialize the entry
	new_entry->sp_till_deadline = 1;
	new_entry->sp_per_tp = 1;
	new_entry->queue_length = 0;
	new_entry->history_length = 0;
	new_entry->history[0] = 0;

unlock_exit:
	mutex_unlock(&freelist_lock);
	return new_entry;
}

void free_histentry(SRT_history_t* used_entry)
{
	SRT_history_t	*prev, *next;

	//remove an entry from the head of the  free list
	mutex_lock(&freelist_lock);

	//search for the correct location to insert the used entry
	prev = history_array;
	next = history_freelist;
	while((next != history_array) && (next < used_entry))
	{
		prev = next;
		next = &(history_array[next->next]);
		//if next->next = 0 (next is the last element), then next will become history_array
	}

	//note that pointer subtraction results in array index

	//set the next and prev pointers of used entry
	used_entry->next = next-history_array;
	used_entry->prev = prev-history_array;

	//set whatever pointer needs to point to used entry
	if(prev == history_array)
	{
		//used_entry will be the first element in the free list
		history_freelist = used_entry;
	}
	else
	{
		//there is at least 1 entry before used_entry in the free list
		prev->next = used_entry - history_array;
	}

	if(next != history_array)
	{
		//there is at least 1 entry after used_entry in the free list
		next->prev = used_entry - history_array;
	}

	mutex_unlock(&freelist_lock);
}

void insert_histentry(SRT_history_t* new_entry)
{
	SRT_history_t	*prev, *next;

	//disable preemption to prevent overlap with the execution of the allocator
	preempt_disable();

	//search for the correct location to insert the used entry
	prev = history_array;
	next = &(history_array[history_list_header->first]);
	while((next != history_array) && (next < new_entry))
	{
		prev = next;
		next = &(history_array[next->next]);
		//if next->next = 0 (next is the last element), then next will become history_array
	}

	//note that pointer subtraction results in array index

	//set the next and prev pointers of new entry
	new_entry->next = next-history_array;
	new_entry->prev = prev-history_array;

	//set whatever pointer needs to point to new entry
	if(prev == history_array)
	{
		//new_entry will be the first element in the history list
		history_list_header->first = new_entry - history_array;
	}
	else
	{
		//there is at least 1 entry before new_entry in the history list
		prev->next = new_entry - history_array;
	}

	if(next != history_array)
	{
		//there is at least 1 entry after used_entry in the free list
		next->prev = new_entry - history_array;
	}

	(history_list_header->SRT_count)++;

	//reenable preemption before leaving
	preempt_enable();
}

void remove_histentry(SRT_history_t* new_entry)
{
	//disable preemption to prevent overlap with the execution of the allocator
	preempt_disable();

	//check if the first entry
	if(new_entry->prev == 0)
	{
		history_list_header->first = new_entry->next;
	}
	else
	{
		history_array[new_entry->prev].next = new_entry->next;
	}

	//check if the last entry
	if(new_entry->next != 0)
	{
		history_array[new_entry->next].prev = new_entry->prev;
	}

	(history_list_header->SRT_count)--;

	//reenable preemption before leaving
	preempt_enable();
}

