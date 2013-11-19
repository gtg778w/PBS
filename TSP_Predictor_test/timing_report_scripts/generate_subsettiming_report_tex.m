
%The purpose of this script is to generate a single tex file containing a table with the 
%timing data associated with each of the workloads. The table to be generated is as 
%follows:
%
%   Workload, Mean, Std, Max ACF, Max ACF Lag
%
%

    %Open the file
    report_name = '../timing_reports/timing_summary_table.tex';
    fid = fopen(report_name, 'w');
 
     %Table header
    fprintf(fid, '\\begin{table}[h]\n');
    fprintf(fid, '\\renewcommand{\\arraystretch}{1.3}\n');
    fprintf(fid, '\\caption{Workload Execution-Time Statistics}\n');
    fprintf(fid, '\\label{tabl_TSP_prediction_workload_statistics}\n');
    fprintf(fid, '\\centering\n');
    fprintf(fid, '    \\begin{tabular} {| c | c | c | c | c | c |}\n');
    fprintf(fid, '        \\hline\n');
    fprintf(fid, '        Workload & Mean (ns) & Std (ns) & Max (ns) & Max ACF & Max ACF Lag \\\\\n');
    fprintf(fid, '        \\hline\n');
    
    %Workload-related arrays
    datadir = '../timing_data_summary/';
    workload_name = {   'encode speech', ...
                        'decode speech', ...
                        'encode audio', ...
                        'decode audio', ...
                        'encode video', ...
                        'decode video', ...
                        'sqrwav'};
                        
    workload_list = {   'ffmpeg/ffmpeg.enc.arthur.wav.amr.mat', ...
                        'ffmpeg/ffmpeg.dec.arthur.spx.mat', ...
                        'ffmpeg/ffmpeg.enc.beethoven.wav.mp3.mat', ...
                        'ffmpeg/ffmpeg.dec.mozart.mp3.mat', ...
                        'ffmpeg/ffmpeg.enc.highway.y4m.mp4.mat', ...
                        'ffmpeg/ffmpeg.dec.sintel.mp4.mat', ...
                        'sqrwav/sqrwav.example.mat'};

    %Loop through each workload
    for w_i = 1:length(workload_list)
    
        %Get the corresponding matfile
        matfile_path = sprintf('%s/%s', datadir, workload_list{w_i});
        load(matfile_path);
    
        fprintf(fid, '        %s & %.2e & %.2e & %.2e & %.2f & %i \\\\\n', ...
                    workload_name{w_i}, ...
                    summary.mean, ...
                    summary.std, ...
                    summary.max, ...
                    summary.max_acor(1), ...
                    summary.max_acor_lag(1));
    end

    %Output the table footer
    fprintf(fid, '        \\hline\n');
    fprintf(fid, '    \\end{tabular}\n');
    fprintf(fid, '\\end{table}\n');
    
    %close the file
    fclose(fid);
    
