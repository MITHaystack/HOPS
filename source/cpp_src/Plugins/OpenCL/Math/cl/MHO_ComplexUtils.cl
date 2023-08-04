#ifndef MHO_ComplexUtils_CL_H__
#define MHO_ComplexUtils_CL_H__

#include "hopsocl_defines.h"

inline CL_TYPE2 ComplexMultiply(CL_TYPE2 x, CL_TYPE2 y)
{
    CL_TYPE2 z;
    z.s0 = x.s0*y.s0 - x.s1*y.s1;
    z.s1 = x.s0*y.s1 + x.s1*y.s0;
    return z;
}

inline CL_TYPE2 ComplexConjugate(CL_TYPE2 x)
{
    CL_TYPE2 z;
    z.s0 = x.s0;
    z.s1 = -x.s1;
    return z;
}

//strided, conjugate array
void Conjugate(unsigned int N, unsigned int stride, CL_TYPE2* array)
{
    for(unsigned int i=0; i<N; i++){ array[i*stride] = ComplexConjugate(array[i*stride]); }
}

#endif /*end of MHO_ComplexUtils_CL_H__ */
