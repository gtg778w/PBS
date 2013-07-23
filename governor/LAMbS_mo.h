#ifndef LAMBS_MO_HEADER
#define LAMBS_MO_HEADER

#include "LAMbS_molookup.h"
#include "LAMbS_mostat.h"

extern u64  LAMbS_current_moi;

int     LAMbS_mo_init(int verbose);
void    LAMbS_mo_uninit(void);

#endif