#! /bin/csh -x

    # Three types of log files
    #   - a general log file to report on the experiments progress
    #   - a log file for the allocator task for each run
    #   - a log file for each SRT task

    # Format for SRT task logs file name:
    # <experiment number>_<task_number>_sqrwavSRT_<-j>_<-P>_<-D>_<-d>_<-M>_<-m>_<-N>_<-p>_<-a>.log

    # Format for the allocator task log file name:
    # <experiment number>_pbs_ul_allocator_<-a>_<-s>.log

    set freqdir="/sys/devices/system/cpu/cpu0/cpufreq/"
    set availablefreqs=`cat ${freqdir}/scaling_available_frequencies`    

    set sa="12100"

    set j1="2700"
    set P1="2700"
    set D1="0.33334"
    set d1="1200"
    set M1="3808000"
    set m1="2808000"
    set N1="0.2"
    set A1="mabank"
    set p1="40000000"
    set b1="16000000"
    set a1="0.75"
    set logfilesuffix1="_sqrwavSRT_"${j1}"_"${P1}"_"${D1}"_"${d1}"_"${M1}"_"${m1}"_"${N1}"_"${A1}"_"${p1}"_"${b1}"_"${a1}".log"

    echo "Training run: ... "
    echo 0 > /proc/sched_pbs_actv
    bin/pbsAllocator -f -s 0 -S &
    sleep 1
    bin/sqrwavSRT -f -j 16 -P ${P1} -D ${D1} -d ${d1} -M ${M1} -m ${m1} -N ${N1} -p $p1} -b ${b1} -a ${a1} -L 2 -R /dev/null
    echo 0 > /proc/sched_pbs_actv
    echo "complete! "

    set repetitions=1

    foreach freq (${availablefreqs})
        echo "userspace" > ${freqdir}"scaling_governor"
        echo ${freq} > ${freqdir}"scaling_setspeed"

        echo "Frequency set to ${freq}KHz"

        set logdir="log/cpufreq_VIC_expt/"${freq}
        
	mkdir -p ${logdir}
        
        #loop through the repetitions of the experiment
        foreach i (`seq 1 1 ${repetitions}`)
            set logfile1=${logdir}"/"${i}${logfilesuffix1}
            
            echo 
            echo "running "$i"of ${repetitions}"
            
            bin/pbsAllocator -f -S -s ${sa} &
            sleep 1
            bin/sqrwavSRT -f -j ${j1} -P ${P1} -D ${D1} -d ${d1} -M ${M1} -m ${m1} -N ${N1} -A ${A1} -p ${p1} -b ${b1} -a ${a1} -L 2 -R ${logfile1} &
            bin/poll_pbs_actv -I

            echo "completed "$i"of ${repetitions}"
        end
    end

