#ifndef LAMBS_MO_HEADER
#define LAMBS_MO_HEADER

#include <linux/kernel.h>

#include "LAMbS_molookup.h"
#include "LAMbS_mostat.h"

struct LAMbS_motrans_notifier_s;

typedef void (*LAMbS_motrans_callback_t)(   struct LAMbS_motrans_notifier_s *motrans_notifier_p,
                                            s32 old_moi, s32 new_moi);

struct LAMbS_motrans_notifier_s {
    struct list_head        chain_node;
    LAMbS_motrans_callback_t callback;
};

void LAMbS_motrans_register_notifier(struct LAMbS_motrans_notifier_s *notifier);
void LAMbS_motrans_unregister_notifier(struct LAMbS_motrans_notifier_s *notifier);

extern s32  LAMbS_current_moi;

int     LAMbS_mo_init(int verbose);
void    LAMbS_mo_uninit(void);

void LAMbS_motrans_notifier_starttest(void);
void LAMbS_motrans_notifier_stoptest(void);

#endif
