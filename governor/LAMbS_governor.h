#ifndef LAMBS_GOVERNOR_H
#define LAMBS_GOVERNOR_H

struct LAMbS_mo_struct
{
    /*The array of frequencies supported by the system*/
    u32 table[LAMbS_mo_MAXCOUNT];

    /*hash table for reverse lookup to go from frequency to array index*/
    s32 hashtable[LAMbS_mo_MAXCOUNT];

    /*The number of modes of operation in the system*/
    u16 count;

    /*The index indicated by cpufreq_frequency_get_table for the
    
    corresponding mo value in the table field*/
    u16 _internal_indices[LAMbS_mo_MAXCOUNT];
};

extern struct LAMbS_mo_struct LAMbS_mo_struct;

static int LAMbS_cpufreq_notifier(struct notifier_block *nb, unsigned long val, void *data);
static int schedule_next_moi(void);
int LAMbS_cpufreq_sched(u64 LAMbS_mo_schedule);
static int LAMbS_freq_set(u32 freq);
static int cpufreq_governor_lambs(struct cpufreq_policy *policy, unsigned int event);
static int __init cpufreq_gov_lambs_init(void);
static int __exit cpufreq_gov_lambs_exit(void);

#endif

