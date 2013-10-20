#! /bin/csh -x

    #Set default values for optional input arguments
    set repetitions="1"
    set LOGDIR="timing_data/"
    set SCRIPTDIR="timing_scripts"
    set PeSoRTADIR="/home/gtg778w/Desktop/expt_February2013/PeSoRTA"
    set BINDIR="/home/gtg778w/Desktop/expt_February2013/PBS/bin"
    
    #Process input arguments for repetitions BINDIR LOGDIR and PeSoRTADIR
    if ( $#argv == 1 ) then
        set repetitions=$argv[1]
    else if ( $#argv == 2 ) then
        set repetitions=$argv[1]
        set LOGDIR=$argv[2]
    else if ( $#argv == 3 ) then
        set repetitions=$argv[1]
        set LOGDIR=$argv[2]
        set SCRIPTDIR=$argv[3]
    else if ( $#argv == 4 ) then
        set repetitions=$argv[1]
        set LOGDIR=$argv[2]
        set SCRIPTDIR=$argv[3]
        set PeSoRTADIR=$argv[4]
    else if ( $#argv == 5 ) then
        set repetitions=$argv[1]
        set LOGDIR=$argv[2]
        set SCRIPTDIR=$argv[3]
        set PeSoRTADIR=$argv[4]
        set BINDIR=$argv[5]
    else if ( $#argv > 5 ) then
        echo "Usage:"$argv[0]" [repetitions [log directory [PeSoRTA directory [bin dir]]]]"
        exit 1
    endif

    csh ${SCRIPTDIR}/timing.base.csh ${repetitions} ${LOGDIR} ${SCRIPTDIR} ${PeSoRTADIR} ${BINDIR}
    csh ${SCRIPTDIR}/timing.sqrwav.csh ${repetitions} ${LOGDIR} ${SCRIPTDIR} ${PeSoRTADIR} ${BINDIR}
    csh ${SCRIPTDIR}/timing.ffmpeg.audio.csh ${repetitions} ${LOGDIR} ${SCRIPTDIR} ${PeSoRTADIR} ${BINDIR}
    csh ${SCRIPTDIR}/timing.ffmpeg.speech.csh ${repetitions} ${LOGDIR} ${SCRIPTDIR} ${PeSoRTADIR} ${BINDIR}
    csh ${SCRIPTDIR}/timing.ffmpeg.video.csh ${repetitions} ${LOGDIR} ${SCRIPTDIR} ${PeSoRTADIR} ${BINDIR}
    csh ${SCRIPTDIR}/timing.ffmpeg.long_video.csh ${repetitions} ${LOGDIR} ${SCRIPTDIR} ${PeSoRTADIR} ${BINDIR}
    
