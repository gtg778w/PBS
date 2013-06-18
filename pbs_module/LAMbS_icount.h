#ifndef LAMbS_ICOUNT_INCLUDE
#define LAMbS_ICOUNT_INCLUDE
#include <linux/kernel.h>

typedef struct LAMbS_icount_s
{
    u64 icount;
} LAMbS_icount_t;

int LAMbS_icount_init(void);
void LAMbS_icount_uninit(void);

void LAMbS_icount_get(LAMbS_icount_t* mostat);
void LAMbS_icount_getDelta(LAMbS_icount_t* mostat, u64 *delta_icount_p);

#endif
