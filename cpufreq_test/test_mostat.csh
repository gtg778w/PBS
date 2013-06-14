#! /bin/csh -x

rm time_in_state.txt

#save the current cpu governor and scaling frequency
set saved_governor=`cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor`
echo "userspace" >> /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor

set saved_frequency=`cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq`

set available_frequencies=`cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_available_frequencies`
set noof_frequencies=$#available_frequencies
@ min_frequency_i = $noof_frequencies

echo $available_frequencies[1] >> /sys/devices/system/cpu/cpu0/cpufreq/scaling_setspeed

insmod cpufreq_test.ko
echo "start:" >> time_in_state.txt
cat /sys/devices/system/cpu/cpu0/cpufreq/stats/time_in_state >> time_in_state.txt

sleep 2
echo $available_frequencies[$min_frequency_i] >> /sys/devices/system/cpu/cpu0/cpufreq/scaling_setspeed

sleep 2
echo $available_frequencies[1] >> /sys/devices/system/cpu/cpu0/cpufreq/scaling_setspeed

sleep 2
echo $available_frequencies[$min_frequency_i] >> /sys/devices/system/cpu/cpu0/cpufreq/scaling_setspeed

sleep 2
echo $available_frequencies[1] >> /sys/devices/system/cpu/cpu0/cpufreq/scaling_setspeed

sleep 2
echo $available_frequencies[$min_frequency_i] >> /sys/devices/system/cpu/cpu0/cpufreq/scaling_setspeed

sleep 2
echo $available_frequencies[1] >> /sys/devices/system/cpu/cpu0/cpufreq/scaling_setspeed

rmmod  cpufreq_test.ko
echo "end:" >> time_in_state.txt
cat /sys/devices/system/cpu/cpu0/cpufreq/stats/time_in_state >> time_in_state.txt
echo >> time_in_stat.txt
echo "dmesg output: " >> time_in_state.txt
dmesg | tail -n 30 >> time_in_state.txt

#restore the saved cpu governor and scaling frequency
echo ${saved_governor} >> /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
echo ${saved_frequency} >> /sys/devices/system/cpu/cpu0/cpufreq/scaling_setspeed

