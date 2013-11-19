#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static char *freq_line_buffer = NULL;
char **scaling_available_frequencies = NULL;
int freq_count = 0;

int cpufreq_get_available_frequencies(void)
{
    FILE *filep;

    size_t  freq_line_buffer_len = 0;
    ssize_t freq_line_len = 0; 
    
    char **prev_freq_list_buffer;
    char **new_freq_list_buffer;
    int  freq_count_local;
    
    char *current_tok = NULL;
        
    /*open the sys file*/
    filep = fopen(  "/sys/devices/system/cpu/cpu0/cpufreq/scaling_available_frequencies", 
                    "r");
    if(NULL == filep)
    {
        perror("cpufreq_get_available_frequencies: fopen failed");
        goto error0;
    }

    /*Read in the single line containing all the frequencies from the sys file*/
    freq_line_len = getline(&freq_line_buffer, &freq_line_buffer_len, filep);
    if(0 > freq_line_len)
    {
        perror("cpufreq_get_available_frequencies: getline failed");
        goto error1;
    }

    /*Extract the tokens from the line and grow the list
    prev_freq_list_buffer, filling it in with the new tokens*/
    prev_freq_list_buffer = NULL;
    freq_count_local = 0;
    
    current_tok = strtok(freq_line_buffer, " \t\n\v\f\r");
    while(NULL != current_tok)
    {
        freq_count_local++;
        new_freq_list_buffer = realloc( prev_freq_list_buffer, 
                                        (sizeof(char*) * freq_count_local));
        if(NULL == new_freq_list_buffer)
        {
            perror( "cpufreq_get_available_frequencies: realloc failed for "
                    "new_freq_list_buffer");
            goto error2;
        }
        
        prev_freq_list_buffer = new_freq_list_buffer;
        prev_freq_list_buffer[freq_count_local - 1] = current_tok;
        
        current_tok = strtok(NULL, " \t\n\v\f\r");
    }

    fclose(filep);

    /*Assign the local varriables to the global varriables*/
    scaling_available_frequencies = prev_freq_list_buffer;
    freq_count = freq_count_local;

    return 0;

error2:
    if(NULL != prev_freq_list_buffer)
    {
        free(prev_freq_list_buffer);
        prev_freq_list_buffer = NULL;
    }
error1:
    if(NULL != freq_line_buffer)
    {
        free(freq_line_buffer);
        freq_line_buffer = NULL;
    }
    fclose(filep);
error0:
    return -1;
}

void cpufreq_free(void)
{
    if(NULL != scaling_available_frequencies)
    {
        free(scaling_available_frequencies);
        scaling_available_frequencies = NULL;
    }
    freq_count = 0;

    if(NULL != freq_line_buffer)
    {
        free(freq_line_buffer);
        freq_line_buffer = NULL;
    }
}

int cpufreq_change_frequency(int setting_index)
{
    FILE *governor_file_p = NULL;
    FILE *speed_file_p = NULL;
    ssize_t bytes_written, setting_len;
    
    /*Check that cpufreq_get_available_frequencies has been called*/
    if(NULL == scaling_available_frequencies)
    {
        fprintf(stderr, "cpufreq_change_frequency: Not ready. Call "
                        "\"cpufreq_get_available_frequencies\" or set the pointer"
                        "\"scaling_available_frequencies\" to an appropriate array of "
                        "string values.\n");
        goto error0;
    }
    
    /*Check that the setting index is valid. (Prevent an array out-of-bound error)*/
    if( (setting_index < 0) || (setting_index >= freq_count))
    {
        fprintf(stderr, "cpufreq_change_frequency: "
                        "invalid argument %i (must be >= 0, < %i)",
                        setting_index, freq_count);
        goto error0;
    }
    
    /*Set the governor to the userspace governor*/
    governor_file_p = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor", "w");
    if(NULL == governor_file_p)
    {
        fprintf(stderr, "Failed to open file "
                        "\"/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor\".\n");
        perror("cpufreq_change_frequency: fopen failed");
        goto error0;
    }
    
    setting_len = strlen("userspace\n");
    bytes_written = fprintf(governor_file_p, "userspace\n");
    if(bytes_written < setting_len)
    {
        fprintf(stderr, "Failed to set the scaling governor to \"userspace\"");
        perror("cpufreq_change_frequency: fprintf failed");
        goto error1;
    }
    
    speed_file_p = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_setspeed", "w");
    if(NULL == speed_file_p)
    {
        fprintf(stderr, "Failed to open file "
                        "\"/sys/devices/system/cpu/cpu0/cpufreq/scaling_setspeed\".\n");
        perror("cpufreq_change_frequency: fopen failed");
        goto error1;
    }
    
    setting_len = strlen(scaling_available_frequencies[setting_index]) + 1;
    bytes_written = fprintf(speed_file_p, "%s\n", 
                            scaling_available_frequencies[setting_index]);
    if(bytes_written < setting_len)
    {
        fprintf(stderr, "Failed to set the scaling frequency to \"%s\"\n", 
                            scaling_available_frequencies[setting_index]);
        perror("cpufreq_change_frequency: fprintf failed");
        goto error2;
    }
    
    fclose(speed_file_p);
    fclose(governor_file_p);
    
    return 0;
error2:
    fclose(speed_file_p);
error1:
    fclose(governor_file_p);
error0:
    return -1;    
}

