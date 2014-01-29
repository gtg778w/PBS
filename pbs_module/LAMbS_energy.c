
#include "LAMbS_energy.h"

static u64 energy_shift_left;

int LAMbS_energy_init(void)
{
    u64 energy_status_units;
    
    /* This doesn't need much in the way of init, though checking CPUID
     * or something to make sure that RAPL is supported should be added */

    /*Initialize the multiplier*/
    unsigned long edx = 0, eax = 0;
    unsigned long ecx = MSR_RAPL_POWER_UNIT;
    __asm__ __volatile__("rdmsr" : "=a"(eax), "=d"(edx) : "c"(ecx));

    /*eax contains low 32 bits*/
    /*bits 8 through 12 contain the energy status units*/
    energy_status_units = (eax >> 8) & (0x1f);

    energy_shift_left = 16 - energy_status_units;

    return 0;
}

void LAMbS_energy_uninit(void)
{
    /* doubt anything needs to be done here */
}

void LAMbS_energy_get(LAMbS_energy_t* ecount)
{
    u64 energy;
    unsigned long edx = 0, eax = 0;
    unsigned long ecx = MSR_PKG_ENERGY_STATUS;
    __asm__ __volatile__("rdmsr" : "=a"(eax), "=d"(edx) : "c"(ecx));
    energy = eax | (u64)edx << 0x20;
    
    if(energy_shift_left > 0)
    {
        energy = energy << energy_shift_left;
    }
    else if(energy_shift_left < 0)
    {
        energy = energy >> (-energy_shift_left);
    }
    
    ecount->cur_energy = energy;
}

void LAMbS_energy_getDelta(LAMbS_energy_t* ecount, u64 *delta_energy_p)
{
    u64 ecount_old;
    ecount_old = ecount->cur_energy;
    LAMbS_energy_get(ecount);
    
    if (ecount_old > ecount->cur_energy) /* overflow */ {
	*delta_energy_p = ecount->cur_energy + (((u64)1 << (8*sizeof(u32))) - ecount_old);
    } else {
	*delta_energy_p = ecount->cur_energy - ecount_old;
    }
    
    ecount->energy += *delta_energy_p;
}

