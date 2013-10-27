#ifndef LIBPREDICTOR_VARIANCE_INCLUDE
#define LIBPREDICTOR_VARIANCE_INCLUDE

/*
    libPredictor_dvariance: used for computing a running mean and average
    
    o the user passes the 
    - the latest observation
    - the length of the buffer
    - buffer containing previous observations, newest to oldest, newest value 
      in index 0
    - output variables to store the mean and variance
    
    o the function 
    - inserts the new observation into the buffer and shifts 
      previous observations over by one
    - computes the mean and variance using Knuth's algorithm
    
*/
void libPredictor_dvariance(double  new_val,
                            int32_t limit,      double *x_buffer,
                            double  *mean_p,    double *variance_p);

#endif
