#! /bin/csh -x
    
    #Set default values for optional input arguments
    set repetitions="1"
    #The name of the application being used for the tests
    set application="base"
    set config="base"
    set LOGDIR="timing_data/"
    set PeSoRTADIR="/media/Data/Research/expt_February2013/PeSoRTA"
    set BINDIR="/home/gtg778w/Desktop/bin/"
    
    #Process input arguments for repetitions BINDIR LOGDIR and PeSoRTADIR
    if ( $#argv == 1 ) then
        set repetitions=$argv[1]
    else if ( $#argv == 2 ) then
        set repetitions=$argv[1]
        set config=$argv[2]
    else if ( $#argv == 3 ) then
        set repetitions=$argv[1]
        set config=$argv[2]
        set application=$argv[3]
    else if ( $#argv == 4 ) then
        set repetitions=$argv[1]
        set config=$argv[2]
        set application=$argv[3]
        set LOGDIR=$argv[4]
    else if ( $#argv == 5 ) then
        set repetitions=$argv[1]
        set config=$argv[2]
        set application=$argv[3]
        set LOGDIR=$argv[4]
        set PeSoRTADIR=$argv[5]
    else if ( $#argv == 6 ) then
        set repetitions=$argv[1]
        set config=$argv[2]
        set application=$argv[3]
        set LOGDIR=$argv[4]
        set PeSoRTADIR=$argv[5]
        set BINDIR=$argv[6]
    else if ( $#argv > 6 ) then
        echo "Usage:"$argv[0]" [repetitions [config [application [log directory [PeSoRTA directory [bin dir]]]]]]"
        exit 1
    endif

    #The directory of the application containing the relevant config directory 
    #and data directory
    set appdir="${PeSoRTADIR}/${application}"
    set config_file="${appdir}/config/${config}.config"
        
    set bin="${BINDIR}/${application}_timing"
    
    set LODIR_LOCAL="${LOGDIR}/${application}/${config}"
    set csv_prefix="timing.${application}.${config}"
    
    mkdir -p ${LODIR_LOCAL}
    
    echo
    echo "application="${application}
    echo "config="${config}
    echo

    #loop through the repetitions of the experiment
    foreach r (`seq 1 1 ${repetitions}`)
        set csv_name="${LODIR_LOCAL}/${csv_prefix}.${r}.csv"
    
        echo 
        echo ${csv_name}":"            
        echo "    running:      ${bin}"
        echo "    exp. dir:     ${appdir}"
        echo "    config file:  ${config_file}"
        echo "    results file: ${csv_name}"
        echo
        #run the experiment
        
        ${bin} -r -R ${appdir} -C ${config_file} -L ${csv_name}
        
    end

