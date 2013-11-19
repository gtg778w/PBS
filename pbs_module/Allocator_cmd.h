#ifndef ALLOCATOR_CMD_INCLUDE
#define ALLOCATOR_CMD_INCLUDE

#include "Common_cmd.h"

#define ALLOCATOR_CMD_SETUP_START (0)
/*
    0) budget type (ns or VIC)
    1) scheduling period (us)
    2) allocator runtime (us)
*/

#define ALLOCATOR_CMD_NEXTJOB   (1)
/*
*/

#define ALLOCATOR_CMD_STOP      (2)
/*
*/

#define ALLOCATOR_CMD_MAX       (3)
#define ALLOCATOR_CMD_MAXARGS   (2)

typedef struct bw_mgt_cmd_s
{
    int             cmd;
    s64         args[3];
} bw_mgt_cmd_t;

typedef struct _SRT_loaddata
{
    u64 job_release_time;   // 8 bytes
    s64 u_c0;               // 8 bytes
    s64 var_c0;             // 8 bytes
    s64 u_cl;               // 8 bytes
    s64 var_cl;             // 8 bytes
    u32 alpha_fp;           // 4 bytes fixed point value containing 16 fractional bits
    u32 pid;                // 4 bytes
    u32 current_runtime;    // 4 bytes
    u16 sp_till_deadline;   // 2 bytes
    u16 sp_per_tp;          // 2 bytes

    u16 queue_length;   // 2 bytes

    //In a 2MB page, can have at most 2^15 elements of size 64B
    //The index of the next valid structure can be stored in a
    //short
    u16 next;   // 2 bytes
    u16 prev;   // 2 bytes

    u16 _padding[2];//2 bytes

    // 8 + 4*8 + 3*4 + 5*2 + 2*1= 64 bytes
} SRT_loaddata_t;

typedef struct _loaddata_list_header
{
    u64 prev_sp_boundary;       //8
    u64 scheduling_period;      //8

    u64 max_allocator_runtime;  //8
    u64 last_allocator_runtime; //8

    u64 energy_last_sp;         //8
    u64 icount_last_sp;         //8

    u16 SRT_count;              //2
    u16 first;                  //2
    
    /*The number of modes of operation (frequencies) in the system*/
    u16 mo_count;               //2
    
    //62 bytes so far. Add 2 bytes of padding to make it 64 bytes.
    u8  padding[10];            //10 bytes to make the structure 64 bytes
    
    /*Time spent in each mode of operation.*/
    /*Tail grown array of variable size since mo_count is unknown ahead of time*/
    u64 mostat_last_sp[1];
} loaddata_list_header_t;

#define LOADDATALIST_ORDER      (20-PAGE_SHIFT)
#define LOADDATALIST_SIZE       (PAGE_SIZE<<LOADDATALIST_ORDER) //1MB
#define LOADDATALIST_PAGEOFFSET (0)

#define ALLOC_ORDER             (17-PAGE_SHIFT)
#define ALLOC_SIZE              (PAGE_SIZE<<ALLOC_ORDER)
#define ALLOC_PAGEOFFSET        (LOADDATALIST_SIZE>>PAGE_SHIFT)

#define LAMbS_MODELS_ORDER      (1)
#define LAMbS_MODELS_SIZE       (PAGE_SIZE << LAMbS_MODELS_ORDER)
#define LAMbS_MODELS_OFFSET     (ALLOC_PAGEOFFSET + (ALLOC_SIZE >> PAGE_SHIFT))

/*The following is intended for 
    instructions retired per nanosecond
    microjoules  comsumed per nanosecond
*/
#define LAMbS_MODELS_FIXEDPOINT_SHIFT (48)

#endif /*#ifndef PBSALLOCATOR_CMD_INCLUDE*/
