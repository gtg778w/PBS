#! /bin/csh

    set LOGDIR="~/Desktop/log/"
    set PeSoRTADIR="../PeSORTA"

    echo "This complete script has an estimated duration of 9:14:0"

    #2:50:42.905760
    csh expt_scripts/run5_ffmpeg_dec.dsailors.mp3_mabank.csh ${LOGDIR} ${PeSoRTADIR}

    #2:50:42.905760   
    csh expt_scripts/run5_ffmpeg_dec.dsailors.mp3_mavslmsbank.csh ${LOGDIR} ${PeSoRTADIR}

    #1:15:4.132800
    csh expt_scripts/run5_ffmpeg_dec.mozart.mp3_mabank.csh ${LOGDIR} ${PeSoRTADIR}          

    #1:15:4.132800
    csh expt_scripts/run5_ffmpeg_dec.mozart.mp3_mavslmsbank.csh ${LOGDIR} ${PeSoRTADIR}

    #0:31:13.152000
    csh expt_scripts/run5_ffmpeg_dec.sintel.mp3_mabank.csh ${LOGDIR} ${PeSoRTADIR}

    #0:31:13.152000
    csh expt_scripts/run5_ffmpeg_dec.sintel.mp3_mavslmsbank.csh ${LOGDIR} ${PeSoRTADIR}

    #6:192:120
    #9:14:0
