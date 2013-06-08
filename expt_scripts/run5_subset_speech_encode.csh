#! /bin/csh

    set LOGDIR="~/Desktop/log/"
    set PeSoRTADIR=""

    echo "This complete script has an estimated duration of 132:13:10"

    csh run5_ffmpeg_enc.arthur.wav.amr_mabank.csh  ${LOGDIR} ${PeSoRTADIR}      #1:4:32.255939
    csh run5_ffmpeg_enc.arthur.wav.amr_mavslmsbank.csh  ${LOGDIR} ${PeSoRTADIR} #1:4:32.255939
    csh run5_ffmpeg_enc.arthur.wav.mp3_mabank.csh  ${LOGDIR} ${PeSoRTADIR}      #1:4:32.255939
    csh run5_ffmpeg_enc.arthur.wav.mp3_mavslmsbank.csh  ${LOGDIR} ${PeSoRTADIR} #1:4:32.255939
    csh run5_ffmpeg_enc.arthur.wav.opus_mabank.csh  ${LOGDIR} ${PeSoRTADIR}     #1:4:32.255939
    csh run5_ffmpeg_enc.arthur.wav.opus_mavslmsbank.csh  ${LOGDIR} ${PeSoRTADIR} #1:4:32.255939

    csh run5_ffmpeg_enc.cc.wav.amr_mabank.csh  ${LOGDIR} ${PeSoRTADIR}          #1:27:30.079950
    csh run5_ffmpeg_enc.cc.wav.amr_mavslmsbank.csh  ${LOGDIR} ${PeSoRTADIR}     #1:27:30.079950
    csh run5_ffmpeg_enc.cc.wav.opus_mabank.csh  ${LOGDIR} ${PeSoRTADIR}         #1:27:30.079950
    csh run5_ffmpeg_enc.cc.wav.opus_mavslmsbank.csh  ${LOGDIR} ${PeSoRTADIR}    #1:27:30.079950

