#include "hops_complex.h"

void zero_complex(hops_complex* val)
{
    #ifdef USE_C_COMPLEX
        *val = 0.0;
    #else
        (*val)[0] = 0.0;
        (*val)[1] = 0.0;
    #endif
}


void set_complex(hops_complex* val, double real, double imag)
{
    #ifdef USE_C_COMPLEX
        *val = real + I*imag;
    #else
        (*val)[0] = real;
        (*val)[1] = imag;
    #endif
}

double abs_complex(hops_complex* val)
{
    #ifdef USE_C_COMPLEX
        return cabs(*val);
    #else
        double real = (*val)[0];
        double imag = (*val)[1];
        return sqrt(real*real + imag*imag);
    #endif
}