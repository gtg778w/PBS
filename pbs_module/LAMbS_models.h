#ifndef LAMbS_MODELS_INCLUDE
#define LAMbS_MODELS_INCLUDE

#include <linux/kernel.h>

#include "pbsAllocator_cmd.h"
/*
    page size and order
*/

#include "LAMbS_molookup.h"
/*
    LAMbS_mo_struct
*/

#include "LAMbS_mo.h"
/*
    LAMbS_motrans_notifier_s
*/

extern struct  page *LAMbS_models_pages;

extern u64  *instruction_retirement_rate;
extern u64  *instruction_retirement_rate_inv;
extern u64  *om_schedule;

extern u64  LAMbS_current_instretirementrate;
extern u64  LAMbS_current_instretirementrate_inv;

extern struct LAMbS_motrans_notifier_s LAMbS_models_motrans_notifier;
void    _LAMbS_models_motrans_callback(  
                                struct LAMbS_motrans_notifier_s *motrans_notifier_p,
                                s32 old_moi,    s32 new_moi);
                                
#define LAMbS_update_current_instrate() \
            _LAMbS_models_motrans_callback( &LAMbS_models_motrans_notifier,\
                                            LAMbS_current_moi, LAMbS_current_moi)

int     LAMbS_models_alloc_pages(void);
void    LAMbS_models_free_pages(void);

void    LAMbS_models_init(void);
void    LAMbS_models_process_omschedule(void);

static inline s64 LAMBS_models_multiply_shift(s64 opA, s64 opB, unsigned int shift)
{
    s64 hiA, loA, hiB, loB, signA, signB;
    s64 hihiC, hiloC, lohiC, loloC;
    s64 signC, C;
    
    /*Extract and check the sign bit of the operand and negate the operand if necessary*/
    signA   = (opA >> 63);
    opA = (signA != 0)? -opA : opA;
    /*Extract the hi and lo parts of the operands*/
    hiA = (opA >> 32) & 0xffffffff;
    loA = (opA) & 0xffffffff;

    /*Extract and check the sign bit of the operand and negate the operand if necessary*/
    signB   = (opB >> 63);
    opB = (signB != 0)? -opB : opB;
    /*Extract the hi and lo parts of the operands*/
    hiB = (opB >> 32) & 0xffffffff;
    loB = (opB) & 0xffffffff;
    
    /*initial multiply operation: 128bit C = 64bit A * 64bit B*/
    hihiC = hiA * hiB;  /* x (2 ^ 64) / (2 ^ shift)*/
    hiloC = hiA * loB;  /* x (2 ^ 32) / (2 ^ shift)*/
    lohiC = loA * hiB;  /* x (2 ^ 32) / (2 ^ shift)*/
    loloC = (loA * loB);/* x (2 ^ 0)  / (2 ^ shift)*/
    
    /*Shift the parts as appropriate according to the shift argument. It is assumed the
    shift argument is less than 64*/
    /*If this function is inlined and the shift operand is assigned from a constant, then 
    it should be possible to optimize out the branch statements. Even if the branch is 
    not optimized out, the consistent branch direction should allow for good branch
    predictor performance.
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
    
    /*The sign of the output should only be negative if signA and signB or unequal*/
    signC   = signA ^ signB;
    
    /*Check signC and negate if necessary*/
    C = (signC != 0)? -C : C;
    
    return C;
}

#endif
