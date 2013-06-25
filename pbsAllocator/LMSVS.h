#include <stdlib.h>
#include <stdint.h>
#include <float.h>

#define LMSVS_EPS (1)

#define LMSVS_MAX_STEPSIZE (1.5)
#define LMSVS_MIN_STEPSIZE (0.0125)
#define LMSVS_alpha (2.0)
#define LMSVS_1overalpha (1/LMSVS_alpha)

/*The m0 and m1 thresholds should not
be more than 127 so that the sign counter
can be stored in 1 byte of storage*/
#define LMSVS_m0 (2)
#define LMSVS_m1 (3)

typedef struct LMSVS_s
{
    int32_t taps;

    /*positive values indiciate consecutive sign changes*/
    /*negative values indiciate consecutive fixed sign values*/
    int8_t  *previous_sign;
    int8_t  *schange_count;

    /*to allow this to be a variable length structure
      to handle a variable number of taps*/
    double  step_size[1];
} LMSVS_t;

#define LMSVS_size(taps) (sizeof(LMSVS_t) + (sizeof(double)*(taps-1)) + (sizeof(int8_t)*(taps*2)))

#define LMSVS_malloc(taps) ((LMSVS_t*)malloc(LMSVS_size(taps)))

void LMSVS_init(LMSVS_t *LMSVS_p, int32_t taps_in);

double LMSVS_predict(LMSVS_t *LMSVS_p, double *x_hat, double *coeffs);

void LMSVS_update(LMSVS_t *LMSVS_p, double *x_hat, double *coeffs, double observed);

