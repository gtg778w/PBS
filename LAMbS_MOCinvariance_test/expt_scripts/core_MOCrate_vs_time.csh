#! /bin/csh -x
    
    set usagestring =  "[<MOC> [<Number of samples> [<Sampling period> [<log directory> [<script directory> [<PeSoRTA directory> [<bin dir>]]]]]]]"
    
    #Set default values for optional input arguments
    set MOC="userinst"
    set SAMPLE_COUNT="24000"
    set SAMPLE_PERIOD="2500000"
    set LOGDIR_ROOT="data2/"
    set SCRIPTDIR="expt_scripts"
    set PeSoRTADIR="/media/Data/Research/expt_February2013/PeSoRTA"
    set BINDIR="/home/gtg778w/Desktop/bin"
    
    #Process input arguments for repetitions BINDIR LOGDIR and PeSoRTADIR
    if ( $#argv > 0 ) then
        set MOC=$argv[1]
        if ( $#argv > 1 ) then
            set SAMPLE_COUNT=$argv[2]
            if ( $#argv > 2 ) then
                set SAMPLE_PERIOD=$argv[3]
                if ( $#argv > 3) then
                    set LOGDIR_ROOT=$argv[4]
                    if ( $#argv > 4) then
                        set SCRIPTDIR=$argv[5]
                        if ( $#argv > 5) then
                            set PeSoRTADIR=$argv[6]
                            if ( $#argv > 6) then
                                set BINDIR=$argv[7]
                                if ( $#argv > 7) then
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
    source ${SCRIPTDIR}/cpufreq_inc.csh

    set APPNAME="ffmpeg"
    set CONFIGNAME_ARRAY=( "dec.sintelfull.720p.mkv" )

    set freq=$availablefreqs[2]

    #Loop over each configuration
    foreach CONFIGNAME ( $CONFIGNAME_ARRAY )
        echo "CPU frequency = ${freq}"
        echo 
        #Set the CPU speed                
        echo ${freq} > ${freqdir}/scaling_setspeed
        
        #Create the log directory if not already there.
        set LOGDIR="${LOGDIR_ROOT}/${APPNAME}/${CONFIGNAME}/${MOC}/"
        mkdir -p ${LOGDIR}

        set APPROOTDIR=${PeSoRTADIR}"/"${APPNAME}
        set BINNAME=${APPNAME}_mocsample_timed
        set CONFIGFILE="config/"${CONFIGNAME}".config"
        set LOGFILENAME="${LOGDIR}/${MOC}_${CONFIGNAME}_${APPNAME}.csv"
        
        #Run a single repetition of the workload
        ${BINDIR}/${BINNAME} -r -p 0.0 -R ${APPROOTDIR} -C ${CONFIGFILE} -L ${LOGFILENAME} -M ${MOC} -P ${SAMPLE_PERIOD} -N ${SAMPLE_COUNT}
    end

