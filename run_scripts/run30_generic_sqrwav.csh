#! /bin/csh -x

    # Three types of log files
    #   - a general log file to report on the experiments progress
    #   - a log file for the allocator task for each run
    #   - a log file for each SRT task

    # Format for SRT task logs file name:
    # <experiment number>_<task_number>_sqrwavSRT_<-j>_<-P>_<-D>_<-d>_<-M>_<-m>_<-N>_<-p>_<-l>.log

    # Format for the allocator task log file name:
    # <experiment number>_pbs_ul_allocator_<-a>_<-s>.log

    set aa="2.0"
    set sa="12100"
    set logfilesuffixa="_pbs_ul_allocator_"${aa}"_"${sa}".log"

    set j1="1800"
    set P1="1800"
    set D1="0.44445"
    set d1="1600"
    set M1="8424000"
    set m1="4212000"
    set N1="0.2"
    set p1="60000"
    set b1="15000"
    set l1="20"
    set logfilesuffix1="_1_sqrwavSRT_"${j1}"_"${P1}"_"${D1}"_"${d1}"_"${M1}"_"${m1}"_"${N1}"_"${p1}"_"${b1}"_"${l1}".log"

    set j2="2700"
    set P2="2700"
    set D2="0.33334"
    set d2="1200"
    set M2="5616000"
    set m2="2808000"
    set N2="0.2"
    set p2="40000"
    set b2="10000"
    set l2="20"
    set logfilesuffix2="_2_sqrwavSRT_"${j2}"_"${P2}"_"${D2}"_"${d2}"_"${M2}"_"${m2}"_"${N2}"_"${p2}"_"${b2}"_"${l2}".log"

    echo "Training run: ... "
    echo 0 > /proc/sched_pbs_actv
    bin/allocator -f -a ${aa} -s 0 -S &
    sleep 1
    bin/sqrwavSRT -f -j 16 -P 1800 -D 0.44445 -d 1600 -M 21060000 -m 4212000 -N 0.2 -p 60000 -b 7000 -l 20 -L /dev/null
    echo 0 > /proc/sched_pbs_actv
    echo "complete! "

    set repetitions=1

    #loop through the repetitions of the experiment
    foreach i (`seq 1 1 ${repetitions}`)
        set logfilea="log/"${i}${logfilesuffixa}
        set logfile1="log/"${i}${logfilesuffix1}
        set logfile2="log/"${i}${logfilesuffix2}

        echo 
        echo "running "$i"of 30"

        bin/allocator -f -a ${aa} -s ${sa} > ${logfilea} &
        sleep 1
        bin/sqrwavSRT -f -j ${j1} -P ${P1} -D ${D1} -d ${d1} -M ${M1} -m ${m1} -N ${N1} -p ${p1} -b ${b1} -l ${l1} -L ${logfile1} &
        bin/sqrwavSRT -f -j ${j2} -P ${P2} -D ${D2} -d ${d2} -M ${M2} -m ${m2} -N ${N2} -p ${p2} -b ${b2} -l ${l2} -L ${logfile2} &
        bin/poll_pbs_actv

        echo "completed "$i"of 30"
    end

