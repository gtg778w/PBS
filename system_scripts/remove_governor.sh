#!/bin/bash
echo userspace > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
rmmod perf_event_test/perf_event_test.ko
rmmod gov_test/gov_test.ko

