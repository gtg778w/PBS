#! /bin/csh -x
    
    #Set default values for optional input arguments
    set repetitions="1"
    set predictor="ma"
    set OUTDIR="data/"
    set BINDIR="/home/gtg778w/Desktop/bin/"
    set PeSoRTADIR="/media/Data/Research/expt_February2013/PeSoRTA"    
    
    #Process input arguments for predictor PREDDIR config application TIMEDIR and BINDIR
    if ( $#argv == 1 ) then
        set repetitions=$argv[1]
    else if ( $#argv == 2 ) then
        set repetitions=$argv[1]
        set predictor=$argv[2]
    else if ( $#argv == 3 ) then
        set repetitions=$argv[1]
        set predictor=$argv[2]
        set OUTDIR=$argv[3]
    else if ( $#argv == 4 ) then
        set repetitions=$argv[1]
        set predictor=$argv[2]
        set OUTDIR=$argv[3]
        set BINDIR=$argv[4]
    else if ( $#argv == 5 ) then
        set repetitions=$argv[1]
        set predictor=$argv[2]
        set OUTDIR=$argv[3]
        set BINDIR=$argv[4]
        set PeSoRTADIR=$argv[5]
    else if ( $#argv > 6 ) then
        echo "Usage:"$argv[0]" [repetitions [predictor [OUTDIR [BINDIR [PeSoRTADIR]]]]]"
        exit 1
    endif
    
    #The period of the allocator task
    set Ta="11111111"
    #The budget assigned to the allocator task over a reservation period
    set Qa="1000000"
    set sa="7199"
    
    #The name of the configuration
    set APPNAME="ffmpeg"
    set CONFIGNAME="enc.highway.y4m.mp4"
    
    #The name of the configuration file for the PeSoRTA workload
    set W1="config/"${CONFIGNAME}".config"
    #The root directory for the PeSoRTA workload
    set D1=${PeSoRTADIR}"/"${APPNAME}
    #The maximum number of jobs to run from the PeSoRTA workload
    set J1="2000"
    #The predictor to be used for budget allocation by the SRT application
    set A1=${predictor}
    #The task period (in ns) of the SRT application
    set p1="33333333"
    #The estimated mean execution time of the SRT application
    set c1="15036753"
    #Alpha values of the workload
    set alpha_array=()
    
    #Loop over the values of alpha
    foreach alpha ($alpha_array)
        
        set LOCALOUTDIR=${OUTDIR}"/"${APPNAME}"/"${CONFIGNAME}"/"${predictor}"/"${alpha}
        mkdir -p ${LOCALOUTDIR}
        
        #The suffix of the log file to store the data collected by the SRT application
        set SRT_logfilesuffix="_SRT_PeSoRTA_"${APPNAME}"_"${CONFIGNAME}"_"${J1}"_"${A1}"_"${p1}"_"${c1}"_"${alpha}".log"        
        set ALC_logfilesuffix="_allocator_PeSoRTA_"${APPNAME}"_"${Ta}"_"${Qa}"_"${sa}".log"
        
        #Loop over the repetitions
        foreach rep (`seq 1 1 ${repetitions}`)
            
            #Display progress and estimated duration
            echo "Approximate total duration of experiment: 0:59:59.999964"
            
            #Names of LOG files
            set SRT_logfile=${LOCALOUTDIR}"/"${rep}${SRT_logfilesuffix}
            
            #Start the allocator            
            ${BINDIR}/Allocator -f -s ${sa} -P ${Ta} -B ${Qa} -L 0&
            #Wait for half a second to let the allocator run a couple of scheduling periods
            sleep 0.5
            #Run the SRT task
            ${BINDIR}/${APPNAME}_SRT -f -W ${W1} -D ${D1} -J ${J1} -A ${A1} -p ${p1} -c ${c1} -a ${alpha} -L 2 -R ${SRT_logfile} &
            #Wait for the allocator task to finish
            ${BINDIR}/poll_pbs_actv
        end
    end

