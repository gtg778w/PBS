#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "/home/mikel/workspace/safayet/papi-5.1.0/src/papi.h"

#define NUM_EVENTS 2
#define ERROR_RETURN(reval) { fprintf(stderr, "Error %d %s:line %d: \n", retval, __FILE__,__LINE__); exit(retval); }
void main(int argc, char* argv[]) {
    int i = 100000;
    int sum;
    if (argc > 1) {
	i = atoi(argv[1]);
    }
	
    int Events[2] = {PAPI_TOT_INS, PAPI_TOT_CYC};

    int num_hwcntrs = 0;
    int retval;
    char errstring[PAPI_MAX_STR_LEN];

    long long values[NUM_EVENTS];

    if ((retval = PAPI_library_init(PAPI_VER_CURRENT)) != PAPI_VER_CURRENT) {
	fprintf(stderr, "Error: %d %s\n", retval, errstring);
	exit(1);
    }

    if ((num_hwcntrs = PAPI_num_counters()) < PAPI_OK) {
	printf("There are no counters available.\n");
        exit(1);
    }

    printf("There are %d counters in the system\n", num_hwcntrs);

    if ((retval = PAPI_start_counters(Events, NUM_EVENTS)) != PAPI_OK) {
        ERROR_RETURN(retval);
    }

    printf("\nCounter Started: \n");

    sum = add_test(i);
    printf("sum = %d\n", sum);
	
    if ((retval = PAPI_read_counters(values, NUM_EVENTS)) != PAPI_OK) {
        ERROR_RETURN(retval);
    }

    printf("Read successfully\n");
    printf("Total instructions executed for addition: %lld \n", values[0]);
    printf("TOtal cycles used %lld\n", values[1]);
    

    if ((retval = PAPI_stop_counters(values, NUM_EVENTS)) != PAPI_OK) {
	ERROR_RETURN(retval);
    }
    exit(0);
}

int add_test(int i) {
	int sum;
	for (; i > 0; i--) {
		if (i % 2 == 0) {
			sum += i;
		} else {
			sum -= i;
		}
	}
	return sum;
}
