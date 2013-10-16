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
    set configs=("dec.arthur.amr" "dec.arthur.opus" "dec.arthur.spx" "dec.cc.amr" "dec.cc.opus" "dec.cc.spx" "dec.vf18.amr" "dec.vf18.opus" "dec.vf18.spx" "enc.arthur.wav.amr" "enc.arthur.wav.opus" "enc.arthur.wav.spx" "enc.cc.wav.amr" "enc.cc.wav.opus" "enc.cc.wav.spx" "enc.vf18.wav.amr" "enc.vf18.wav.opus" "enc.vf18.wav.spx")

    foreach config (${configs})
        csh ${SCRIPTDIR}/timing.generic.csh ${repetitions} ${config} ${application} ${LOGDIR} ${PeSoRTADIR} ${BINDIR}
    end

