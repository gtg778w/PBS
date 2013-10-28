#include <stdint.h>
#include "libPredictor_variance.h"

void libPredictor_dvariance(double new_val,
                            int32_t limit,      double *x_buffer,
                            double  *mean_p,    double *variance_p)
{
    int32_t i;
    
    /*Variables for shifting the buffer*/
    double  next_indep_var;
    double  prev_indep_var;
    
    /*Variables associated with computing mean and variance in Knuth's algorithm*/
    double Mnext, Mprev, Snext, Sprev;
    double diff_prev;
    double n;
    double mean, variance;
    
    /*The first iteration is done outside the loop*/
        /*compute the statistics of the history of computation times*/
        next_indep_var = new_val;
        
        /*Initialize the M and S variable from Knuth's algorithm*/
        Mnext = next_indep_var;
        Snext = 0.0;

        /*Shift the new value into the buffer*/
        prev_indep_var = next_indep_var;
        next_indep_var = x_buffer[0];
        x_buffer[0]    = prev_indep_var;
        
    /*The remaining iterations are done inside the loop*/
    for(i = 1, n = 2.0; i < limit; i++, n+=1.0)
    {
        /*The following two blocks of code are based on the recursive relation
        presented by Knuth*/
        Mprev = Mnext;
        diff_prev = next_indep_var - Mprev;
        Mnext = Mprev + (diff_prev/n);
        
        Sprev = Snext;
        Snext = Sprev + diff_prev*(next_indep_var - Mnext);
                    
        /*Shift the new value into the buffer*/
        prev_indep_var = next_indep_var;
        next_indep_var = x_buffer[i];
        x_buffer[i]    = prev_indep_var;
    }
    
    /*Extract the mean and variance*/
    mean    = Mnext;
    variance= Snext/(n-1.0);
    
    /*Set the output variables*/
    *mean_p = mean;
    *variance_p = variance;
}

void libPredictor_fvariance(float   new_val,
                            int32_t limit,      float  *x_buffer,
                            double  *mean_p,    double *variance_p)
{
    int32_t i;
    
    /*Variables for shifting the buffer*/
    float  next_indep_var;
    float  prev_indep_var;
    
    /*Variables associated with computing mean and variance in Knuth's algorithm*/
    double Mnext, Mprev, Snext, Sprev;
    double diff_prev;
    double n;
    double mean, variance;
    
    /*The first iteration is done outside the loop*/
        /*compute the statistics of the history of computation times*/
        next_indep_var = new_val;
        
        /*Initialize the M and S variable from Knuth's algorithm*/
        Mnext = (double)next_indep_var;
        Snext = 0.0;

        /*Shift the new value into the buffer*/
        prev_indep_var = next_indep_var;
        next_indep_var = x_buffer[0];
        x_buffer[0]    = prev_indep_var;
        
    /*The remaining iterations are done inside the loop*/
    for(i = 1, n = 2.0; i < limit; i++, n+=1.0)
    {
        /*The following two blocks of code are based on the recursive relation
        presented by Knuth*/
        Mprev = Mnext;
        diff_prev = ((double)next_indep_var) - Mprev;
        Mnext = Mprev + (diff_prev/n);
        
        Sprev = Snext;
        Snext = Sprev + diff_prev*( ((double)next_indep_var) - Mnext);
                    
        /*Shift the new value into the buffer*/
        prev_indep_var = next_indep_var;
        next_indep_var = x_buffer[i];
        x_buffer[i]    = prev_indep_var;
    }
    
    /*Extract the mean and variance*/
    mean    = Mnext;
    variance= Snext/(n-1.0);
    
    /*Set the output variables*/
    *mean_p = mean;
    *variance_p = variance;
}

