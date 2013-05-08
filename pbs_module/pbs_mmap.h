#ifndef PBS_MMAP_INCLUDE
#define PBS_MMAP_INCLUDE

#include <linux/mm_types.h>
/*
struct page
struct vma_struct
*/

#include <linux/mm.h>
/*
page_address()
page protection bit definitions e.g. VM_READ
remap_pfn_range()
*/

#include <linux/gfp.h>
/*
alloc_pages()
flags associated with alloc_pages
__free_pages
*/

#include <linux/mutex.h>
/*
mutex related code
*/


#define HISTLIST_SIZE	(1<<20) //1MB
#define HISTLIST_ORDER	(20-PAGE_SHIFT)

#define ALLOC_SIZE	(1<<17)
#define ALLOC_ORDER	(17-PAGE_SHIFT)


//Each SRT_history structure is 64 bytes long (to fit in 1 cache line)
//ideally the structure should be stored in an aligned address
//the elements of the structure has been arranged for compactness

typedef struct _SRT_history
{
	u64	job_release_time;	// 8 bytes
	u32	pid;			    // 4 bytes
	u32	current_runtime;	// 4 bytes
	u32	sp_till_deadline;   // 4 bytes
	u32	sp_per_tp;		    // 4 bytes

	unsigned short	queue_length;	// 2 bytes

	//In a 2MB page, can have at most 2^15 elements of size 64B
	//The index of the next valid structure can be stored in a
	//short
	unsigned short	next; 	// 2 bytes
	unsigned short	prev;	// 2 bytes

	char	        history_length; // 1 bytes

	char	        buffer_index;   //1 byte

    // 8 + 4*4 + 3*2 + 2= 32 bytes

	u32	history[120]; //120*4 bytes = 480 bytes

    //480 + 32 = 512 bytes
} SRT_history_t;

extern SRT_history_t	*history_array;
extern u64			*allocation_array;

typedef struct _history_list_header
{
	u64			prev_sp_boundary;		//8
	u64			scheduling_period;	//8

	u64			max_allocator_runtime;	//8
	u64			last_allocator_runtime;	//8

	unsigned short	SRT_count;			//2
	unsigned short	first;			//2
	//sum so far = 20 bytes
	unsigned char	buffer[492];//492 bytes to make the structure 512 bytes
} history_list_header_t;

SRT_history_t* alloc_histentry(void);
void free_histentry(SRT_history_t* used_entry);

void insert_histentry(SRT_history_t* new_entry);
void remove_histentry(SRT_history_t* new_entry);

int do_pbs_mmap(struct vm_area_struct *vmas);

int allocate_mapping_pages(void);
int init_histlist(void);
void free_mapping_pages(void);

#endif
