#!/bin/bash
echo userspace > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
rmmod gov_test/freq_change_test.ko
rmmod gov_test/gov_test.ko

