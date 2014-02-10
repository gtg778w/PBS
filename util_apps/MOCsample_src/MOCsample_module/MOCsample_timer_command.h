#ifndef MOCsample_TIMER_COMMAND_INCLUDE
#define MOCsample_TIMER_COMMAND_INCLUDE

    #include <linux/kernel.h>

    typedef struct MOCsample_timed_sample_s
    {
        u64     time_stamp;
        u64     MOCsample;
    } MOCsample_timed_sample_t;

    typedef struct MOCsample_timer_command_s
    {
        int command;
        u64 arguments[3];
    } MOCsample_timer_command_t;
    
    /*Command Name*/
    #define MOCsample_TIMER_COMMAND_START           (1)
    /*Command Arguments*/
    #define MOCsample_TIMER_COMMAND_START_PERIOD    (0)
    #define MOCsample_TIMER_COMMAND_START_COUNT     (1)
    
    /*Command Name*/
    #define MOCsample_TIMER_COMMAND_STOP            (2)
    /*Command Arguments*/
    #define MOCsample_TIMER_COMMAND_STOP_BUFFSIZ    (0)
    #define MOCsample_TIMER_COMMAND_STOP_VLDELMS    (1)
    #define MOCsample_TIMER_COMMAND_STOP_BUFFPTR    (2)
#endif
