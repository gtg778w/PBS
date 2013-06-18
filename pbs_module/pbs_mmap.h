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
/*
loaddata structures
*/

#include "LAMbS_molookup.h"
/*
LAMbS_mo_count
*/

/*the loaddata header size is defined as a macro to allow for the tail-grown array 
inside the structure. This macro is only available to the kernel, because it makes
use of the LAMbS_mo_count global variable*/
#define sizeof_loaddata_header()    (   sizeof(loaddata_list_header_t) +    \
                                        (sizeof(u64)*(LAMbS_mo_count-1)))

extern SRT_loaddata_t   *loaddata_array;
extern u64              *allocation_array;

SRT_loaddata_t* alloc_loaddata(void);
void free_loaddata(SRT_loaddata_t* used_entry);

void insert_loaddata(SRT_loaddata_t* new_entry);
void remove_loaddata(SRT_loaddata_t* new_entry);

int do_pbs_mmap(struct vm_area_struct *vmas);

int allocate_mapping_pages(void);
int init_loaddataList(void);
void free_mapping_pages(void);

#endif
