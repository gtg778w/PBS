#! /bin/csh -x

    #Set default values for optional input arguments
    set repetitions="1"
    set LOGDIR="data/"
    set PeSoRTADIR="/media/Data/Research/expt_February2013/PeSoRTA"
    set BINDIR="/home/gtg778w/Desktop/bin/"

    #Process input arguments for repetitions BINDIR LOGDIR and PeSoRTADIR
    if ( $#argv == 1 ) then
        set repetitions=$argv[1]
    else if ( $#argv == 2 ) then
        set repetitions=$argv[1]
        set LOGDIR=$argv[2]
    else if ( $#argv == 3 ) then
        set repetitions=$argv[1]
        set LOGDIR=$argv[2]
        set PeSoRTADIR=$argv[3]
    else if ( $#argv == 4 ) then
        set repetitions=$argv[1]
        set LOGDIR=$argv[2]
        set PeSoRTADIR=$argv[3]
        set BINDIR=$argv[4]
    else if ( $#argv > 4 ) then
        echo "Usage:"$argv[0]" [repetitions] [log directory] [PeSoRTA directory] [bin dir]"
        exit 1
    endif
    
    #The period of the allocator task
    set Ta="10000000"
    #The budget assigned to the allocator task over a reservation period
    set Qa="1000000"
    set sa="12100"
    
    #The name of the configuration
    set APPNAME="sqrwav"
    set CONFIGNAME="constant_med"
    set LOCALLOGDIR=${LOGDIR}"/"${APPNAME}"/"${CONFIGNAME}"/freqcycle/ns"
    mkdir -p ${LOCALLOGDIR}
    
    #The name of the configuration file for the PeSoRTA workload
    set W1="config/"${CONFIGNAME}".config"
    #The root directory for the PeSoRTA workload
    set D1=${PeSoRTADIR}"/"${APPNAME}
    #The maximum number of jobs to run from the PeSoRTA workload
    set J1="2100"
    #The predictor to be used for budget allocation by the SRT application
    set A1="mabank"
    #The task period (in ns) of the SRT application
    set p1="40000000"
    #The estimated mean execution time of the SRT application
    set c1="20000000"
    #Alpha values of the workload
    set alpha_array=("1.555555")
    
    @ duration_secs = 121 * ${repetitions}
    set oscillate_duration=121
    
    #Loop over the values of alpha
    foreach alpha ($alpha_array)
        
        #The suffix of the log file to store the data collected by the SRT application
        set SRT_logfilesuffix="_SRT_PeSoRTA_"${APPNAME}"_"${CONFIGNAME}"_"${J1}"_"${A1}"_"${p1}"_"${c1}"_"${alpha}".log"        
        
        #Loop over the repetitions
        foreach rep (`seq 1 1 ${repetitions}`)
            
            #Display progress and estimated duration
            echo "Experiment: ${LOCALLOGDIR}"
            echo "Approximate total duration of experiment: ${duration_secs}"
            echo "Repetition "${rep}" of ${repetitions}"
                
            #Names of LOG files
            set SRT_logfile=${LOCALLOGDIR}"/"${rep}"_freqcycle"${SRT_logfilesuffix}
            
            #Start the frequency cycling program
            ${BINDIR}/cpufreq_oscillate ${oscillate_duration} 1 0.0 1.0 &
            #Start the allocator
            ${BINDIR}/pbsAllocator -f -s ${sa} -P ${Ta} -B ${Qa} &
            #Wait for half a second to let the allocator run a couple of scheduling periods
            sleep 0.5
            #Run the SRT task
            ${BINDIR}/${APPNAME}_pbsSRT -f -W ${W1} -D ${D1} -J ${J1} -A ${A1} -p ${p1} -c ${c1} -a ${alpha} -L 2 -R ${SRT_logfile} &
            #Wait for the allocator task to finish
            ${BINDIR}/poll_pbs_actv -I
            
        end
    end

