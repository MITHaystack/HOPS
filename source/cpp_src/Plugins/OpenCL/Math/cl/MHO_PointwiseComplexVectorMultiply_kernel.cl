#ifndef MHO_PointwiseComplexVectorMultiply_Defined_H
#define MHO_PointwiseComplexVectorMultiply_Defined_H

#include "hopsocl_defines.h"

__kernel void
PointwiseComplexVectorMultiply(const unsigned int array_size,
                               __global CL_TYPE2* input1,
                               __global CL_TYPE2* input2,
                               __global CL_TYPE2* output)
{
    int i = get_global_id(0);
    if(i < array_size)
    {
        CL_TYPE2 a = input1[i];
        CL_TYPE2 b = input2[i];
        CL_TYPE2 c;

        c.x = (a.x)*(b.x) - (a.y)*(b.y);
        c.y = (a.x)*(b.y) + (a.y)*(b.x);

        output[i] = c;
    }
}

#endif
