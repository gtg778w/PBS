
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

