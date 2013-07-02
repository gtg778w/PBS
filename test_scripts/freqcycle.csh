#! /bin/csh -x

    set freqdir="/sys/devices/system/cpu/cpu0/cpufreq/"
    set availablefreqs=`cat ${freqdir}/scaling_available_frequencies`
    
    foreach freq (${availablefreqs})
        echo $freq
    end
    
