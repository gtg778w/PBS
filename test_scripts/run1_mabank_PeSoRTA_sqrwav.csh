#! /bin/csh -x

    #The period of the allocator task
    set Ta="10000000"
    #The budget assigned to the allocator task over a reservation period
    set Qa="1000000"
    set sa="12100"
    #The suffix of the log file to store the data collected by the allocator task
    set logfilesuffixa="_allocator_PeSoRTA_sqrwav_"${Ta}"_"${Qa}"_"${sa}".log"

    #The name of the configuration file for the PeSoRTA workload
    set W1="config/example.config"
    #The root directory for the PeSoRTA workload
    set D1="/media/Data/Research/expt_February2013/PeSoRTA/sqrwav/"
    #The maximum number of jobs to run from the PeSoRTA workload
    set J1="7500"
    #The predictor to be used for budget allocation by the SRT application
    set A1="mabank"
    #The task period (in ns) of the SRT application
    set p1="10000000"
    #The estimated mean execution time of the SRT application
    set c1="5000000"
    #The alpha parameter
    set a1="0.75"
    #The suffix of the log file to store the data collected by the SRT application
    set logfilesuffix1="_1_SRT_PeSoRTA_sqrwav_"${J1}"_"${A1}"_"${p1}"_"${c1}"_"${a1}".log"

    set logfilea="log/1"${logfilesuffixa}
    set logfile1="log/1"${logfilesuffix1}

    bin/pbsAllocator -f -s ${sa} -P ${Ta} -B ${Qa} > ${logfilea} &
    sleep 1
    bin/sqrwav_pbsSRT -f -W ${W1} -D ${D1} -J ${J1} -A ${A1} -p ${p1} -c ${c1} -a ${a1} -L 2 -R ${logfile1} &
    bin/poll_pbs_actv

