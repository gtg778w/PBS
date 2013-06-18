#ifndef LAMbS_ENERGY_INCLUDE
#define LAMbS_ENERGY_INCLUDE

#include <linux/kernel.h>

typedef struct LAMbS_energy_s
{
    u64 energy;    
} LAMbS_energy_t;

int LAMbS_energy_init(void);
void LAMbS_energy_uninit(void);

void LAMbS_energy_get(LAMbS_energy_t* icount);
void LAMbS_energy_getDelta(LAMbS_energy_t* icount, u64 *delta_energy_p);

#endif
