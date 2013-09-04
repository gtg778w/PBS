#! /bin/csh -x
    
    #Set default values for optional input arguments
    set repetitions="1"
    set LOGDIR="./data/"
    set PeSoRTADIR="/media/Data/Research/expt_February2013/PeSoRTA"
    set BINDIR="/home/gtg778w/Desktop/bin/"
    set EXPTDIR="./expt_scripts/"
    
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
    else if ( $#argv == 5 ) then
        set repetitions=$argv[1]
        set LOGDIR=$argv[2]
        set PeSoRTADIR=$argv[3]
        set BINDIR=$argv[4]
        set EXPTDIR=$argv[5]
    else if ( $#argv > 5 ) then
        echo "Usage:"$argv[0]" [repetitions] [log directory] [PeSoRTA directory] [bin dir] [script dir]"
        exit 1
    endif
    
    echo "*******************************************************************************"
    echo "run_ffmpeg_dec.bigbuckbunnyfull.mov_mavslmsbank_freqconstmed_ns.csh"
    echo "*******************************************************************************"

    csh ${EXPTDIR}/run_ffmpeg_dec.bigbuckbunnyfull.mov_mavslmsbank_freqconstmed_ns.csh ${repetitions} ${LOGDIR} ${PeSoRTADIR} ${BINDIR}
    echo "*"
    echo "*"
    echo "*******************************************************************************"
    echo "*******************************************************************************"
    echo "*"
    echo "*"
    echo "*******************************************************************************"
    echo "run_ffmpeg_dec.bigbuckbunnyfull.mov_mavslmsbank_freqconstmed_VIC.csh"
    echo "*******************************************************************************"

    csh ${EXPTDIR}/run_ffmpeg_dec.bigbuckbunnyfull.mov_mavslmsbank_freqconstmed_VIC.csh ${repetitions} ${LOGDIR} ${PeSoRTADIR} ${BINDIR}
    echo "*"
    echo "*"
    echo "*******************************************************************************"
    echo "*******************************************************************************"
    echo "*"
    echo "*"
    echo "*******************************************************************************"
    echo "run_ffmpeg_dec.bigbuckbunnyfull.mov_mavslmsbank_freqcycle_ns.csh"
    echo "*******************************************************************************"

    csh ${EXPTDIR}/run_ffmpeg_dec.bigbuckbunnyfull.mov_mavslmsbank_freqcycle_ns.csh ${repetitions} ${LOGDIR} ${PeSoRTADIR} ${BINDIR}
    echo "*"
    echo "*"
    echo "*******************************************************************************"
    echo "*******************************************************************************"
    echo "*"
    echo "*"
    echo "*******************************************************************************"
    echo "run_ffmpeg_dec.bigbuckbunnyfull.mov_mavslmsbank_freqcycle_VIC.csh"
    echo "*******************************************************************************"
    
    csh ${EXPTDIR}/run_ffmpeg_dec.bigbuckbunnyfull.mov_mavslmsbank_freqcycle_VIC.csh ${repetitions} ${LOGDIR} ${PeSoRTADIR} ${BINDIR}

    echo "*"
    echo "*"
    echo "*******************************************************************************"
    echo "*******************************************************************************"
