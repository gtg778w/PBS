#include <stdint.h>
#include <stdio.h>

/*This is the user level header file for the PBS module for SRT tasks*/

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


