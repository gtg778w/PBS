#ifndef PBS_ALLOCATOR_INCLUDE
#define PBS_ALLOCATOR_INCLUDE

#include <stdint.h>

typedef uint32_t    u32;
typedef uint64_t    u64;
typedef int32_t     s32;
typedef int64_t     s64;

#include "pbsAllocator_cmd.h"

extern SRT_history_t		    *history_array;
extern history_list_header_t    *history_list_header;
extern uint64_t			        *allocation_array;

int allocator_setup(uint64_t scheduling_period,
                    uint64_t allocator_bandwidth);
int allocator_close(int proc_file);

extern int64_t                  sp_limit;

int setup_log_memory(void);
void log_allocator_dat( long long sp_count);
void log_SRT_sp_dat(int task_index,
                    long long sp_count,
                    SRT_history_t	*SRT_history_p,
                    uint64_t SRT_budget2);
void free_log_memory(void);

extern double alpha;

int compute_budget(SRT_history_t *history, uint64_t* budget);
void compute_budget2(SRT_history_t *history, uint64_t* budget);


#endif
