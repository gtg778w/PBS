#ifndef LOADGEN_HEADER
#define LOADGEN_HEADER

#include <time.h>
#include <stdint.h>

/*
This file needs to be linked with -lrt
*/

static double ipms = 1000000.0;

static int work_function(long count)
{
	int i;
	int32_t a=134775813, c=1, x=0;

	for(i = 0; i < count; i++)
	{
		x = a*x+c;
	}

	return x;
}

#define timed_work_function(ms)\
{\
    long iterations = (long)(ipms * ms);\
    work_function(iterations);\
}

static long callibrate(long count)
{
    int ret;
    struct timespec start_ts, stop_ts;
    double start, stop, interval;
    long x;

    if(count == 0) count = 100000000;

    ret = clock_gettime(CLOCK_MONOTONIC, &start_ts);
    if(ret == -1)
    {
        ipms = -1.0;
        return -1;
    }

    x = work_function(count);

    ret = clock_gettime(CLOCK_MONOTONIC, &stop_ts);
    if(ret == -1)
    {
        ipms = -1.0;
        return -1;
    }

    start = ((start_ts.tv_sec * 1000000000.0) + 
            (start_ts.tv_nsec));

    stop = ((stop_ts.tv_sec * 1000000000.0) + 
            (stop_ts.tv_nsec));
    interval = stop - start;
    ipms = (((double)count)/interval)*1000000.0;
    return x;
}

#endif
