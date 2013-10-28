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

    csh ${SCRIPTDIR}/pred.base.csh ${predictor} ${PREDDIR} ${TIMEDIR} ${SCRIPTDIR} ${BINDIR}
    csh ${SCRIPTDIR}/pred.sqrwav.csh ${predictor} ${PREDDIR} ${TIMEDIR} ${SCRIPTDIR} ${BINDIR}
    csh ${SCRIPTDIR}/pred.ffmpeg.audio.csh ${predictor} ${PREDDIR} ${TIMEDIR} ${SCRIPTDIR} ${BINDIR}
    csh ${SCRIPTDIR}/pred.ffmpeg.speech.csh ${predictor} ${PREDDIR} ${TIMEDIR} ${SCRIPTDIR} ${BINDIR}
    csh ${SCRIPTDIR}/pred.ffmpeg.video.csh ${predictor} ${PREDDIR} ${TIMEDIR} ${SCRIPTDIR} ${BINDIR}
    csh ${SCRIPTDIR}/pred.ffmpeg.long_video.csh ${predictor} ${PREDDIR} ${TIMEDIR} ${SCRIPTDIR} ${BINDIR}
    
