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
    __global const unsigned int* array_dimensions, //sizes of the array in each dimension
    __global const CL_TYPE2* twiddle, //fft twiddle factors
    __global const unsigned int* permutation_array, //bit reversal permutation indices
    __global CL_TYPE2* data // the data to be transformed (in-place)
)
{
    //get the index of the current thread
    unsigned int i_global = get_global_id(0);

    //assign a private variable the array dimensions
    unsigned int dim[FFT_NDIM];
    for(unsigned int i=0; i<FFT_NDIM; i++){dim[i] = array_dimensions[i];}

    size_t index[FFT_NDIM];
    size_t div_space[FFT_NDIM];
    size_t non_active_dimension_size[FFT_NDIM-1];
    size_t non_active_dimension_value[FFT_NDIM-1];
    size_t non_active_dimension_index[FFT_NDIM-1];

    //figure out the total number of 1-D FFTs to perform along this axis
    size_t n_fft = 1;
    size_t count = 0;
    for(size_t i = 0; i < FFT_NDIM; i++)
    {
        if(i != D)
        {
            n_fft *= dim[i];
            non_active_dimension_index[count] = i;
            non_active_dimension_size[count] = dim[i];
            count++;
        }
    }
    
    data[i_global] = n_fft;

    //figure out which chunk of the data this thread is responsible
    unsigned int offset = 0;
    unsigned int data_location = 0;
    unsigned int stride = StrideFromRowMajorIndex(FFT_NDIM, D, dim); //stride for this axis
    if(i_global < n_fft) //thread id must be less than total number of 1d fft's
    {
        offset = i_global;
        //invert place in list to obtain indices of block in array
        RowMajorIndexFromOffset(FFT_NDIM, offset, non_active_dimension_size, non_active_dimension_value, div_space);
        
        //copy the value of the non-active dimensions in to the index array
        for(size_t i=0; i<FFT_NDIM-1; i++)
        {
            index[ non_active_dimension_index[i] ] = non_active_dimension_value[i];
        }
        index[D] = 0; //for the selected dimension, index value is zero

        __global CL_TYPE2* chunk;
        data_location = OffsetFromRowMajorIndex( FFT_NDIM, dim, index);
        chunk = &( data[data_location] );

        // for(size_t j=0; j < dim[D]; j++)
        // {
        //     chunk[j*stride] = dim[1];
        // }
        chunk[i_global] = stride;

        //compute the FFT of the row selected
        // PermuteArray(dim[D], stride, permutation_array, chunk);
        // FFTRadixTwo_DIT(dim[D], stride, chunk, twiddle);
    }









    // //get the index of the current thread
    // unsigned int i_global = get_global_id(0);
    // 
    // //assign a private variable the array dimensions
    // unsigned int dim[FFT_NDIM];
    // for(unsigned int i=0; i<FFT_NDIM; i++){dim[i] = array_dimensions[i];}
    // 
    // size_t index[FFT_NDIM];
    // size_t div_space[FFT_NDIM];
    // size_t non_active_dimension_size[FFT_NDIM-1];
    // size_t non_active_dimension_value[FFT_NDIM-1];
    // size_t non_active_dimension_index[FFT_NDIM-1];
    // 
    // //figure out the total number of 1-D FFTs to perform along this axis
    // size_t n_fft = 1;
    // size_t count = 0;
    // for(size_t i = 0; i < FFT_NDIM; i++)
    // {
    //     if(i != D)
    //     {
    //         n_fft *= dim[i];
    //         non_active_dimension_index[count] = i;
    //         non_active_dimension_size[count] = dim[i];
    //         count++;
    //     }
    // }
    // 
    // //figure out which chunk of the data this thread is responsible
    // unsigned int offset = 0;
    // unsigned int data_location = 0;
    // unsigned int stride = StrideFromRowMajorIndex(FFT_NDIM, D, dim); //stride for this axis
    // if(i_global < n_fft) //thread id must be less than total number of 1d fft's
    // {
    //     offset = i_global;
    //     //invert place in list to obtain indices of block in array
    //     RowMajorIndexFromOffset(FFT_NDIM, offset, non_active_dimension_size, non_active_dimension_value, div_space);
    // 
    //     //copy the value of the non-active dimensions in to the index array
    //     for(size_t i=0; i<FFT_NDIM-1; i++)
    //     {
    //         index[ non_active_dimension_index[i] ] = non_active_dimension_value[i];
    //     }
    //     index[D] = 0; //for the selected dimension, index value is zero
    // 
    //     __global CL_TYPE2* chunk;
    //     data_location = OffsetFromRowMajorIndex( FFT_NDIM, dim, index);
    //     chunk = &( data[data_location] );
    // 
    //     // for(size_t j=0; j < dim[D]; j++)
    //     // {
    //     //     chunk[j*stride] = D;
    //     // }
    //     //compute the FFT of the row selected
    //     PermuteArray(dim[D], stride, permutation_array, chunk);
    //     FFTRadixTwo_DIT(dim[D], stride, chunk, twiddle);
    // }

}

#endif /* MHO_MultidimensionalFastFourierTransform_Defined_H */
