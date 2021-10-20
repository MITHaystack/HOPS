// minvert () is passed an n x n matrix a 
// and returns its inverse in ainv
// rc is non-zero if matrix a is singular
//
// initial code (adapted from internet version)  2019.9.6  rjc

#include <stdio.h>
#include <math.h>

int minvert (size_t n,              // matrix size is n x n
             double* a,        // input matrix
             double* ainv)     // inverse of the input matrix (returned)
    {
    int i, 
        j, 
        k, 
        rc = 0;

    double x[2*n*n], 
           ratio, 
           c;

    for (i=0; i<n; i++)             // copy a xrix into work area
        for (j=0; j<2*n; j++)
            if (j < n)
                x[i*n + j] = a[i*n + j];
            else
                x[i*n + j] = 0;

    for(i = 0; i < n; i++)
        for(j = n; j < 2*n; j++)
            if(i==(j-n))
                x[i*n + j] = 1.0;
            else
                x[i*n + j] = 0.0;
        
    for(i = 0; i < n; i++)
        for(j = 0; j < n; j++)
            if(i != j)
                {
                ratio = x[j*n + i]/x[i*n + i];
                for(k = 0; k < 2*n; k++)
                    x[j*n+k] -= ratio * x[i*n+k];
                }

    for(i = 0; i < n; i++)
        {
        c = x[i*n + i];
        for(j = n; j < 2*n; j++)
            {
            x[i*n + j] /= c;
            rc |= isnan (x[i*n + j]);
            }
        }

    for (i=0; i<n; i++)             // copy inverse into ainv
        for (j=0; j<n; j++)
            ainv[i*n + j] = x[i*n+j+n];

    return rc;
    }
