#ifndef PBS_ALLOCATOR_INCLUDE
#define PBS_ALLOCATOR_INCLUDE

#include <stdint.h>

typedef uint32_t    u32;
typedef uint64_t    u64;
typedef uint16_t    u16;
typedef uint8_t     u8;
typedef int32_t     s32;
typedef int64_t     s64;

#include <unistd.h>
#define PAGE_SIZE   (getpagesize())

#include <strings.h>
#define PAGE_SHIFT  (ffs(PAGE_SIZE)-1)

#include "pbsAllocator_cmd.h"

extern SRT_loaddata_t         *loaddata_array;
extern loaddata_list_header_t *loaddata_list_header;
extern uint64_t               *allocation_array;

int allocator_setup(uint64_t scheduling_period,
                    uint64_t allocator_bandwidth);
int allocator_close(int proc_file);

extern int64_t  sp_limit;


int setup_log_memory(long log_level);
void log_summary_setmocount(void);
void log_allocator_summary(void);
void log_allocator_dat( long long sp_count, 
                        double est_icount,
                        double est_energy);
void log_SRT_sp_dat(int task_index,
                    long long sp_count,
                    SRT_loaddata_t  *SRT_loaddata_p,
                    uint64_t SRT_budget);
void free_log_memory(long log_level);

void compute_max_CPU_budget(void);
void compute_budget(SRT_loaddata_t *loaddata, double* budget);
extern enum pbs_budget_type budget_type;
extern double               *presaturation_budget_array;
extern double               maximum_available_CPU_time;
extern double               maximum_available_CPU_budget;

extern double  *perf_model_coeffs_double;
extern double  *power_model_coeffs_double;
int     pbsAllocator_modeladapters_init(int proc_file);
void    pbsAllocator_modeladapters_adapt(double *est_icount_p, double *est_energy_p);
void    pbsAllocator_modeladapters_free(int proc_file);

int     max_perf_coeff_moi;
double  max_perf_coeff;


#endif
