#ifndef PBSSRT_INCLUDE
#define PBSSRT_INCLUDE

#include <stdint.h>
#include <stdio.h>

#include "pbsSRT_predictor.h"

/*This is the user level header file for the PBS module for SRT tasks*/

typedef struct SRT_handle_s
{
	int 				procfile;
	pbsSRT_predictor_t  *predictor;
	uint64_t			period;
	uint64_t			start_bandwidth;
	int				    history_length;

	char				logging_enabled;
	struct SRT_job_log	*log;
	/*FIXME: remove these buffers to reduce overhead*/
	int64_t             *pu_c0;
	int64_t             *pstd_c0;
	int64_t             *pu_cl;
	int64_t             *pstd_cl;
	FILE				*log_file;
	int				log_index;
	int				log_size;

} SRT_handle;

int pbsSRT_setup(  uint64_t period, uint64_t start_bandwidth, 
                    int history_length,
                    pbsSRT_predictor_t *predictor,
                    SRT_handle *handle, 
			        char Lflag, int logCount, char *logFileName);

int pbsSRT_sleepTillFirstJob(SRT_handle *handle);
int pbsSRT_sleepTillNextJob(SRT_handle *handle);

void pbsSRT_close(SRT_handle *handle);

#endif
