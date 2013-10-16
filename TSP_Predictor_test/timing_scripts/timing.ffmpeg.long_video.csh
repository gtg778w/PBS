#! /bin/csh -x

    #Set default values for optional input arguments
    set repetitions="1"
    set LOGDIR="timing_data/"
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
        set PeSoRTADIR=$argv[3]
    else if ( $#argv == 4 ) then
        set repetitions=$argv[1]
        set LOGDIR=$argv[2]
        set PeSoRTADIR=$argv[3]
        set BINDIR=$argv[4]
    else if ( $#argv > 4 ) then
        echo "Usage:"$argv[0]" [repetitions [log directory [PeSoRTA directory [bin dir]]]]"
        exit 1
    endif

    #The name of the application being used for the tests
    set application="ffmpeg"
    set configs=("dec.audio.bigbuckbunnyfull.mov" "dec.bigbuckbunnyfull.mov" "dec.audio.sintelfull.mkv" "dec.sintelfull.mkv")

    foreach config (${configs})
        csh timing_generic.csh ${repetitions} ${config} ${application} ${LOGDIR} ${PeSoRTADIR} ${BINDIR}
    end

