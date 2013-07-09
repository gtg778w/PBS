#ifndef LAMbS_MODELS_INCLUDE
#define LAMbS_MODELS_INCLUDE

#include <linux/kernel.h>

#include "pbsAllocator_cmd.h"
/*
For the page size and order
*/

#include "LAMbS_molookup.h"
/*
For LAMbS_mo_count
*/

extern struct  page *LAMbS_models_pages;

extern u64  *instruction_retirement_rate;
extern u64  *instruction_retirement_rate_inv;
extern u64  *om_schedule;

int     LAMbS_models_alloc_pages(void);
void    LAMbS_models_free_pages(void);

void    LAMbS_models_init(void);
void    LAMbS_models_process_omschedule(void);

static inline u64 LAMBS_models_multiply_shift(u64 opA, u64 opB, unsigned int shift)
{
    u64 hiA, loA, hiB, loB;
    u64 hihiC, hiloC, lohiC, loloC;
    u64 C;
    
    /*Extract the hi and lo parts of the operands*/
    /*All operands are unsigned. There should be no sign extension*/
    hiA = (opA >> 32);
    loA = (opA);

    hiB = (opB >> 32);
    loB = (opB);
    
    /*initial multiply operation: 128bit C = 64bit A * 64bit B*/
    hihiC = hiA * hiB;  /* x (2 ^ 64) */
    hiloC = hiA * loB;  /* x (2 ^ 32) */
    lohiC = loA * hiB;  /* x (2 ^ 32) */
    loloC = (loA * loB);/* x (2 ^ 0)  */
    
    /*Shift the parts as appropriate according to the shift argument. It is assumed the
    shift argument is less than 64*/
    /*If this function is inlined and the shift operand is assigned from a constant, then 
    it should be possible to optimize out the branch statements.
    */
    hihiC = hihiC << (64 - shift);
    if(shift > 32)
    {
        hiloC = hiloC >> (shift - 32);
        lohiC = lohiC >> (shift - 32);
    }
    else if(shift < 32)
    {
        hiloC = hiloC << (32 - shift);
        lohiC = lohiC << (32 - shift);
    }
    loloC = loloC >> shift;
    
    C = hihiC + hiloC + lohiC + loloC;
    
    return C;
}

#endif
