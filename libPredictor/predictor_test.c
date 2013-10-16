#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "libPredictor.h"

int main(int argc, char ** argv)
{
    size_t ret;    
    
    char*file_name;
    FILE *filep;
    char *line = NULL;
    size_t line_len = 0;
    
    char *predictor_name;
    libPredictor_t predictor;
    long long int temp;
    int64_t observed;
    int64_t u_c0 = 0, std_c0 = 0, u_cl = 0, std_cl = 0;
    
    if(argc != 3)
    {
        fprintf(stderr, "%s <predictor> <csv file>", argv[0]);
        ret = -1;
        goto exit0;
    }
    else
    {
        predictor_name = argv[1];
        file_name = argv[2];
    }
    
    filep = fopen(file_name, "r");
    if(NULL == filep)
    {
        fprintf(stderr, "Failed to open %s for reading!\n", file_name);
        perror("fopen failed:");
        ret = -1;
        goto exit0;
    }
    
    ret = libPredictor_getPredictor(&predictor, predictor_name);
    if(ret != 0)
    {
        fprintf(stderr, "libPredictor_getPredictor for predictor name \"%s\"!\n",
                        predictor_name);
        ret = -1;
        goto exit1;
    }
    
    while((ret = getline(&line, &line_len, filep)) != -1)
    {
        errno = 0;
        ret = sscanf(line, "%lli, ", &temp);
        if(1 != ret)
        {
            fprintf(stderr, "Failed to parse csv file!");
            goto exit2;
        }
        else
        {
            observed = (int64_t)temp;
            ret = 0;
        }
        
        printf("%lli, ", (long long int)observed);
        
        ret = predictor.update(predictor.state, observed, 
                               &u_c0, &std_c0, &u_cl, &std_cl);
        if(ret == -1)
        {
            u_c0    = 0;
            std_c0  = 0;
            u_cl    = 0;
            std_cl  = 0;
        }
        
        printf("%lli, %lli, %lli, %lli\n", 
                (long long int)u_c0, 
                (long long int)std_c0, 
                (long long int)u_cl, 
                (long long int)std_cl);
    }
    
    if(!feof(filep))
    {
        fprintf(stderr, "Failed to read line from %s!\n", file_name);
        perror("getline failed");
    }

exit2:
    libPredictor_freePredictor((&predictor));
    if(NULL != line)
    {
        free(line);
    }
exit1:
    fclose(filep);    
exit0:
    return ret;
}

