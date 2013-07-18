#ifndef LAMBS_MOLOOKUP_HEADER
#define LAMBS_MOLOOKUP_HEADER

#define LAMbS_molookup_HASHMOD (257)
#define LAMbS_molookup_HASHSIZE LAMbS_molookup_HASHMOD

#define LAMbS_molookup_hashfunc(x) (x % LAMbS_molookup_HASHMOD) 

/*The maximum number of modes of operation that can be handled*/
#define LAMbS_mo_MAXCOUNT (LAMbS_molookup_HASHSIZE)

struct LAMbS_mo_struct
{
    /*The array of frequencies supported by the system*/
    u32 table[LAMbS_mo_MAXCOUNT];

    /*hash table for reverse lookup to go from frequency to array index*/
    u32 hashtable[LAMbS_mo_MAXCOUNT];

    /*The number of modes of operation in the system*/
    u16 count;

    /*The index indicated by cpufreq_frequency_get_table for the
    corresponding mo value in the table field*/
    u16 _internal_indices[LAMbS_mo_MAXCOUNT];
};

extern struct LAMbS_mo_struct LAMbS_mo_struct;

/*convert an mo identifier like frequency to an index*/
/*The following function has been optimized for the
common case where the desired MO exists in the MO table
and there is no collision in the hash table*/
static inline int LAMbS_molookup(int mo)
{
    int found_mo;
    int moi;
    int hashtable_start_i = LAMbS_molookup_hashfunc(mo);
    int hashtable_i = hashtable_start_i;
    
    /*Find the desired MO*/
    do
    {
        moi = LAMbS_mo_struct.hashtable[hashtable_i];
        found_mo = LAMbS_mo_struct.table[moi];
        
        /*If there is a collision and the desired MO is not found*/
        if(found_mo != mo)
        {
            /*Look at the next spot in the table*/
            hashtable_i = (hashtable_i + 1) % LAMbS_molookup_HASHSIZE;
            
            /*If the entire table has been searched*/
            if(hashtable_i != hashtable_start_i)
            {
                /*return -1*/
                return -1;
            }
        }
        else
        {
            /*The desired mo has been found. Return the index*/
            return moi;
        }

    }while(1);
}

/*Initialize the tables used by the function LAMbS_motoi*/
int LAMbS_molookup_init(void);

/*Free tables used by the function LAMbS_motoi*/
void LAMbS_molookup_uninit(void);

/*Test the MO lookup mechanism*/
int LAMbS_molookup_test(int verbose);

#endif

