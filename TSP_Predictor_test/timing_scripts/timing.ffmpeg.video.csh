#! /bin/csh -x

    #Set default values for optional input arguments
    set repetitions="1"
    set LOGDIR="timing_data/"
    set SCRIPTDIR="timing_scripts"
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

    #The name of the application being used for the tests
    set application="ffmpeg"
    set configs=("dec.deadline.flv" "dec.deadline.mp4" "dec.deadline.webm" "dec.factory.flv" "dec.factory.mp4" "dec.factory.webm" "dec.highway.flv" "dec.highway.mp4" "dec.highway.webm" "dec.sintel.flv" "dec.sintel.mp4" "dec.sintel.webm" "enc.bridge_close.y4m.flv" "enc.bridge_close.y4m.mp4" "enc.bridge_close.y4m.webm" "enc.deadline.y4m.flv" "enc.deadline.y4m.mp4" "enc.deadline.y4m.webm" "enc.highway.y4m.flv" "enc.highway.y4m.mp4" "enc.highway.y4m.webm" "enc.mad900.y4m.flv" "enc.mad900.y4m.mp4" "enc.mad900.y4m.webm")

    foreach config (${configs})
        csh ${SCRIPTDIR}/timing.generic.csh ${repetitions} ${config} ${application} ${LOGDIR} ${PeSoRTADIR} ${BINDIR}
    end

