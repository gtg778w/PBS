#! /bin/csh -x

    #Set default values for optional input arguments
    set repetitions="1"
    set LOGDIR="log"
    set PeSoRTADIR="../PeSoRTA"
    set BINDIR="bin/"
    set SCRIPTSDIR="LAMbS_VIC_expt_scripts/"

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

    echo "******************************high*****************************************"
    echo
    csh ${SCRIPTSDIR}/run_ffmpeg_dec.bigbuckbunnyfull.mov_mavslmsbank_freqconsthigh.csh \
        ${repetitions} ${LOGDIR} ${PeSoRTADIR} ${BINDIR}
    echo    
    echo "***************************************************************************"
    echo
    echo
    echo
    echo
    echo "******************************medium***************************************"
    echo
    csh ${SCRIPTSDIR}/run_ffmpeg_dec.bigbuckbunnyfull.mov_mavslmsbank_freqconstmed.csh \
        ${repetitions} ${LOGDIR} ${PeSoRTADIR} ${BINDIR}
    echo    
    echo "***************************************************************************"
    echo
    echo
    echo
    echo
    echo "*********************************low***************************************"
    echo
    csh ${SCRIPTSDIR}/run_ffmpeg_dec.bigbuckbunnyfull.mov_mavslmsbank_freqconstlow.csh \
        ${repetitions} ${LOGDIR} ${PeSoRTADIR} ${BINDIR}
    echo    
    echo "***************************************************************************"
    echo
    echo
    echo
    echo
    echo "*******************************cycle***************************************"
    echo
    csh ${SCRIPTSDIR}/run_ffmpeg_dec.bigbuckbunnyfull.mov_mavslmsbank_freqcycle.csh \
        ${repetitions} ${LOGDIR} ${PeSoRTADIR} ${BINDIR}
    echo    
    echo "***************************************************************************"

