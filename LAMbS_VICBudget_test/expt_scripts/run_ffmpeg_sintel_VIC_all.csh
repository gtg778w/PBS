#! /bin/csh -x
    
    set usagestring = " [<repetitions> [<frequency_cycle half-period> [<log directory> [<script directory> [<PeSoRTA directory> [<bin dir>]]]]]]"
    
    #Set default values for optional input arguments
    set repetitions="1"
    
    #arguments for the cpufreq_oscillate utility application
    set FC_DURATION="1.0"
        
    set ROOT_LOGDIR="data/ffmpeg/sintel/"
    set SCRIPTDIR="expt_scripts"
    set PeSoRTADIR="/media/Data/Research/expt_February2013/PeSoRTA"
    set BINDIR="/home/gtg778w/Desktop/bin"
    
    #Process input arguments for repetitions BINDIR LOGDIR and PeSoRTADIR
    if ( $#argv > 0 ) then
        set repetitions=$argv[1]
        if ( $#argv > 1 ) then
            set FC_DURATION=$argv[2]
            if ( $#argv > 2 ) then
                set LOGDIR=$argv[3]
                if ( $#argv > 3) then
                    set SCRIPTDIR=$argv[4]
                    if ( $#argv > 4) then
                        set PeSoRTADIR=$argv[5]
                        if ( $#argv > 5) then
                            set BINDIR=$argv[6]
                            if ( $#argv > 6) then
                                echo "Usage:"$argv[0]$usagestring
                                exit 1
                            endif
                        endif
                    endif
                endif
            endif
        endif
    endif

    echo "*******************************************************************************"
    echo "run the ffmpeg workload at maximum frequency with VIC-based budget"
    echo "*******************************************************************************"
    
    set LOGDIR="${ROOT_LOGDIR}/max_freq/VIC/"
    csh ${SCRIPTDIR}/core_ffmpeg_sintel_VIC.csh ${repetitions} ${FC_DURATION} "0.9" "0.9" ${LOGDIR} ${SCRIPTDIR} ${PeSoRTADIR} ${BINDIR}

    echo "*"
    echo "*"
    echo "*******************************************************************************"
    echo "*******************************************************************************"
    echo "*"
    echo "*"
    echo "*******************************************************************************"
    echo "run the ffmpeg workload at minimum frequency with VIC-based budget"
    echo "*******************************************************************************"
    
    set LOGDIR="${ROOT_LOGDIR}/min_freq/VIC/"
    csh ${SCRIPTDIR}/core_ffmpeg_sintel_VIC.csh ${repetitions} ${FC_DURATION} "0.0" "0.0"  ${LOGDIR} ${SCRIPTDIR} ${PeSoRTADIR} ${BINDIR}
    echo "*"
    echo "*"
    echo "*******************************************************************************"
    echo "*******************************************************************************"
    echo "*"
    echo "*"
    echo "*******************************************************************************"
    echo "run the ffmpeg workload at oscillating frequency with VIC-based budget"
    echo "*******************************************************************************"
    
    set LOGDIR="${ROOT_LOGDIR}/oscil_freq/VIC/"
    csh ${SCRIPTDIR}/core_ffmpeg_sintel_VIC.csh ${repetitions} ${FC_DURATION} "0.0" "0.9" ${LOGDIR} ${SCRIPTDIR} ${PeSoRTADIR} ${BINDIR}
    echo "*"
    echo "*"
    echo "*******************************************************************************"
    echo "*******************************************************************************"

