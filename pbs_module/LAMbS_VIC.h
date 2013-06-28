#ifndef LAMbS_VIC_INCLUDE
#define LAMbS_VIC_INCLUDE

#include <linux/kernel.h>

int     LAMbS_VIC_init(void);
u64     LAMbS_VIC_get(void);
void    LAMbS_VIC_uninit(void);

#endif
