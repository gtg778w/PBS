#! /bin/csh -x
    
    #The name of the application being used for the tests
    set predictor="ma"
    set PREDDIR="prdtst_data/"
    set config="base"
    set application="base"
    set TIMEDIR="timing_data/Core2_extreme/"
    set BINDIR="/home/gtg778w/Desktop/bin/"
    
    #Process input arguments for predictor PREDDIR config application TIMEDIR and BINDIR
    if ( $#argv == 1 ) then
        set predictor=$argv[1]
    else if ( $#argv == 2 ) then
        set predictor=$argv[1]
        set PREDDIR=$argv[2]
    else if ( $#argv == 3 ) then
        set predictor=$argv[1]
        set PREDDIR=$argv[2]
        set config=$argv[3]
    else if ( $#argv == 4 ) then
        set predictor=$argv[1]
        set PREDDIR=$argv[2]
        set config=$argv[3]
        set application=$argv[4]
    else if ( $#argv == 5 ) then
        set predictor=$argv[1]
        set PREDDIR=$argv[2]
        set config=$argv[3]
        set application=$argv[4]
        set TIMEDIR=$argv[5]
    else if ( $#argv == 6 ) then
        set predictor=$argv[1]
        set PREDDIR=$argv[2]
        set config=$argv[3]
        set application=$argv[4]
        set TIMEDIR=$argv[5]
        set BINDIR=$argv[6]
    else if ( $#argv > 6 ) then
        echo "Usage:"$argv[0]" [predictor [prediction data dir [config [application [timing data dir [bin dir]]]]]]"
        exit 1
    endif
        
    set bin="${BINDIR}/predictor_test"
    
    set TIMEDIR_LOCAL="${TIMEDIR}/${application}/${config}"
    set timcsv_prefix="timing.${application}.${config}"
    
    set timcsv_list=`ls ${TIMEDIR_LOCAL}`
    set repetitions=$#timcsv_list
    
    set PREDDIR_LOCAL="${PREDDIR}/${application}/${config}/${predictor}/"
    set prdcsv_prefix="prediction.${application}.${config}.${predictor}"
    mkdir -p ${PREDDIR_LOCAL}
    
    echo
    echo "application="${application}
    echo "config="${config}
    echo "predictor="${predictor}
    echo "repetitions="${repetitions}
    echo

    #loop through the repetitions of the experiment
    foreach r (`seq 1 1 ${repetitions}`)
        set timcsv_name="${TIMEDIR_LOCAL}/${timcsv_prefix}.${r}.csv"
        set prdcsv_name="${PREDDIR_LOCAL}/${prdcsv_prefix}.${r}.csv"
    
        echo 
        echo "    input  file:  ${timcsv_name}"
        echo "    output file:  ${prdcsv_name}"
        echo
        
        #do the prediction
        ${bin} -I ${timcsv_name} -P ${predictor} -L ${prdcsv_name}
        
    end

