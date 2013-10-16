#! /bin/csh -x

    #The period of the allocator task
    set Ta="33333333"
    #The budget assigned to the allocator task over a reservation period
    set Qa="1000000"
    set sa="5000"
    #The suffix of the log file to store the data collected by the allocator task
    set logfilesuffixa="_allocator_PeSoRTA_ffmpeg_"${Ta}"_"${Qa}"_"${sa}".log"

    #The name of the configuration file for the PeSoRTA workload
    set W1="config/dec.deadline.mp4.config"
    #The root directory for the PeSoRTA workload
    set D1="/media/Data/Research/expt_February2013/PeSoRTA/ffmpeg/"
    #The maximum number of jobs to run from the PeSoRTA workload
    set J1="10000"
    #The predictor to be used for budget allocation by the SRT application
    set A1="mabank"
    #The task period (in ns) of the SRT application
    set p1="33333333"
    #The estimated mean execution time of the SRT application
    set c1="20000000"
    #The alpha parameter
    set a1="0.75"
    #The suffix of the log file to store the data collected by the SRT application
    set logfilesuffix1="_1_SRT_PeSoRTA_ffmpeg_"${J1}"_"${A1}"_"${p1}"_"${c1}"_"${a1}".log"

    set logfilea="log/1"${logfilesuffixa}
    set logfile1="log/1"${logfilesuffix1}

    bin/pbsAllocator -f -s ${sa} -P ${Ta} -B ${Qa} > ${logfilea} &
    sleep 1
    bin/ffmpeg_pbsSRT -f -W ${W1} -D ${D1} -J ${J1} -A ${A1} -p ${p1} -c ${c1} -a ${a1} -L 2 -R ${logfile1} &
    bin/poll_pbs_actv -I

