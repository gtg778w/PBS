#include <stdint.h>

#include <math.h>

#include "pbsAllocator.h"

double alpha;

/*
FIXME: Hard to spot problem with initial conditions

1) When the history is not filled compute_budget doesn't do anything to the budget.
   However, the budget may be scaled back under a saturation condition. 
   
2) If a second task starts adapting and causing severe saturation, and the history of the
   first job is not yet filled, the budget will continue to be scaled down, and never
   scaled back up since the history isn't filled. Eventually, the allocated budget reaches
   zero with the history not filled. This is an irrecoverable situation. Because the
   budget is zero, the jobs will never finish to fill the history to allow the budget to
   be modified.
*/
int compute_budget(SRT_history_t *history, uint64_t* budget)
{
	int64_t M2, delta, mean, variance, n;
	double  stdeviation;

	int64_t  offset;

	uint64_t est_tot_cmpt, est_cur_cmpt, est_rem_cmpt;
	int i;

	/*make sure there is sufficient history to compute bandwidth*/
	if( (history->buffer_index < 0) || 
        (history->history_length == 0))
	{
		return -1;
	}

	/*make sure there is computation remaining*/
	if(history->queue_length == 0)
	{
		*budget 		  = 0;
		return 0;
	}

	/*compute the statistics of the history of computation times*/
	n = 0;
	mean = 0;
	M2 = 0;
    
	for(i = 0; i < history->history_length; i++)
	{
		n = n+1;
		delta = history->history[i] - mean;
		mean = mean + (delta/n);
        /*The following expression uses the new value of mean*/
		M2 = M2 + delta*(history->history[i]-mean);
	}
	variance = M2/(n-1);
    
	stdeviation = sqrt((double)variance);

	/*compute the estimated copmutation time of the currently running job, 
	//assuming an exponential distribution with the statistics of
	//previously completed jobs*/

	offset = mean - (uint64_t)stdeviation;

	offset = (history->current_runtime > offset)? 
             history->current_runtime : offset;

	est_cur_cmpt = offset + (uint64_t)(stdeviation * alpha);
    
	/*compute the estimate of the total computation time for
	//all queued jobs and computation remaining*/
	est_tot_cmpt = est_cur_cmpt + (history->queue_length-1)*mean;

	est_rem_cmpt = est_tot_cmpt - history->current_runtime;
	*budget = (est_rem_cmpt / (history->sp_till_deadline));
    
	return 0;
}

int compute_budget2(SRT_history_t *history, uint64_t* budget)
{
    return -1;    
}

