#! /bin/csh -x

    #Set default values for optional input arguments
    set predictor="ma"
    set PREDDIR="prdtst_data/"
    set TIMEDIR="timing_data/Core2_extreme/"
    set SCRIPTDIR="prdtst_scripts"
    set BINDIR="/home/gtg778w/Desktop/bin/"
    
    #Process input arguments for predictor PREDDIR TIMEDIR SCRIPTDIR and BINDIR
    if ( $#argv == 1 ) then
        set predictor=$argv[1]
    else if ( $#argv == 2 ) then
        set predictor=$argv[1]
        set PREDDIR=$argv[2]
    else if ( $#argv == 3 ) then
        set predictor=$argv[1]
        set PREDDIR=$argv[2]
        set TIMEDIR=$argv[3]
    else if ( $#argv == 4 ) then
        set predictor=$argv[1]
        set PREDDIR=$argv[2]
        set TIMEDIR=$argv[3]
        set SCRIPTDIR=$argv[4]
    else if ( $#argv == 5 ) then
        set predictor=$argv[1]
        set PREDDIR=$argv[2]
        set TIMEDIR=$argv[3]
        set SCRIPTDIR=$argv[4]
        set BINDIR=$argv[5]
    else if ( $#argv > 5 ) then
        echo "Usage:"$argv[0]" [predictor [predictor data dir [timig data dir [script dir [bin dir]]]]]"
        exit 1
    endif

    #The name of the application being used for the tests
    set application="ffmpeg"
    set configs=("dec.deadline.flv" "dec.deadline.mp4" "dec.deadline.webm" "dec.factory.flv" "dec.factory.mp4" "dec.factory.webm" "dec.highway.flv" "dec.highway.mp4" "dec.highway.webm" "dec.sintel.flv" "dec.sintel.mp4" "dec.sintel.webm" "enc.bridge_close.y4m.flv" "enc.bridge_close.y4m.mp4" "enc.bridge_close.y4m.webm" "enc.deadline.y4m.flv" "enc.deadline.y4m.mp4" "enc.deadline.y4m.webm" "enc.highway.y4m.flv" "enc.highway.y4m.mp4" "enc.highway.y4m.webm" "enc.mad900.y4m.flv" "enc.mad900.y4m.mp4" "enc.mad900.y4m.webm")

    foreach config (${configs})
        csh ${SCRIPTDIR}/pred.generic.csh ${predictor} ${PREDDIR} ${config} ${application} ${TIMEDIR} ${BINDIR}
    end

