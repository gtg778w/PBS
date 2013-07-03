#!/bin/bash
insmod gov_test/gov_test.ko
echo lambs > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
insmod perf_event_test/perf_event_test.ko

