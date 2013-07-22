#ifndef LAMbS_INCLUDE
#define LAMbS_INCLUDE

#include <linux/kernel.h>

int LAMbS_init(void);
void LAMbS_uninit(void);

int LAMbS_measurements_alloc(void);
void LAMbS_models_measurements_init(void);
void LAMbS_measure_delta(   u64* delta_icount_p,
                            u64* delta_energy_p,
                            u64* delta_mostat);

void LAMbS_measurements_free(void);

#endif
