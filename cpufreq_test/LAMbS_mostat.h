#ifndef LAMBS_MOSTAT_HEADER
#define LAMBS_MOSTAT_HEADER

#include <asm-generic/int-ll64.h>
/*
u64
*/

struct mostat_s
{
    u64 last_transition_time;
    int last_moi;
    /*The following field is a variable length array*/
    u64 stat[LAMbS_mo_MAXCOUNT];
};

extern struct mostat_s mostat;

extern void (*LAMbS_mostat_transition_p)(int old_moi, int new_moi);

/*This macro should be called with interrupts disabled to prevent the value of 
mostat.last_moi from chaning between the time it is read in the caller and the time
it is checked in LAMbS_mostat_transition */
#define _LAMbS_mostat_update() LAMbS_mostat_transition_p(mostat.last_moi, mostat.last_moi)

int LAMbS_mostat_init(void);
void LAMbS_mostat_free(void);

#endif
