
#include "LAMbS_energy.h"

int LAMbS_energy_init(void)
{
    /* This doesn't need much in the way of init, though checking CPUID
     * or something to make sure that RAPL is supported should be added */
    return 0;
}

void LAMbS_energy_uninit(void)
{
    /* doubt anything needs to be done here */
}

void LAMbS_energy_get(LAMbS_energy_t* ecount)
{
    unsigned long edx = 0, eax = 0;
    unsigned long ecx = MSR_PKG_ENERGY_STATUS;
    __asm__ __volatile__("rdmsr" : "=a"(eax), "=d"(edx) : "c"(ecx));

    ecount->cur_energy = eax | (u64)edx << 0x20;
}

void LAMbS_energy_getDelta(LAMbS_energy_t* ecount, u64 *delta_energy_p)
{
    u64 ecount_old;
    ecount_old = ecount->cur_energy;
    LAMbS_energy_get(ecount);
    
    if (ecount_old > ecount->cur_energy) /* overflow */ {
	*delta_energy_p = ecount->cur_energy + ((1 << sizeof(u32)) - ecount_old);
    } else {
	*delta_energy_p = ecount->cur_energy - ecount_old;
    }
    
    ecount->energy += *delta_energy_p;
}

