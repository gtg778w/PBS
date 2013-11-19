
%The purpose of this script is to generate three tex file containing a table with the 
%predictor erformance each workload. The table to be generated is as 
%follows:
%
%   Workload, Predictor Overhead, Normalized RMS Predictor error
%
%The script should also generate a fourth tex file containing a table comparing 
%performance accross all predictors:
%
%   Workload, MA, MABank, Hybrid
%

    %Open the files
    fidCompareO      = fopen('../prdtst_reports/predictor_summary_table_CompareOverhead.tex', 'w');
    fidCompareM      = fopen('../prdtst_reports/predictor_summary_table_CompareMSE.tex', 'w');
    fidCompareA      = fopen('../prdtst_reports/predictor_summary_table_CompareErrorAutocorr.tex', 'w');
        
    fprintf(fidCompareO, '\\begin{table}[h]\n');
    fprintf(fidCompareO, '\\renewcommand{\\arraystretch}{1.3}\n');
    fprintf(fidCompareO, '\\caption{Comparison of Predictor Overheads}\n');
    fprintf(fidCompareO, '\\label{tabl_TSP_prediction_compare_overhead}\n');
    fprintf(fidCompareO, '\\centering\n');
    fprintf(fidCompareO, '    \\begin{tabular} {| c | c | c | c |}\n');
    fprintf(fidCompareO, '        \\hline\n');
    fprintf(fidCompareO, '        Workload & MA & MA Bank & LMS Hybrid \\\\\n');
    fprintf(fidCompareO, '        \\hline\n');
    
    fprintf(fidCompareM, '\\begin{table}[h]\n');
    fprintf(fidCompareM, '\\renewcommand{\\arraystretch}{1.3}\n');
    fprintf(fidCompareM, '\\caption{Comparison of Predictor Performance}\n');
    fprintf(fidCompareM, '\\label{tabl_TSP_prediction_compare_performance}\n');
    fprintf(fidCompareM, '\\centering\n');
    fprintf(fidCompareM, '    \\begin{tabular} {| c | c | c | c |}\n');
    fprintf(fidCompareM, '        \\hline\n');
    fprintf(fidCompareM, '        Workload & MA & MA Bank & LMS Hybrid \\\\\n');
    fprintf(fidCompareM, '        \\hline\n');
    
    fprintf(fidCompareA, '\\begin{table}[h]\n');
    fprintf(fidCompareA, '\\renewcommand{\\arraystretch}{1.3}\n');
    fprintf(fidCompareA, '\\caption{Comparison of Autocorrelation in the Predictor Error}\n');
    fprintf(fidCompareA, '\\label{tabl_TSP_prediction_compare_performance}\n');
    fprintf(fidCompareA, '\\centering\n');
    fprintf(fidCompareA, '    \\begin{tabular} {| c | c | c | c |}\n');
    fprintf(fidCompareA, '        \\hline\n');
    fprintf(fidCompareA, '        Workload & MA & MA Bank & LMS Hybrid \\\\\n');
    fprintf(fidCompareA, '        \\hline\n');
    
    %Workload-related arrays
    prdtst_dir = '../prdtst_data_summary/';
    workload_name = {   'encode speech', ...
                        'decode speech', ...
                        'encode audio', ...
                        'decode audio', ...
                        'encode video', ...
                        'decode video', ...
                        'square-wave'};
    
    application_list = {'ffmpeg', ...
                        'ffmpeg', ...
                        'ffmpeg', ...
                        'ffmpeg', ...
                        'ffmpeg', ...
                        'ffmpeg', ...
                        'sqrwav'};
    
    config_list = { 'enc.arthur.wav.amr', ...
                    'dec.arthur.spx', ...
                    'enc.beethoven.wav.mp3', ...
                    'dec.mozart.mp3', ...
                    'enc.highway.y4m.mp4', ...
                    'dec.sintel.mp4', ...
                    'example'};

    %Loop through each workload
    for w_i = 1:length(workload_name)

        %Get the corresponding dir
        matfile_dir = sprintf('%s/%s/%s/',  prdtst_dir, ...
                                            application_list{w_i}, ...
                                            config_list{w_i});
                                            
        %Load the mat file for MA
        matfile_name= sprintf('%s/%s.%s.ma.mat',matfile_dir, ...
                                                application_list{w_i}, ...
                                                config_list{w_i});
        load(matfile_name);
        
        %Get the overhead and normalized RMS error
        NRMSE_MA = summary.rms_error / summary.std_exectimes;
        OVRHD_MA = summary.mean_overhead;
        ACOR_MA  = summary.acor_max_error;
        ACORLAG_MA = summary.acor_max_lag_error;
        
        %Load the mat file for MABANK
        matfile_name= sprintf('%s/%s.%s.mabank.mat',matfile_dir, ...
                                                    application_list{w_i}, ...
                                                    config_list{w_i});
        load(matfile_name);
        
        %Get the overhead and normalized RMS error
        NRMSE_MAbank =  summary.rms_error / summary.std_exectimes;
        OVRHD_MAbank =  summary.mean_overhead;
        ACOR_MAbank  =  summary.acor_max_error;
        ACORLAG_MAbank = summary.acor_max_lag_error;
        
        %Load the mat file for MAVSLMSBank
        matfile_name= sprintf('%s/%s.%s.mavslmsbank.mat',   matfile_dir, ...
                                                            application_list{w_i}, ...
                                                            config_list{w_i});
        load(matfile_name);
        
        %Get the overhead and normalized RMS error
        NRMSE_MAVSLMSbank = summary.rms_error / summary.std_exectimes;
        OVRHD_MAVSLMSbank = summary.mean_overhead;
        ACOR_MAVSLMSbank  = summary.acor_max_error;
        ACORLAG_MAVSLMSbank = summary.acor_max_lag_error;

        fprintf(fidCompareO,    '        %s & %.2f & %.2f & %.2f \\\\\n', ...
                                workload_name{w_i}, ...
                                OVRHD_MA, ...
                                OVRHD_MAbank, ...
                                OVRHD_MAVSLMSbank);
                                        
        fprintf(fidCompareM,    '        %s & %.2f & %.2f & %.2f \\\\\n', ...
                                workload_name{w_i}, ...
                                NRMSE_MA, ...
                                NRMSE_MAbank, ...
                                NRMSE_MAVSLMSbank);
        
        ACOR_MA
        ACOR_MAbank
        ACOR_MAVSLMSbank
        
        fprintf(fidCompareA,    '        %s & %.2f & %.2f & %.2f \\\\\n', ...
                                workload_name{w_i}, ...
                                ACOR_MA(1), ...
                                ACOR_MAbank(1), ...
                                ACOR_MAVSLMSbank(1));
    end

    %Output the table footer
    fprintf(fidCompareO,    '        \\hline\n');
    fprintf(fidCompareO,    '    \\end{tabular}\n');
    fprintf(fidCompareO,    '\\end{table}\n');
    
    %Output the table footer
    fprintf(fidCompareM,    '        \\hline\n');
    fprintf(fidCompareM,    '    \\end{tabular}\n');
    fprintf(fidCompareM,    '\\end{table}\n');
    
    %Output the table footer
    fprintf(fidCompareA,    '        \\hline\n');
    fprintf(fidCompareA,    '    \\end{tabular}\n');
    fprintf(fidCompareA,    '\\end{table}\n');
    
    %close the files
    fclose(fidCompareO);
    fclose(fidCompareM);
    fclose(fidCompareA);
    
