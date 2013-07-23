#ifndef LAMBS_MOSTAT_HEADER
#define LAMBS_MOSTAT_HEADER

#include <linux/kernel.h>

#include "LAMbS_molookup.h"

int LAMbS_mostat_init(void);
void LAMbS_mostat_uninit(void);

typedef struct LAMbS_mostat_s
{
    u64 time_stamp;
    u64 stat[1];
} LAMbS_mostat_t;

#define LAMbS_mostat_size() (   sizeof(LAMbS_mostat_t) + \
                                (sizeof(u64) * (LAMbS_mo_struct.count-1)))

LAMbS_mostat_t* LAMbS_mostat_alloc(void);
void LAMbS_mostat_free(LAMbS_mostat_t* mostat);
void LAMbS_mostat_get(LAMbS_mostat_t* mostat);
void LAMbS_mostat_getDelta(LAMbS_mostat_t* mostat, u64 *delta_mostat);

#endif
