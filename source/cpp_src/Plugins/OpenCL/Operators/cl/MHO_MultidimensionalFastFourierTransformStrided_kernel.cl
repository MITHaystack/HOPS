#ifndef MHO_MultidimensionalFastFourierTransform_Defined_H
#define MHO_MultidimensionalFastFourierTransform_Defined_H

#include "hopsocl_defines.h"
#include "MHO_ArrayMath.cl"
#include "MHO_ComplexUtils.cl"
#include "MHO_FastFourierTransformUtilities.cl"

//compile time constants
//FFT_NDIM

__kernel void
MultidimensionalFastFourierTransform_Radix2Stage(
    unsigned int D, //d = 0, 1, ...FFT_NDIM-1 specifies the dimension/axis selected to be transformed
    __global const unsigned int* dim, //sizes of the array in each dimension
    __constant const CL_TYPE2* twiddle, //fft twiddle factors
    __constant const unsigned int* permutation_array, //bit reversal permutation indices
    __global CL_TYPE2* data, // the data to be transformed
    __global CL_TYPE2* workspace // workspace 
)
{
    //get the index of the current work thread
    unsigned int i_global = get_global_id(0);

    //workspace for index calculations
    unsigned int index[FFT_NDIM];
    unsigned int div_space[FFT_NDIM-1];
    unsigned int non_active_dimension_size[FFT_NDIM-1];
    unsigned int non_active_dimension_value[FFT_NDIM-1];
    unsigned int non_active_dimension_index[FFT_NDIM-1];

    //figure out the total number of 1-D FFTs to perform along this axis
    unsigned int n_fft;
    n_fft = CalculateWorkItemInfo(FFT_NDIM, D, dim, non_active_dimension_index, non_active_dimension_size);

    //figure out which chunk of the data this thread is responsible
    unsigned int offset = i_global;
    __global CL_TYPE2* chunk;
    if(offset < n_fft) //thread id must be less than total number of 1d fft's
    {
        //invert place in list to obtain indices of block in array
        RowMajorIndexFromOffset(FFT_NDIM-1, offset, non_active_dimension_size, non_active_dimension_value, div_space);
        
        //copy the value of the non-active dimensions in to the index array
        for(unsigned int i=0; i<FFT_NDIM-1; i++)
        {
            index[ non_active_dimension_index[i] ] = non_active_dimension_value[i];
        }
        index[D] = 0; //for the selected dimension, index value is zero

        unsigned int data_location = OffsetFromRowMajorIndex(FFT_NDIM, dim, index);
        unsigned int stride = StrideFromRowMajorIndex(FFT_NDIM, D, dim); //stride for this axis
        chunk = &( data[data_location] );

        PermuteArrayStrided(dim[D], stride, permutation_array, chunk);
        FFTRadixTwo_DITStrided(dim[D], stride, twiddle, chunk);
    }

}

#endif /* MHO_MultidimensionalFastFourierTransform_Defined_H */
