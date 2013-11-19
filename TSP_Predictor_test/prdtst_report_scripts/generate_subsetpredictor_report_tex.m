
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
    fidMA           = fopen('../prdtst_reports/predictor_summary_table_MA.tex', 'w');
    fidMAbank       = fopen('../prdtst_reports/predictor_summary_table_MAbank.tex', 'w');
    fidMAVSLMSbank  = fopen('../prdtst_reports/predictor_summary_table_Hybrid.tex', 'w');
    fidCompare      = fopen('../prdtst_reports/predictor_summary_table_Compare.tex', 'w');
    
     %Table headers
    fprintf(fidMA,  '\\begin{table}[h]\n');
    fprintf(fidMA,  '\\renewcommand{\\arraystretch}{1.3}\n');
    fprintf(fidMA,  '\\caption{Performance of a Fixed-Length Moving Average}\n');
    fprintf(fidMA,  '\\label{tabl_TSP_prediction_MA_performance}\n');
    fprintf(fidMA,  '\\centering\n');
    fprintf(fidMA,  '    \\begin{tabular} {| c | c | c |}\n');
    fprintf(fidMA,  '        \\hline\n');
    fprintf(fidMA,  '        Workload & Overhead (ns) & NRMS error \\\\\n');
    fprintf(fidMA,  '        \\hline\n');
    
    fprintf(fidMAbank,  '\\begin{table}[h]\n');
    fprintf(fidMAbank,  '\\renewcommand{\\arraystretch}{1.3}\n');
    fprintf(fidMAbank,  '\\caption{Performance of a Bank of Moving Averages}\n');
    fprintf(fidMAbank,  '\\label{tabl_TSP_prediction_MAbank_performance}\n');
    fprintf(fidMAbank,  '\\centering\n');
    fprintf(fidMAbank,  '    \\begin{tabular} {| c | c | c |}\n');
    fprintf(fidMAbank,  '        \\hline\n');
    fprintf(fidMAbank,  '        Workload & Overhead (ns) & NRMS error \\\\\n');
    fprintf(fidMAbank,  '        \\hline\n');
    
    fprintf(fidMAVSLMSbank, '\\begin{table}[h]\n');
    fprintf(fidMAVSLMSbank, '\\renewcommand{\\arraystretch}{1.3}\n');
    fprintf(fidMAVSLMSbank, '\\caption{Performance of a Predictor based on LMS Filtering}\n');
    fprintf(fidMAVSLMSbank, '\\label{tabl_TSP_prediction_hybrid_performance}\n');
    fprintf(fidMAVSLMSbank, '\\centering\n');
    fprintf(fidMAVSLMSbank, '    \\begin{tabular} {| c | c | c |}\n');
    fprintf(fidMAVSLMSbank, '        \\hline\n');
    fprintf(fidMAVSLMSbank, '        Workload & Overhead (ns) & NRMS error \\\\\n');
    fprintf(fidMAVSLMSbank, '        \\hline\n');
    
    fprintf(fidCompare, '\\begin{table}[h]\n');
    fprintf(fidCompare, '\\renewcommand{\\arraystretch}{1.3}\n');
    fprintf(fidCompare, '\\caption{Performance Comparison of the Predictors}\n');
    fprintf(fidCompare, '\\label{tabl_TSP_prediction_compare_performance}\n');
    fprintf(fidCompare, '\\centering\n');
    fprintf(fidCompare, '    \\begin{tabular} {| c | c | c | c |}\n');
    fprintf(fidCompare, '        \\hline\n');
    fprintf(fidCompare, '        Workload & MA & MA Bank & LMS Hybrid \\\\\n');
    fprintf(fidCompare, '        \\hline\n');
    
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
        
        %Load the mat file for MA
        matfile_name= sprintf('%s/%s.%s.mabank.mat',matfile_dir, ...
                                                    application_list{w_i}, ...
                                                    config_list{w_i});
        load(matfile_name);
        
        %Get the overhead and normalized RMS error
        NRMSE_MAbank =  summary.rms_error / summary.std_exectimes;
        OVRHD_MAbank =  summary.mean_overhead;
        
        %Load the mat file for MA
        matfile_name= sprintf('%s/%s.%s.mavslmsbank.mat',   matfile_dir, ...
                                                            application_list{w_i}, ...
                                                            config_list{w_i});
        load(matfile_name);
        
        %Get the overhead and normalized RMS error
        NRMSE_MAVSLMSbank = summary.rms_error / summary.std_exectimes;
        OVRHD_MAVSLMSbank = summary.mean_overhead;
        
        %Print the workload name in the first column
        fprintf(fidMA,          '        %s & %.2f & %.2f \\\\\n', ...
                                workload_name{w_i}, ...
                                OVRHD_MA, ...
                                NRMSE_MA);
        
        fprintf(fidMAbank,      '        %s & %.2f & %.2f \\\\\n', ...
                                workload_name{w_i}, ...
                                OVRHD_MAbank, ...
                                NRMSE_MAbank);
                                
        fprintf(fidMAVSLMSbank, '        %s & %.2f & %.2f \\\\\n', ...
                                workload_name{w_i}, ...
                                OVRHD_MAVSLMSbank, ...
                                NRMSE_MAVSLMSbank);
                                
        fprintf(fidCompare,     '        %s & %.2f & %.2f & %.2f \\\\\n', ...
                                workload_name{w_i}, ...
                                NRMSE_MA, ...
                                NRMSE_MAbank, ...
                                NRMSE_MAVSLMSbank);
    end

    %Output the table footer
    fprintf(fidMA,  '        \\hline\n');
    fprintf(fidMA,  '    \\end{tabular}\n');
    fprintf(fidMA,  '\\end{table}\n');
    
    fprintf(fidMAbank,  '        \\hline\n');
    fprintf(fidMAbank,  '    \\end{tabular}\n');
    fprintf(fidMAbank,  '\\end{table}\n');
    
    fprintf(fidMAVSLMSbank, '        \\hline\n');
    fprintf(fidMAVSLMSbank, '    \\end{tabular}\n');
    fprintf(fidMAVSLMSbank, '\\end{table}\n');
    
    fprintf(fidCompare, '        \\hline\n');
    fprintf(fidCompare, '    \\end{tabular}\n');
    fprintf(fidCompare, '\\end{table}\n');
    
    %close the files
    fclose(fidMA);
    fclose(fidMAbank);
    fclose(fidMAVSLMSbank);
    fclose(fidCompare);
    
