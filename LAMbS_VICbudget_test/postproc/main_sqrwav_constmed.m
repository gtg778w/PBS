
    sqrwav_constmed_dir =  '../data/sqrwav/constant_med';
    
    freqcycle_ns_dir    = [sqrwav_constmed_dir, '/freqcycle/ns'];
    freqcycle_VIC_dir   = [sqrwav_constmed_dir, '/freqcycle/VIC'];
    
    filename = [freqcycle_ns_dir, '/', '1_freqcycle_SRT_PeSoRTA_sqrwav_constant_med_7000_mavslmsbank_40000000_20000000_1.555555.log'];
    data_freqcycle_ns = parse_csv_file(filename);
    
    filename = [freqcycle_VIC_dir, '/', '1_freqcycle_SRT_PeSoRTA_sqrwav_constant_med_7000_mavslmsbank_40000000_20000000_1.555555.log'];
    data_freqcycle_VIC = parse_csv_file(filename);
    
    VFT_eror_plot = plot_VFT_error(data_freqcycle_ns, 'freqcycle ns-based budget');
    VFT_eror_plot = plot_VFT_error(data_freqcycle_VIC, 'freqcycle VIC-based budget');
    
    
