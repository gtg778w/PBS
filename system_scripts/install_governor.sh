#!/bin/bash
insmod ../governor/LAMbS_governor.ko
echo lambs > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
insmod ../governor/freq_change_test.ko
