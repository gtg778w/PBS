#include <linux/module.h>
/*
MODULE_AUTHOR
MODULE_LICENSE
THIS_MODULE

module_init
module_exit
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

/*Currently, MO is synonymous with frequency*/
int LAMbS_mo_count = 0;

/*Maps hash values to entries in the mo table*/
int LAMbS_mo[LAMbS_molookup_HASHSIZE];
int LAMbS_molookup_hashtable[LAMbS_molookup_HASHSIZE];

/*Initialize the tables used by the function LAMbS_motoi*/
int LAMbS_molookup_init(void)
{
    int cpu;
    int freqtable_i;

    int max_index = 0;
    int hashtable_i;
    int hashtable_i_start;

    int moi;

    struct cpufreq_frequency_table *freq_table;

    cpu = smp_processor_id();
    freq_table = cpufreq_frequency_get_table(cpu);
    
    /*Loop through the available frequencies to determine the number of entries
    necessary for the MO table*/
    for (   freqtable_i = 0; 
            freq_table[freqtable_i].frequency != CPUFREQ_TABLE_END; 
            freqtable_i++) 
    {
        max_index = (max_index < freq_table[freqtable_i].index)? 
                        freq_table[freqtable_i].index :
                        max_index;
    }
    LAMbS_mo_count = max_index+1;
    
    /*Check if there are too many MO for the hash table*/
    if(LAMbS_mo_count > LAMbS_molookup_HASHSIZE)
    {
        printk(KERN_INFO "LAMbS_molookup_init: Currently the system is not designed to handle "
                        "more than %i mo.", LAMbS_molookup_HASHSIZE);
        goto error0;
    }
    
    /*Fill the MO table with -1 (an invalid MO)*/
    for(moi = 0; moi < LAMbS_mo_count; moi++)
    {
        LAMbS_mo[moi] = -1;
    }
    
    /*Fill in the hash table with -1 (an invalid MO table index)*/
    for(hashtable_i = 0; hashtable_i < LAMbS_molookup_HASHSIZE; hashtable_i++)
    {
        LAMbS_molookup_hashtable[hashtable_i] = -1;
    }
    
    /*Fill in the tables with valid values at the appropriate indices*/
    for(    freqtable_i = 0; 
            freq_table[freqtable_i].frequency != CPUFREQ_TABLE_END; 
            freqtable_i++) 
    {
        /*Get the idnex of the current MO*/
        moi = freq_table[freqtable_i].index;
        
        /*Insert the MO into the corresponding index of the MO table*/
        LAMbS_mo[moi] = freq_table[freqtable_i].frequency;
        
        /*Hash the MO to get a hash table index*/
        hashtable_i = LAMbS_molookup_hashfunc(freq_table[freqtable_i].frequency);
        
        /*Check if there is a collision with the hash value from another MO.
        If there is a collision, the hash table entry is valid (non-negative)*/
        if(0 <= LAMbS_molookup_hashtable[hashtable_i])
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
                    printk(KERN_INFO "LAMbS_molookup_init: Cycled through the complete hash "
                                    "table. No free spots.");
                    goto error0;
                }
                
            }while(0 <= LAMbS_molookup_hashtable[hashtable_i]);
        }

        /*Insert the MO table index, moi, into the hash table at index hashtable_i*/
        LAMbS_molookup_hashtable[hashtable_i] = moi;
    }
    
    return 0;
    
error0:
    LAMbS_mo_count = 0;
    return -1;
}

/*Free tables used by the function LAMbS_motoi*/
void LAMbS_molookup_free(void)
{
    LAMbS_mo_count = 0;
}

int LAMbS_molookup_test(void)
{
    int ret = 0;
    int moi, mo, mo_lookup;
    
    printk(KERN_INFO "LAMbS_mollokup: %i modes of operation", LAMbS_mo_count);
    printk(KERN_INFO "{");
    for(moi = 0; moi < LAMbS_mo_count; moi++)
    {
        mo = LAMbS_mo[moi];
        mo_lookup = LAMbS_molookup(mo);
        printk(KERN_INFO "\t%i) %i, %i", moi, mo, mo_lookup);
        
        if(moi != mo_lookup)
        {
            ret = -1;
        }
    }
    printk(KERN_INFO "}");
    
    return ret;
}

