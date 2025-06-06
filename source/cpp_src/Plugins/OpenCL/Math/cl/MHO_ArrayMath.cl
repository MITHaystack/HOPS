#ifndef MHO_NDArrayMath_CL_H__
#define MHO_NDArrayMath_CL_H__

#include "hopsocl_defines.h"

//modulus of two integers
inline unsigned int
Modulus(unsigned int arg, unsigned int n)
{
    return arg % n;
    // // //returns arg mod n;
    // CL_TYPE Div = ( (CL_TYPE)arg )/( (CL_TYPE) n);
    // return (unsigned int)(fabs( (CL_TYPE)arg - floor(Div)*((CL_TYPE)n) ) );
}

//for a multidimensional array (using row major indexing) which has the
//dimensions specified in DimSize, this function computes the offset from
//the first element given the indices in the array Index
inline unsigned int
OffsetFromRowMajorIndex(unsigned int ndim, const unsigned int* DimSize, const unsigned int* Index)
{
    unsigned int val = Index[0];
    for(unsigned int i=1; i<ndim; i++)
    {
        val *= DimSize[i];
        val += Index[i];
    }
    return val;
}


//for a multidimensional array (using row major indexing) which has the
//dimensions specified in DimSize, this function computes the stride between
//consecutive elements in the selected dimension given that the other indices are fixed
//the first element given the indices in the array Index
inline unsigned int
StrideFromRowMajorIndex(unsigned int ndim, unsigned int selected_dim, const unsigned int* DimSize)
{
    unsigned int val = 1;
    for(unsigned int i=0; i<ndim; i++)
    {
        if(i > selected_dim){val *= DimSize[i];};
    }
    return val;
}



//for a multidimensional array (using row major indexing) which has the
//dimensions specified in DimSize, this function computes the indices of
//the elements which has the given offset from the first element
//must provide a workspace Div[ndim]
inline void
RowMajorIndexFromOffset(unsigned int ndim, const unsigned int offset, const unsigned int* DimSize, unsigned int* Index, unsigned int* Div)
{
    //in row major format the last index varies the fastest
    unsigned int i;

    for(unsigned int d=0; d < ndim; d++)
    {
        i = ndim - d - 1;

        if(d == 0)
        {
            Index[i] = Modulus(offset, DimSize[i]);
            Div[i] = (offset - Index[i])/DimSize[i];
        }
        else
        {
            Index[i] = Modulus(Div[i+1], DimSize[i]);
            Div[i] = (Div[i+1] - Index[i])/DimSize[i];
        }
    }
}

//given the dimensions of an array, computes its total size, assuming all dimensions are non-zero
inline unsigned int
TotalArraySize(unsigned int ndim, const unsigned int* DimSize)
{
    unsigned int val = DimSize[0];
    for(unsigned int i=1; i<ndim; i++)
    {
        val *= DimSize[i];
    }
    return val;
}

#endif /* MHO_NDArrayMath_CL_H__ */
