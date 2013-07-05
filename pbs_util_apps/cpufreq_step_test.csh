#! /bin/csh

    set outer_iterations="10"
    set inner_delay="1.0"
    
    if ($#argv > 0) then
        set outer_iterations=$1
    endif
    
    if ($#argv > 1) then
        set inner_delay=$2
    endif

    if ($#argv > 2) then
        echo "Usage: $0 outer_iterations inner_delay"
        echo "prints the current frequency outer_iterations many times, separated by delays of inner_delay seconds"
        goto done
    endif
        
    foreach freq (`seq 1 1 ${outer_iterations}`)
        cat "/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq"
        sleep ${inner_delay}
    end
    
    exit 0
error0:
    exit 1    

