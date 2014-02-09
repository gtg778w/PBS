#! /bin/csh -x
    
    set usagestring = " [<repetitions> [<log directory> [<script directory> [<PeSoRTA directory> [<bin dir>]]]]]]]]]"
    
    #Set default values for optional input arguments
    set repetitions="1"
    set LOGDIR_ROOT="data/"
    set SCRIPTDIR="expt_scripts"
    set PeSoRTADIR="/media/Data/Research/expt_February2013/PeSoRTA"
    set BINDIR="/home/gtg778w/Desktop/bin"
    
    #Process input arguments for repetitions BINDIR LOGDIR and PeSoRTADIR
    if ( $#argv > 0 ) then
        set repetitions=$argv[1]
        if ( $#argv > 1) then
            set LOGDIR_ROOT=$argv[2]
            if ( $#argv > 2) then
                set SCRIPTDIR=$argv[3]
                if ( $#argv > 3) then
                    set PeSoRTADIR=$argv[4]
                    if ( $#argv > 4) then
                        set BINDIR=$argv[5]
                        if ( $#argv > 5) then
                            echo "Usage:"$argv[0]$usagestring
                            exit 1
                        endif
                    endif
                endif
            endif
        endif
    endif

    source ${SCRIPTDIR}/cpufreq_inc.csh
    set availablefreqs=$availablefreqs[2]
    
    set APPNAME="membound"
    set CONFIGNAME_ARRAY=("cacheline" "L1cache" "L2cache" "L3cache" "thrash")

    set MOC_ARRAY=("nsec" "cycl" "inst" "userinst") #"nsec" "cycl" "inst" "userinst"
    
    #Loop over each configuration
    foreach CONFIGNAME ( $CONFIGNAME_ARRAY )
        
        #Start the allocator
        #   -f run without prompting
        #   -P scheduling period
        #   -B allocator budget
        #   -s infinite number of scheduling periods
        #   -L no logging
        #
        ${BINDIR}/Allocator -f -P 10000000 -B 500000 -s -1 -L 0 &

        #Busyloop while the Allocator task is active
        ${BINDIR}/poll_pbs_actv -I &

        #Loop over each mode of operation
        foreach MOC ( $MOC_ARRAY )

            #Loop through each CPU frequency
            foreach freq ( $availablefreqs )

                echo "CPU frequency = ${freq}"
                echo                
                #Set the CPU speed                
                echo ${freq} > ${freqdir}/scaling_setspeed
                
                #Create the log directory if not already there.
                set LOGDIR="${LOGDIR_ROOT}/${APPNAME}/${CONFIGNAME}/${MOC}/${freq}"
                mkdir -p ${LOGDIR}
                
                #Run a single repetition of the workload
                csh ${SCRIPTDIR}/core_script.csh  ${repetitions} ${CONFIGNAME} ${APPNAME} ${MOC} ${LOGDIR} ${SCRIPTDIR} ${PeSoRTADIR} ${BINDIR}
                                
            end
            
        end

        #Terminate the Allocator
        echo 0> /proc/sched_pbs_actv
        
    end

