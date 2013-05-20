#include <stdint.h>
#include <stdio.h>
#include "pbs_cmd.h"

/*This is the user level header file for the PBS module for SRT tasks*/

#define PBS_IOCTL_JBMGT_PERIOD 		0
#define PBS_IOCTL_JBMGT_SRT_RUNTIME		1
#define PBS_IOCTL_JBMGT_SRT_HISTLEN		3
#define PBS_IOCTL_JBMGT_START			4
#define PBS_IOCTL_JBMGT_MAX			5

typedef uint64_t u64;
typedef uint32_t u32;

struct SRT_job_log
{
	u64	abs_releaseTime;
	u64	abs_LFT;
	u32	runtime;

    u32	last_sp_compt_allocated;
	u32	last_sp_compt_used_sofar;

    //FIXME
    u32 throttle_count;
    u32 switch_count;
    unsigned char miss;
};

typedef struct SRT_handle_s
{
	int 				procfile;
	uint64_t			period;
	uint64_t			start_bandwidth;
	int				history_length;

	char				logging_enabled;
	struct SRT_job_log	*log;
	FILE				*log_file;
	int				log_index;
	int				log_size;
	
} SRT_handle;

int pbs_SRT_setup(uint64_t period, uint64_t start_bandwidth, int history_length, SRT_handle *handle, 
			char Lflag, int logCount, char *logFileName);

int pbs_begin_SRT_job(SRT_handle *handle);

void pbs_SRT_close(SRT_handle *handle);


