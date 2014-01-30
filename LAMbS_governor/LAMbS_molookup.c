#include <linux/kernel.h>

#include <linux/module.h>
/*
EXPORT_SYMBOL
*/

#include <linux/sort.h>
/*
    sort
*/

#include <linux/cpufreq.h>
/*
    struct cpufreq_frequency_table
    cpufreq_frequency_get_table
*/

#include <linux/smp.h>
/*
    smp_processor_id
*/

#include "LAMbS_molookup.h"

struct LAMbS_mo_struct LAMbS_mo_struct;

EXPORT_SYMBOL(LAMbS_mo_struct);

static void moswap(void *a, void *b, int size)
{
    int a_i, b_i;
    u32 temp_mo;
    u16 temp_moii;
    /*Some pointer arithmatic to get array indices*/
    a_i = ((u32*)a) - LAMbS_mo_struct.table;
    b_i = ((u32*)b) - LAMbS_mo_struct.table;
    
    /*Swap the frequencies*/
    temp_mo                     = LAMbS_mo_struct.table[a_i];
    LAMbS_mo_struct.table[a_i]  = LAMbS_mo_struct.table[b_i];
    LAMbS_mo_struct.table[b_i]  = temp_mo;

    /*Swap the frequency indices*/    
    temp_moii                               = LAMbS_mo_struct._internal_indices[a_i];
    LAMbS_mo_struct._internal_indices[a_i]  = LAMbS_mo_struct._internal_indices[b_i];
    LAMbS_mo_struct._internal_indices[b_i]  = temp_moii;
}

static int mocmp(const void *a, const void *b)
{
    int cmp = *((u32*)a) - *((u32*)b);
    cmp =   (cmp > 0)? 1 :
            ((cmp < 0)? -1: 0);
    return cmp;
}

/*Initialize the tables used by the function LAMbS_motoi*/
int LAMbS_molookup_init(void)
{
    s32 freqtable_i;

    s32 hashtable_i;
    s32 hashtable_i_start;

    struct cpufreq_frequency_table *freq_table;

    freq_table = cpufreq_frequency_get_table(smp_processor_id());

    /*Loop through the available frequencies and copy the table*/
    for (   freqtable_i = 0;
            (freq_table[freqtable_i].frequency != CPUFREQ_TABLE_END)
            && (freqtable_i < LAMbS_molookup_HASHSIZE); 
            freqtable_i++) 
    {
        LAMbS_mo_struct.table[freqtable_i] = freq_table[freqtable_i].frequency;
        LAMbS_mo_struct._internal_indices[freqtable_i] = freq_table[freqtable_i].index;        
    }

    /*Check if there are too many MO for the hash table*/
    if( (freq_table[freqtable_i].frequency != CPUFREQ_TABLE_END)
        && (freqtable_i >= LAMbS_molookup_HASHSIZE) )
    {
        printk(KERN_INFO "LAMbS_molookup_init: Currently the system is not designed to handle "
                        "more than %i mo.", LAMbS_molookup_HASHSIZE);
        goto error0;
    }
    else
    {
        /*Update LAMbS_mo_struct.count*/
        LAMbS_mo_struct.count = freqtable_i;
    }
    
    /*Sort the LAMbS_mo table*/
    sort(   LAMbS_mo_struct.table, 
            LAMbS_mo_struct.count, sizeof(u32), 
            mocmp, moswap);

    /*Initialize the hash table with -1s (an invalid MO table index)*/
    for(hashtable_i = 0; hashtable_i < LAMbS_molookup_HASHSIZE; hashtable_i++)
    {
        LAMbS_mo_struct.hashtable[hashtable_i] = -1;
    }
    
    /*Compute and populate the hashtable*/
    for(    freqtable_i = 0; 
            freqtable_i < LAMbS_mo_struct.count; 
            freqtable_i++)
    {
        /*Hash the MO to get a hash table index*/
        hashtable_i = LAMbS_molookup_hashfunc(LAMbS_mo_struct.table[freqtable_i]);
        
        /*Check if there is a collision with the hash value from another MO.
        If there is a collision, the hash table entry is valid (non-negative)*/
        if(0 <= LAMbS_mo_struct.hashtable[hashtable_i])
        {
            /*If there is a collision, loop through the hash table and find an entry
            with which there is no collision or the table is found to be full.*/
            hashtable_i_start = hashtable_i;
            do
            {
                hashtable_i = (hashtable_i + 1) % LAMbS_molookup_HASHSIZE;
                
                /*Check if the complete table has been searched*/
                if(hashtable_i_start == hashtable_i)
                {
                    printk(KERN_INFO "LAMbS_molookup_init: Cycled through the complete "
                                    "hash table. No free spots.");
                    goto error0;
                }
                
            }while(0 <= LAMbS_mo_struct.hashtable[hashtable_i]);
        }

        /*Insert the MO table index, moi, into the hash table at index hashtable_i*/
        LAMbS_mo_struct.hashtable[hashtable_i] = freqtable_i;
    }
    
    return 0;
    
error0:
    LAMbS_mo_struct.count = 1;
    return -1;
}

/*Free tables used by the function LAMbS_motoi*/
void LAMbS_molookup_uninit(void)
{
    LAMbS_mo_struct.count = 1;
}

int LAMbS_molookup_test(int verbose)
{
    int ret = 0;
    u32 mo;
    s32 mo_lookup;
    s32 moi;
    
    if(0 != verbose)
    {
        printk(KERN_INFO "LAMbS_mollokup: %i modes of operation", LAMbS_mo_struct.count);
        printk(KERN_INFO "{");
    }
    
    for(moi = 0; moi < LAMbS_mo_struct.count; moi++)
    {
        mo = LAMbS_mo_struct.table[moi];
        mo_lookup = LAMbS_molookup(mo);
        
        if(0 != verbose)
        {
            printk(KERN_INFO "\t%i) %i, %i", moi, mo, mo_lookup);
        }
            
        if(moi != mo_lookup)
        {
            ret = -1;
        }
    }
    
    if(0 != verbose)
    {
        printk(KERN_INFO "}");
    }
    
    return ret;
}

