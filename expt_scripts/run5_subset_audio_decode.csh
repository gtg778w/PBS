#! /bin/csh

    set LOGDIR="~/Desktop/log/"
    set PeSoRTADIR=""

    echo "This complete script has an estimated duration of 132:13:10"

    csh run5_ffmpeg_dec.dsailors.mp3_mabank.csh  ${LOGDIR} ${PeSoRTADIR}        #2:50:42.905760
    csh run5_ffmpeg_dec.dsailors.mp3_mavslmsbank.csh  ${LOGDIR} ${PeSoRTADIR}   #2:50:42.905760

    csh run5_ffmpeg_dec.mozart.mp3_mabank.csh  ${LOGDIR} ${PeSoRTADIR}          #1:15:4.132800
    csh run5_ffmpeg_dec.mozart.mp3_mavslmsbank.csh  ${LOGDIR} ${PeSoRTADIR}     #1:15:4.132800

    csh run5_ffmpeg_dec.sintel.mp3_mabank.csh  ${LOGDIR} ${PeSoRTADIR}          #0:31:13.152000
    csh run5_ffmpeg_dec.sintel.mp3_mavslmsbank.csh  ${LOGDIR} ${PeSoRTADIR}     #0:31:13.152000

