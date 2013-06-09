#! /bin/csh

    set LOGDIR="~/Desktop/log/"
    set PeSoRTADIR="../PeSORTA"

    echo "This complete script has an estimated duration of 9:13:57"

    #2:50:43.507902
    csh expt_scripts/run5_ffmpeg_enc.dsailors.wav.mp3_mabank.csh  ${LOGDIR} ${PeSoRTADIR}
    #2:50:43.507902
    csh expt_scripts/run5_ffmpeg_enc.dsailors.wav.mp3_mavslmsbank.csh  ${LOGDIR} ${PeSoRTADIR}
    
    #1:15:3.531957
    csh expt_scripts/run5_ffmpeg_enc.mozart.wav.mp3_mabank.csh  ${LOGDIR} ${PeSoRTADIR}
    #1:15:3.531957
    csh expt_scripts/run5_ffmpeg_enc.mozart.wav.mp3_mavslmsbank.csh  ${LOGDIR} ${PeSoRTADIR}
    
    #0:31:12.383971
    csh expt_scripts/run5_ffmpeg_enc.sintel.wav.mp3_mabank.csh  ${LOGDIR} ${PeSoRTADIR}
    #0:31:12.383971
    csh expt_scripts/run5_ffmpeg_enc.sintel.wav.mp3_mavslmsbank.csh  ${LOGDIR} ${PeSoRTADIR}

    #6:192:117
    #9:13:57

