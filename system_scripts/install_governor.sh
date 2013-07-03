#!/bin/bash
insmod gov_test/gov_test.ko
echo lambs > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
insmod gov_test/freq_change_test.ko

