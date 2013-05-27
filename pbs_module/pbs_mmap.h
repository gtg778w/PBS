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

#include "pbsAllocator_cmd.h"

extern SRT_history_t	*history_array;
extern u64			*allocation_array;

SRT_history_t* alloc_histentry(void);
void free_histentry(SRT_history_t* used_entry);

void insert_histentry(SRT_history_t* new_entry);
void remove_histentry(SRT_history_t* new_entry);

int do_pbs_mmap(struct vm_area_struct *vmas);

int allocate_mapping_pages(void);
int init_histlist(void);
void free_mapping_pages(void);

#endif
