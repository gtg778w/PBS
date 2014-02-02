# This file is meant to be sourced from other CSH files.

    #The relevant folder for CPU speed settings
    set freqdir="/sys/devices/system/cpu/cpu0/cpufreq"
    #The list of available frequencies
    set availablefreqs=`cat ${freqdir}"/scaling_available_frequencies"`
    #Set the scaling governor to userspace
    echo "userspace" > ${freqdir}"/scaling_governor"

