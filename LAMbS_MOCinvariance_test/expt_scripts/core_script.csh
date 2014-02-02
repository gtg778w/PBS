#! /bin/csh -x
    
    set usagestring = " [<repetitions> [<config name> [<application> [<measure of computation> [<log directory> [<script directory> [<PeSoRTA directory> [<bin dir>]]]]]]]]"
    
    #Set default values for optional input arguments
    set repetitions="1"
    set CONFIGNAME="4"
    set APPNAME="membound"
    set MOC="nsec"
    set LOGDIR="data/expt1/"
    set SCRIPTDIR="expt_scripts"
    set PeSoRTADIR="/media/Data/Research/expt_February2013/PeSoRTA"
    set BINDIR="/home/gtg778w/Desktop/bin"
    
    #Process input arguments for repetitions BINDIR LOGDIR and PeSoRTADIR
    if ( $#argv > 0 ) then
        set repetitions=$argv[1]
        if ( $#argv > 1 ) then
            set CONFIGNAME=$argv[2]
            if ( $#argv > 2 ) then
                set APPNAME=$argv[3]
                if ( $#argv > 3) then
                    set MOC=$argv[4]
                    if ( $#argv > 4) then
                        set LOGDIR=$argv[5]
                        if ( $#argv > 5) then
                            set SCRIPTDIR=$argv[6]
                            if ( $#argv > 6) then
                                set PeSoRTADIR=$argv[7]
                                if ( $#argv > 7) then
                                    set BINDIR=$argv[8]
                                    if ( $#argv > 8) then
                                        echo "Usage:"$argv[0]$usagestring
                                        exit 1
                                    endif
                                endif
                            endif
                        endif
                    endif
                endif
            endif
        endif
    endif
    
    set APPROOTDIR=${PeSoRTADIR}"/"${APPNAME}
    set BINNAME=${APPNAME}_mocsample
    set CONFIGFILE="config/"${CONFIGNAME}".config"
    
    #Loop over the repetitions
    foreach rep_a (`seq 1 1 ${repetitions}`)
        
        #Display progress and estimated duration
        echo
        echo
        echo "Log directory:      ${LOGDIR}"
        echo
        echo "Application name:   ${APPNAME}" 
        echo "Configuration name: ${CONFIGNAME}"
        echo "MOC:                ${MOC}"
        echo 
        echo "Repetition:         ${rep_a} of ${repetitions}"
        echo
        echo

        set LOGFILE=${LOGDIR}"/${rep_a}_${MOC}_${CONFIGNAME}_${APPNAME}.csv"
                    
        #Start the timing workload and background it
        ${BINDIR}/${BINNAME} -r -p 0.0 -R ${APPROOTDIR} -C ${CONFIGFILE} -L ${LOGFILE} -M ${MOC}
        
    end

