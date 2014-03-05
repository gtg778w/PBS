#! /bin/csh -x
    
    set usagestring = " [<repetitions> [<frequency_cycle half-period> [<frequency_cycle minimum speed> [<frequency_cycle maximum speed> [<log directory> [<script directory> [<PeSoRTA directory> [<bin dir>]]]]]]]]"
    
    #Set default values for optional input arguments
    set repetitions="1"
    
    #arguments for the cpufreq_oscillate utility application
    set FC_DURATION="2.0"
    set CPU_MINSPEED="0.0"
    set CPU_MAXSPEED="0.9"
    
    set LOGDIR="data/"
    set SCRIPTDIR="expt_scripts"
    set PeSoRTADIR="/media/Data/Research/expt_February2013/PeSoRTA"
    set BINDIR="/home/gtg778w/Desktop/bin"
    
    #Process input arguments for repetitions BINDIR LOGDIR and PeSoRTADIR
    if ( $#argv > 0 ) then
        set repetitions=$argv[1]
        if ( $#argv > 1 ) then
            set FC_DURATION=$argv[2]
            if ( $#argv > 2 ) then
                set CPU_MINSPEED=$argv[3]
                if ( $#argv > 3) then
                    set CPU_MAXSPEED=$argv[4]
                    if ( $#argv > 4) then
                        set LOGDIR=$argv[5]
                        if ( $#argv > 5) then
                            set SCRIPTDIR=$argv[6]
                            if ( $#argv > 6) then
                                set PeSoRTADIR=$argv[7]
                                if ( $#argv > 7) then
                                    set BINDIR=$argv[8]
                                    if ( $#argv > 8) then
                                        echo "Usage:"$argv[0]$usagestring
                                        exit 1
                                    endif
                                endif
                            endif
                        endif
                    endif
                endif
            endif
        endif
    endif

    #The period of the allocator task
    set Ta="10000000"
    #The budget assigned to the allocator task over a reservation period
    set Qa="1000000"
    set sa="1600"
    set Ll="0"
    #The budget type should be "" for ns and "-VIC" for VIC.
    set BUDGET_TYPE="-VIC"
    
    #The name of the configuration
    set APPNAME="membound"
    set APPROOTDIR=${PeSoRTADIR}"/"${APPNAME}
    set CONFIGNAME="thrash_shortjob"
    set CONFIGFILE="config/"${CONFIGNAME}".config"
    
    #The name of the configuration file for the PeSoRTA workload
    set W1=${CONFIGFILE}
    #The root directory for the PeSoRTA workload
    set D1=${APPROOTDIR}
    #The maximum number of jobs to run from the PeSoRTA workload
    set J1="400"
    #The predictor to be used for budget allocation by the SRT application
    set A1="mavslmsbank"
    #The task period (in ns) of the SRT application
    set p1="20000000"
    #The estimated mean execution time of the SRT application
    set c1="3600000"
    #Alpha values of the workload
    set alpha_array=("1.25")
    
    @ duration_secs = ((((${Ta} / 1000) * ${sa}) / 1000) * ${repetitions}) / 1000
    echo "duration_secs=${duration_secs}"
    @ oscillate_duration = ((((${Ta} / 1000) * ${sa}) / 1000) * ${repetitions}) / 1000 - 2
    echo "oscillate_duration=${oscillate_duration}"

    #Loop over the values of alpha
    foreach alpha ($alpha_array)
        
        set LOCALLOGDIR="${LOGDIR}/${alpha}"
        mkdir -p ${LOCALLOGDIR}
        
        #The suffix of the log file to store the data collected by the SRT application
        set SRT_logfilesuffix="_SRT_PeSoRTA_"${APPNAME}"_"${CONFIGNAME}"_"${J1}"_"${A1}"_"${p1}"_"${c1}"_"${alpha}".log"        
        
        #Loop over the repetitions
        foreach rep (`seq 1 1 ${repetitions}`)
            
            #Display progress and estimated duration
            echo "***********************************************************************"
            echo "Log directory: ${LOGDIR}"
            echo 
            echo "alpha:         ${alpha} (0.1: 0.1: 3.0)"
            echo
            echo "Repetition:    ${rep} of ${repetitions}"
            echo 
            echo "Duration:      ${duration_secs}"
            echo 
            echo "***********************************************************************"
            
            #Names of LOG files
            set SRT_logfile=${LOCALLOGDIR}"/"${rep}${SRT_logfilesuffix}
            
            #Start the frequency cycling program
            ${BINDIR}/cpufreq_oscillate ${oscillate_duration}, ${FC_DURATION}, ${CPU_MINSPEED}, ${CPU_MAXSPEED} &
            #Start the allocator
            ${BINDIR}/Allocator -f -s ${sa} -P ${Ta} -B ${Qa} ${BUDGET_TYPE} &
            #Wait for half a second to let the allocator run a couple of scheduling periods
            sleep 0.5
            #Run the SRT task
            ${BINDIR}/${APPNAME}_SRT -f -W ${W1} -D ${D1} -J ${J1} -A ${A1} -p ${p1} -c ${c1} -a ${alpha} -L 2 -R ${SRT_logfile} &
            #Wait for the allocator task to finish
            ${BINDIR}/poll_pbs_actv -I
            
        end
    end
    
    #slow the CPU to the slowest speed
    echo 0 > /sys/devices/system/cpu/cpu0/cpufreq/scaling_setspeed
    
