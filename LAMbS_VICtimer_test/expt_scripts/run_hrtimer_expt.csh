#! /bin/csh

    set test_log_dir="./data/hrtimer"
    mkdir -p ${test_log_dir}
    
    set bindir="/home/gtg778w/Desktop/bin"
    
    set test_mod_dir="/media/Data/Research/expt_February2013/PBS/LAMbS_VICtimer_test/mod"
    set mod="${test_mod_dir}/hrtimer_response_test.ko"

    set freqdir="/sys/devices/system/cpu/cpu0/cpufreq"
    set setspeed="${freqdir}/scaling_setspeed"
    set availablefreqs=`cat ${freqdir}/scaling_available_frequencies`    

    @ freq_i = $#availablefreqs
    set max_freq=$availablefreqs[1]
    set min_freq=$availablefreqs[${freq_i}]

    set test_length="5000"
    set test_interval_array=("100000" "200000" "500000" "1000000" "2000000" "5000000" "10000000" "50000000")
    
    set rp_interval="10000000"
    
    set cpufreq_steptime=`echo "($rp_interval * 10) / 1000000000" | bc -l`
        
    set cpufreq_name_array=("${max_freq}" "${min_freq}" "cycle")
    @ testcount = $#test_interval_array * $#cpufreq_name_array
    @ testidx =  1

    foreach testinterval (${test_interval_array})
        set testtime=`echo "$testinterval" "*" "$test_length" | bc -l`
        set testtime=`echo "$testtime / 1000000000" | bc -l`
        set cpufreq_cycle_time=`echo "$testtime + 5.0" | bc -l`
        set testtime=`echo "$testtime + 10.0" | bc -l`
        
        set cpufreq_command_array=( "echo ${max_freq} > ${setspeed}" \
                                    "echo ${min_freq} > ${setspeed}" \
                                    "${bindir}/cpufreq_step ${cpufreq_cycle_time} ${cpufreq_steptime} 0")
        
        foreach cpufreqi (`seq 1 1 $#cpufreq_name_array`)
            set cpufreq_name=$cpufreq_name_array[$cpufreqi]
            set cpufreq_command="$cpufreq_command_array[$cpufreqi]"
            
            set logfile_name="${test_log_dir}/hrtimer_response_test_${test_length}_${testinterval}_${cpufreq_name}.csv"
            
            echo
            echo "Experiment ${testidx} of ${testcount}"
            echo "Interval=${testinterval} CPU Frequency Configuration=${cpufreq_name}"
            echo "Duration of experiment: $testtime"
            echo
            
            csh -xc "${cpufreq_command}"
            insmod $mod test_length=${test_length} interval_base=${testinterval}
            sleep $testtime
            rmmod  $mod
            cp /var/log/hrtimer_response_test_log.csv ${logfile_name}
            
            @ testidx =  ${testidx} + 1
        end
    end
