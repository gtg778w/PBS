#! /bin/bash
sudo echo -1 > /proc/sys/kernel/sched_rt_runtime_us
cat /proc/sys/kernel/sched_rt_runtime_us

