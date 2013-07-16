#ifndef LAMBS_MO_HEADER
#define LAMBS_MO_HEADER

#include "LAMbS_molookup.h"
#include "LAMbS_mostat.h"

int     LAMbS_mo_init(int verbose);
void    LAMbS_mo_uninit(void);
void    LAMbS_mo_modelupdate_notify(void);

extern u64  LAMbS_current_moi;
extern u64  LAMbS_current_instretirementrate;
extern u64  LAMbS_current_instretirementrate_inv;
#endif
