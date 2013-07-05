#ifndef CPUFREQ_HELPER_INCLUDE
#define CPUFREQ_HELPER_INCLUDE

    extern char *scaling_available_frequencies[];
    extern int freq_count;

    int cpufreq_get_available_frequencies(void);
    void cpufreq_free(void);
    int cpufreq_change_frequency(int setting_index);

#endif
