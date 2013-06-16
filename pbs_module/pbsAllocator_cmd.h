#ifndef PBSALLOCATOR_CMD_INCLUDE
#define PBSALLOCATOR_CMD_INCLUDE


#define PBS_BWMGT_CMD_SETUP_START (0)
/*
    0) scheduling period (us)
    1) allocator runtime (us)
*/

#define PBS_BWMGT_CMD_NEXTJOB   (1)
/*
*/

#define PBS_BWMGT_CMD_STOP      (2)
/*
*/

#define PBS_BWMGT_CMD_MAX       (3)
#define PBS_BWMGT_CMD_MAXARGS   (2)
typedef struct bw_mgt_cmd_s
{
    int             cmd;
    s64         args[2];
} bw_mgt_cmd_t;

#define LOADDATALIST_SIZE   (1<<20) //1MB
#define LOADDATALIST_ORDER  (20-PAGE_SHIFT)

#define ALLOC_SIZE  (1<<17)
#define ALLOC_ORDER (17-PAGE_SHIFT)

typedef struct _SRT_loaddata
{
    u64 job_release_time;   // 8 bytes
    s64 u_c0;               // 8 bytes
    s64 var_c0;             // 8 bytes
    s64 u_cl;               // 8 bytes
    s64 var_cl;             // 8 bytes
    u32 pid;                // 4 bytes
    u32 current_runtime;    // 4 bytes
    u32 sp_till_deadline;   // 4 bytes
    u32 sp_per_tp;          // 4 bytes

    unsigned short  queue_length;   // 2 bytes

    //In a 2MB page, can have at most 2^15 elements of size 64B
    //The index of the next valid structure can be stored in a
    //short
    unsigned short  next;   // 2 bytes
    unsigned short  prev;   // 2 bytes

    unsigned char  _padding[2];//2 bytes

    // 8 + 4*8 + 4*4 + 3*2 + 2*1= 64 bytes
} SRT_loaddata_t;

typedef struct _loaddata_list_header
{
    u64 prev_sp_boundary;       //8
    u64 scheduling_period;      //8

    u64 max_allocator_runtime;  //8
    u64 last_allocator_runtime; //8

    u16 SRT_count;              //2
    u16 first;                  //2
    
    //36 bytes so far.
    u8  padding[28];            //28 bytes to make the structure 64 bytes
    
} loaddata_list_header_t;

/*This is defined as a macro to allow for a tail-grown array inside the structure,
later in the development*/
#define sizeof_loaddata_header() (sizeof(loaddata_list_header_t))

#endif /*#ifndef PBSALLOCATOR_CMD_INCLUDE*/
