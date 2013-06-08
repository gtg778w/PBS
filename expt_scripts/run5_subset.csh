#! /bin/csh

    set LOGDIR="~/Desktop/log/"
    set PeSoRTADIR=""

    echo "This complete script has an estimated duration of 132:13:10"

    csh run5_ffmpeg_dec.arthur.amr_mabank.csh  ${LOGDIR} ${PeSoRTADIR}          #1:4:32.160000
    csh run5_ffmpeg_dec.arthur.amr_mavslmsbank.csh  ${LOGDIR} ${PeSoRTADIR}     #1:4:32.160000
    csh run5_ffmpeg_dec.arthur.mp3_mabank.csh  ${LOGDIR} ${PeSoRTADIR}          #1:4:33.312000
    csh run5_ffmpeg_dec.arthur.mp3_mavslmsbank.csh  ${LOGDIR} ${PeSoRTADIR}     #1:4:33.312000
    csh run5_ffmpeg_dec.arthur.opus_mabank.csh  ${LOGDIR} ${PeSoRTADIR}         #1:4:33.312000
    csh run5_ffmpeg_dec.arthur.opus_mavslmsbank.csh  ${LOGDIR} ${PeSoRTADIR}    #1:4:33.312000

    csh run5_ffmpeg_dec.cc.amr_mabank.csh  ${LOGDIR} ${PeSoRTADIR}              #1:27:30.24000
    csh run5_ffmpeg_dec.cc.amr_mavslmsbank.csh  ${LOGDIR} ${PeSoRTADIR}         #1:27:30.24000
    csh run5_ffmpeg_dec.cc.opus_mabank.csh  ${LOGDIR} ${PeSoRTADIR}             #1:27:30.24000
    csh run5_ffmpeg_dec.cc.opus_mavslmsbank.csh  ${LOGDIR} ${PeSoRTADIR}        #1:27:30.24000

    csh run5_ffmpeg_dec.deadline.flv_mabank.csh  ${LOGDIR} ${PeSoRTADIR}        #0:27:28.799984
    csh run5_ffmpeg_dec.deadline.flv_mavslmsbank.csh  ${LOGDIR} ${PeSoRTADIR}   #0:27:28.799984
    csh run5_ffmpeg_dec.deadline.mp4_mabank.csh  ${LOGDIR} ${PeSoRTADIR}        #0:27:28.799984
    csh run5_ffmpeg_dec.deadline.mp4_mavslmsbank.csh  ${LOGDIR} ${PeSoRTADIR}   #0:27:28.799984
    csh run5_ffmpeg_dec.deadline.webm_mabank.csh  ${LOGDIR} ${PeSoRTADIR}       #0:27:28.799984
    csh run5_ffmpeg_dec.deadline.webm_mavslmsbank.csh  ${LOGDIR} ${PeSoRTADIR}  #0:27:28.799984

    csh run5_ffmpeg_dec.dsailors.mp3_mabank.csh  ${LOGDIR} ${PeSoRTADIR}        #2:50:42.905760
    csh run5_ffmpeg_dec.dsailors.mp3_mavslmsbank.csh  ${LOGDIR} ${PeSoRTADIR}   #2:50:42.905760

    csh run5_ffmpeg_dec.factory.flv_mabank.csh  ${LOGDIR} ${PeSoRTADIR}         #0:26:46.799984
    csh run5_ffmpeg_dec.factory.flv_mavslmsbank.csh  ${LOGDIR} ${PeSoRTADIR}    #0:26:46.799984
    csh run5_ffmpeg_dec.factory.mp4_mabank.csh  ${LOGDIR} ${PeSoRTADIR}         #0:26:46.799984
    csh run5_ffmpeg_dec.factory.mp4_mavslmsbank.csh  ${LOGDIR} ${PeSoRTADIR}    #0:26:46.799984
    csh run5_ffmpeg_dec.factory.webm_mabank.csh  ${LOGDIR} ${PeSoRTADIR}        #0:33:28.499871
    csh run5_ffmpeg_dec.factory.webm_mavslmsbank.csh  ${LOGDIR} ${PeSoRTADIR}   #0:33:28.499871

    csh run5_ffmpeg_dec.highway.flv_mabank.csh  ${LOGDIR} ${PeSoRTADIR}         #0:39:59.999976
    csh run5_ffmpeg_dec.highway.flv_mavslmsbank.csh  ${LOGDIR} ${PeSoRTADIR}    #0:39:59.999976
    csh run5_ffmpeg_dec.highway.mp4_mabank.csh  ${LOGDIR} ${PeSoRTADIR}         #0:39:59.999976
    csh run5_ffmpeg_dec.highway.mp4_mavslmsbank.csh  ${LOGDIR} ${PeSoRTADIR}    #0:39:59.999976
    csh run5_ffmpeg_dec.highway.webm_mabank.csh  ${LOGDIR} ${PeSoRTADIR}        #0:39:59.999976
    csh run5_ffmpeg_dec.highway.webm_mavslmsbank.csh  ${LOGDIR} ${PeSoRTADIR}   #0:39:59.999976

    csh run5_ffmpeg_dec.mozart.mp3_mabank.csh  ${LOGDIR} ${PeSoRTADIR}          #1:15:4.132800
    csh run5_ffmpeg_dec.mozart.mp3_mavslmsbank.csh  ${LOGDIR} ${PeSoRTADIR}     #1:15:4.132800

    csh run5_ffmpeg_dec.sintel.mp3_mabank.csh  ${LOGDIR} ${PeSoRTADIR}          #0:31:13.152000
    csh run5_ffmpeg_dec.sintel.mp3_mavslmsbank.csh  ${LOGDIR} ${PeSoRTADIR}     #0:31:13.152000

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

    csh run5_ffmpeg_enc.deadline.y4m.flv_mabank.csh  ${LOGDIR} ${PeSoRTADIR}    #0:27:28.799984
    csh run5_ffmpeg_enc.deadline.y4m.flv_mavslmsbank.csh  ${LOGDIR} ${PeSoRTADIR} #0:27:28.799984
    csh run5_ffmpeg_enc.deadline.y4m.mp4_mabank.csh  ${LOGDIR} ${PeSoRTADIR}    #0:27:28.799984
    csh run5_ffmpeg_enc.deadline.y4m.mp4_mavslmsbank.csh  ${LOGDIR} ${PeSoRTADIR} #0:27:28.799984
    csh run5_ffmpeg_enc.deadline.y4m.webm_mabank.csh  ${LOGDIR} ${PeSoRTADIR}   #0:27:28.799984
    csh run5_ffmpeg_enc.deadline.y4m.webm_mavslmsbank.csh  ${LOGDIR} ${PeSoRTADIR} #0:27:28.799984

    csh run5_ffmpeg_enc.dsailors.wav.mp3_mabank.csh  ${LOGDIR} ${PeSoRTADIR}    #2:50:43.507902
    csh run5_ffmpeg_enc.dsailors.wav.mp3_mavslmsbank.csh  ${LOGDIR} ${PeSoRTADIR} #2:50:43.507902

    csh run5_ffmpeg_enc.highway.y4m.flv_mabank.csh  ${LOGDIR} ${PeSoRTADIR}     #0:39:59.999976
    csh run5_ffmpeg_enc.highway.y4m.flv_mavslmsbank.csh  ${LOGDIR} ${PeSoRTADIR} #0:39:59.999976
    csh run5_ffmpeg_enc.highway.y4m.mp4_mabank.csh  ${LOGDIR} ${PeSoRTADIR}     #0:39:59.999976
    csh run5_ffmpeg_enc.highway.y4m.mp4_mavslmsbank.csh  ${LOGDIR} ${PeSoRTADIR} #0:39:59.999976
    csh run5_ffmpeg_enc.highway.y4m.webm_mabank.csh  ${LOGDIR} ${PeSoRTADIR}    #0:39:59.999976
    csh run5_ffmpeg_enc.highway.y4m.webm_mavslmsbank.csh  ${LOGDIR} ${PeSoRTADIR} #0:39:59.999976

    csh run5_ffmpeg_enc.mad900.y4m.flv_mabank.csh  ${LOGDIR} ${PeSoRTADIR}      #0:17:59.999989
    csh run5_ffmpeg_enc.mad900.y4m.flv_mavslmsbank.csh  ${LOGDIR} ${PeSoRTADIR} #0:17:59.999989
    csh run5_ffmpeg_enc.mad900.y4m.mp4_mabank.csh  ${LOGDIR} ${PeSoRTADIR}      #0:17:59.999989
    csh run5_ffmpeg_enc.mad900.y4m.mp4_mavslmsbank.csh  ${LOGDIR} ${PeSoRTADIR} #0:17:59.999989
    csh run5_ffmpeg_enc.mad900.y4m.webm_mabank.csh  ${LOGDIR} ${PeSoRTADIR}     #0:17:59.999989
    csh run5_ffmpeg_enc.mad900.y4m.webm_mavslmsbank.csh  ${LOGDIR} ${PeSoRTADIR} #0:17:59.999989

    csh run5_ffmpeg_enc.mozart.wav.mp3_mabank.csh  ${LOGDIR} ${PeSoRTADIR}      #1:15:3.531957
    csh run5_ffmpeg_enc.mozart.wav.mp3_mavslmsbank.csh  ${LOGDIR} ${PeSoRTADIR} #1:15:3.531957
    csh run5_ffmpeg_enc.sintel.wav.mp3_mabank.csh  ${LOGDIR} ${PeSoRTADIR}      #0:31:12.383971
    csh run5_ffmpeg_enc.sintel.wav.mp3_mavslmsbank.csh  ${LOGDIR} ${PeSoRTADIR} #0:31:12.383971
    
