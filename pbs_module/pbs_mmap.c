#include "pbs_mmap.h"
#include "pbs_timing.h"

struct  page		*loaddata_pages = NULL;

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
		if(size == LOADDATALIST_SIZE)
		{
			mappable = loaddata_pages;
			if((protection_flag.pgprot & VM_WRITE) != 0)
			{
				printk(KERN_INFO "pbs_mmap: mapped pages with offset 0 are not granted write permission!\n");				
				return -EPERM;
			}
			
		}
		else
		{
			printk(KERN_INFO "pbs_mmap: Invalid size for offset = 0! Can only be %ib!\n", LOADDATALIST_SIZE);
			return -EINVAL;
		}
	}
	else if(vmas->vm_pgoff == (LOADDATALIST_SIZE>>PAGE_SHIFT))
	{
		if(size == ALLOC_SIZE)
		{
			mappable = allocation_pages;
		}
		else
		{
			printk(KERN_INFO "pbs_mmap: Invalid size for offset = %i! Can only be %ib!\n", LOADDATALIST_SIZE, ALLOC_SIZE);
			return -EINVAL;
		}
	}
	else
	{
		printk(KERN_INFO "pbs_mmap: Invalid offset. Can only be 0 or %i!\n", LOADDATALIST_SIZE);
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

loaddata_list_header_t	*loaddata_list_header;

SRT_loaddata_t		*loaddata_array;

SRT_loaddata_t		*loaddata_freelist;
struct mutex		freelist_lock;

int init_loaddataList(void)
{
	int loaddata_index;

	if(loaddata_array == NULL)	
		return -EBUSY;

	loaddata_list_header = (loaddata_list_header_t*)loaddata_array;

	loaddata_list_header->prev_sp_boundary  = 0;
	loaddata_list_header->scheduling_period = scheduling_period_ns.tv64;

	loaddata_list_header->max_allocator_runtime = 0;
	loaddata_list_header->last_allocator_runtime = 0;

	loaddata_list_header->SRT_count = 0;
	loaddata_list_header->first = 0;
	

	loaddata_freelist = &(loaddata_array[1]);
	for(    loaddata_index = 1; 
	        loaddata_index < (LOADDATALIST_SIZE/sizeof(SRT_loaddata_t)); 
	        loaddata_index++)
	{
        loaddata_array[loaddata_index].job_release_time = 0;
		loaddata_array[loaddata_index].pid			= 0;
		loaddata_array[loaddata_index].current_runtime = 0;
		loaddata_array[loaddata_index].sp_till_deadline= 1;
		loaddata_array[loaddata_index].sp_per_tp		= 1;
		loaddata_array[loaddata_index].queue_length	= 0;
		loaddata_array[loaddata_index].next		= (loaddata_index+1);
		loaddata_array[loaddata_index].prev		= (loaddata_index-1);
	}
	loaddata_array[(LOADDATALIST_SIZE/sizeof(SRT_loaddata_t))-1].next = 0;

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

	loaddata_pages = alloc_pages((__GFP_WAIT | __GFP_REPEAT), LOADDATALIST_ORDER);
	if(loaddata_pages == NULL)
	{
		returnable = -ENOMEM;
		goto error_free_ap;
	}

	if(page_address(loaddata_pages) == NULL)
	{
		//the allocated memory is in high memory, which is bad for us
		printk(KERN_INFO "The allocated memory is in high memory!\n");
		returnable = -ENOMEM;
		goto error_free_hp;
	}

	loaddata_array = (SRT_loaddata_t*)page_address(loaddata_pages);

	printk(KERN_INFO "pages allocated!\n");

	//initialize the locks
	mutex_init(&freelist_lock);

	//call function to initialize loaddata list;
	return init_loaddataList();

error_free_hp:
	__free_pages(loaddata_pages, LOADDATALIST_ORDER);
	loaddata_pages = NULL;
	loaddata_list_header = NULL;
	loaddata_array = NULL;

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

	__free_pages(loaddata_pages, LOADDATALIST_ORDER);
	loaddata_pages = NULL;

	__free_pages(allocation_pages, ALLOC_ORDER);
	allocation_pages = NULL;
	
	//enable preemption before leaving
	preempt_enable();
}

SRT_loaddata_t* alloc_loaddata(void)
{
	SRT_loaddata_t	*new_entry;

	//remove an entry from the head of the  free list
	mutex_lock(&freelist_lock);

	new_entry = loaddata_freelist;
	if(new_entry == loaddata_array)
	{
		new_entry = NULL;
		goto unlock_exit;
	}		

	loaddata_freelist = &(loaddata_array[new_entry->next]);
	if(loaddata_freelist != loaddata_array)
	{
		loaddata_freelist->prev = 0;
	}

	//initialize the entry
	new_entry->sp_till_deadline = 1;
	new_entry->sp_per_tp = 1;
	new_entry->queue_length = 0;

unlock_exit:
	mutex_unlock(&freelist_lock);
	return new_entry;
}

void free_loaddata(SRT_loaddata_t* used_entry)
{
	SRT_loaddata_t	*prev, *next;

	//remove an entry from the head of the  free list
	mutex_lock(&freelist_lock);

	//search for the correct location to insert the used entry
	prev = loaddata_array;
	next = loaddata_freelist;
	while((next != loaddata_array) && (next < used_entry))
	{
		prev = next;
		next = &(loaddata_array[next->next]);
		//if next->next = 0 (next is the last element), then next will become loaddata_array
	}

	//note that pointer subtraction results in array index

	//set the next and prev pointers of used entry
	used_entry->next = next-loaddata_array;
	used_entry->prev = prev-loaddata_array;

	//set whatever pointer needs to point to used entry
	if(prev == loaddata_array)
	{
		//used_entry will be the first element in the free list
		loaddata_freelist = used_entry;
	}
	else
	{
		//there is at least 1 entry before used_entry in the free list
		prev->next = used_entry - loaddata_array;
	}

	if(next != loaddata_array)
	{
		//there is at least 1 entry after used_entry in the free list
		next->prev = used_entry - loaddata_array;
	}

	mutex_unlock(&freelist_lock);
}

void insert_loaddata(SRT_loaddata_t* new_entry)
{
	SRT_loaddata_t	*prev, *next;

	//disable preemption to prevent overlap with the execution of the allocator
	preempt_disable();

	//search for the correct location to insert the used entry
	prev = loaddata_array;
	next = &(loaddata_array[loaddata_list_header->first]);
	while((next != loaddata_array) && (next < new_entry))
	{
		prev = next;
		next = &(loaddata_array[next->next]);
		//if next->next = 0 (next is the last element), then next will become loaddata_array
	}

	//note that pointer subtraction results in array index

	//set the next and prev pointers of new entry
	new_entry->next = next-loaddata_array;
	new_entry->prev = prev-loaddata_array;

	//set whatever pointer needs to point to new entry
	if(prev == loaddata_array)
	{
		//new_entry will be the first element in the loaddata list
		loaddata_list_header->first = new_entry - loaddata_array;
	}
	else
	{
		//there is at least 1 entry before new_entry in the loaddata list
		prev->next = new_entry - loaddata_array;
	}

	if(next != loaddata_array)
	{
		//there is at least 1 entry after used_entry in the free list
		next->prev = new_entry - loaddata_array;
	}

	(loaddata_list_header->SRT_count)++;

	//reenable preemption before leaving
	preempt_enable();
}

void remove_loaddata(SRT_loaddata_t* new_entry)
{
	//disable preemption to prevent overlap with the execution of the allocator
	preempt_disable();

	//check if the first entry
	if(new_entry->prev == 0)
	{
		loaddata_list_header->first = new_entry->next;
	}
	else
	{
		loaddata_array[new_entry->prev].next = new_entry->next;
	}

	//check if the last entry
	if(new_entry->next != 0)
	{
		loaddata_array[new_entry->next].prev = new_entry->prev;
	}

	(loaddata_list_header->SRT_count)--;

	//reenable preemption before leaving
	preempt_enable();
}

