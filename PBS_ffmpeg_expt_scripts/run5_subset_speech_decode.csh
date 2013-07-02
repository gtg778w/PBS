#! /bin/csh

    set LOGDIR="~/Desktop/log/"
    set PeSoRTADIR="../PeSoRTA"

    echo "This complete script has an estimated duration of 12:14:00"

    #1:4:32.160000
    csh expt_scripts/run5_ffmpeg_dec.arthur.amr_mabank.csh  ${LOGDIR} ${PeSoRTADIR}
    #1:4:32.160000
    csh expt_scripts/run5_ffmpeg_dec.arthur.amr_mavslmsbank.csh  ${LOGDIR} ${PeSoRTADIR}
    #1:4:33.312000
    csh expt_scripts/run5_ffmpeg_dec.arthur.opus_mabank.csh  ${LOGDIR} ${PeSoRTADIR}     
    #1:4:33.312000
    csh expt_scripts/run5_ffmpeg_dec.arthur.opus_mavslmsbank.csh  ${LOGDIR} ${PeSoRTADIR}
    #1:4:33.312000
    csh expt_scripts/run5_ffmpeg_dec.arthur.mp3_mabank.csh  ${LOGDIR} ${PeSoRTADIR}    
    #1:4:33.312000
    csh expt_scripts/run5_ffmpeg_dec.arthur.mp3_mavslmsbank.csh  ${LOGDIR} ${PeSoRTADIR}

    #1:27:30.24000
    csh expt_scripts/run5_ffmpeg_dec.cc.amr_mabank.csh  ${LOGDIR} ${PeSoRTADIR}
    #1:27:30.24000
    csh expt_scripts/run5_ffmpeg_dec.cc.amr_mavslmsbank.csh  ${LOGDIR} ${PeSoRTADIR}
    #1:27:30.24000
    csh expt_scripts/run5_ffmpeg_dec.cc.opus_mabank.csh  ${LOGDIR} ${PeSoRTADIR}
    #1:27:30.24000
    csh expt_scripts/run5_ffmpeg_dec.cc.opus_mavslmsbank.csh  ${LOGDIR} ${PeSoRTADIR}

    #10:132:120
    #12:14:00

