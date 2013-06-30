#ifndef PBA_INCLUDE
#define PBA_INCLUDE

#include "jb_mgt.h"

void pba_firstjob(struct SRT_struct *ss);
void pba_nextjob(struct SRT_struct *ss);
void pba_nextjob2(struct SRT_struct *ss);
u64 pba_get_jbruntime(struct pba_struct *pba_struct_p);

void pba_refresh_budget(struct SRT_struct *SRT_struct_p);

#define pba_set_budget(SRT_struct_p, budget) \
(SRT_struct_p->pba_struct.sp_budget = budget)

void pba_init(struct SRT_struct *SRT_struct_p);
void pba_uninit(struct SRT_struct *SRT_struct_p);

#endif
