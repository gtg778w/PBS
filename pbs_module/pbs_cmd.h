
#define PBS_JBMGT_CMD_SETUP     (0)
/*
    0) period (us)
    1) alpha  (x65536 i.e. fixed point value with 16bits after the point)
*/

#define PBS_JBMGT_CMD_START     (1)
/*
*/

#define PBS_JBMGT_CMD_NEXTJOB   (2)
/*
    0) u_c1
    1) std_c1
    2) u_cl
    3) std_cl
*/

#define PBS_JBMGT_CMD_STOP      (3)
/*
*/

#define PBS_JBMGT_CMD_MAX       (4)
#define PBS_JBMGT_CMD_MAXARGS   (4)
typedef struct job_mgt_cmd_s
{
    int             cmd;
    unsigned long   args[4];
} job_mgt_cmd_t;


struct SRT_job_log
{
	u64	abs_releaseTime;
	u64	abs_LFT;
	u32	runtime;
	u32 runtime2;

    u32	last_sp_compt_allocated;
	u32	last_sp_compt_used_sofar;

    //FIXME
    u32 throttle_count;
    u32 switch_count;
    unsigned char miss;
};

/*FIXME: This should be removed along with the rest of the ioctl code*/
#define PBS_IOCTL_JBMGT_PERIOD 		0
#define PBS_IOCTL_JBMGT_SRT_RUNTIME		1
#define PBS_IOCTL_JBMGT_SRT_HISTLEN		3
#define PBS_IOCTL_JBMGT_START			4
#define PBS_IOCTL_JBMGT_MAX			5

