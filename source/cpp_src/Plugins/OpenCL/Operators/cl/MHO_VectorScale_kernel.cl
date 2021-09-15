#ifndef NTSVectorScale_Defined_H
#define NTSVectorScale_Defined_H

#include "hopsocl_defines.h"
#include "MHO_ComplexMultiply.cl"

__kernel void
VectorScale(const unsigned int array_size,
            CL_FACTOR_TYPE factor, //CL_FACTOR_TYPE is passed a compiler define
            __global CL_DATA_TYPE* data) //CL_DATA_TYPE is passed a compiler define
{
    int i = get_global_id(0);
    if(i < array_size)
    {
        #ifdef COMPLEX_COMPLEX
            data[i] = ComplexMultiply(data[i], factor); //do complex*complex multiply
        #else
            data[i] *= factor; //regular scalar multiply
        #endif
    }
}

#endif
