#include <stdio.h>
#include <errno.h>

char *usage = "[outer loop iterations] [inner loop iterations]";

int main(int argc, char** argv)
{
    long inner_iterations = 2000000, outer_iterations = 10000;
    long i_inner, i_outer;
    
    char *endptr;
    /*Parse input arguments*/
    switch(argc)
    {
        case 3:
            errno = 0;
            inner_iterations =  strtol(argv[2], &endptr, 0);
            if((errno != 0) || (endptr == argv[2]))
            {
                fprintf(stderr, "main: strtol failed to parse argument 2");
                goto error0;
            }
            
        case 2:
            errno = 0;
            outer_iterations =  strtol(argv[1], &endptr, 0);
            if((errno != 0) || (endptr == argv[1]))
            {
                fprintf(stderr, "main: strtol failed to parse argument 1");
                goto error0;
            }
            
        case 1:
            break;
        
        default:
            fprintf(stderr, "Usage: %s %s\n", argv[0], usage);
            goto error0;
        
    }

    for(i_outer = 0; i_outer < outer_iterations; i_outer++)
    {
        fprintf(stderr, "%li\r", i_outer);
        for(i_inner = 0; i_inner < inner_iterations; i_inner++)
        {
            asm volatile ("nop");
        }
    }
    
    return 0;
error0:
    fprintf(stderr, "%s: main failed\n", argv[0]);
    return -1;
}
