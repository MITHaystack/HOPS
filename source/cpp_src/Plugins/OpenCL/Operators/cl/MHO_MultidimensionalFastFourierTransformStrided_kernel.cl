#ifndef MHO_MultidimensionalFastFourierTransform_Defined_H
#define MHO_MultidimensionalFastFourierTransform_Defined_H

#include "hopsocl_defines.h"
#include "MHO_ArrayMath.cl"
#include "MHO_ComplexUtils.cl"
#include "MHO_FastFourierTransformUtilities.cl"

//compile time constants
//FFT_NDIM

__kernel void
MultidimensionalFastFourierTransformStrided_Radix2Stage(
    unsigned int D, //d = 0, 1, ...FFT_NDIM-1 specifies the dimension/axis selected to be transformed
    __global const unsigned int* dim_arr, //sizes of the array in each dimension
    __global CL_TYPE2* data, // the data to be transformed
    __local CL_TYPE2* twiddle_scratch //scratch space for the twiddle factor basis
)
{    
    //get the index of the current work item in the global list 
    unsigned int offset = get_global_id(0);
    unsigned int i_local = get_local_id(0);
    unsigned int workgroup_size = get_local_size(0);
    unsigned int dim[FFT_NDIM];
    for(unsigned int i=0;i<FFT_NDIM; i++){dim[i] = dim_arr[i];}
    
    //work-item specific space for the twiddle factor basis
    __local CL_TYPE2* twiddle_basis = &(twiddle_scratch[workgroup_size*get_local_id(0)]);
    unsigned int log2N = LogBaseTwo(dim[D]);
    ComputeTwiddleFactorBasis(log2N, twiddle_basis);

    //pointer to the work-item's data chunk
    __global CL_TYPE2* chunk;

    #ifdef USE_PRIVATE_MEM
    __private CL_TYPE2 buffer[4096];
    #endif

    //workspace for index calculations
    unsigned int index[FFT_NDIM];
    unsigned int div_space[FFT_NDIM-1];
    unsigned int na_dimension_size[FFT_NDIM-1];
    unsigned int na_dimension_value[FFT_NDIM-1];
    unsigned int na_dimension_index[FFT_NDIM-1];

    //figure out the total number of 1-D FFTs to perform and the stride along this axis
    //as well as the coordinates of this work item in the non-active dimensions
    unsigned int n_fft, stride, data_location;
    n_fft = CalculateWorkItemInfo(FFT_NDIM, D, dim, na_dimension_index, na_dimension_size);
    stride = StrideFromRowMajorIndex(FFT_NDIM, D, dim);

    //invert our location in work-item list to obtain indices of the block start in the global data array
    RowMajorIndexFromOffset(FFT_NDIM-1, offset, na_dimension_size, na_dimension_value, div_space);
    index[D] = 0; //for the current selected dimension, the index value is always zero
    for(unsigned int i=0; i<FFT_NDIM-1; i++)
    {
        //copy the value of the non-active dimensions in to the index array
        index[ na_dimension_index[i] ] = na_dimension_value[i]; 
    }
    //calculate the total memory offset to the start of the work-item's data block
    data_location = OffsetFromRowMajorIndex(FFT_NDIM, dim, index);
    chunk = &( data[data_location] );

    if(offset < n_fft) //thread id must be less than total number of 1d fft's
    {
        #ifndef USE_PRIVATE_MEM
        //perform the strided FFT in-place
        PermuteArrayStrided(dim[D], stride, chunk);
        FFTRadixTwo_DITStridedCached(dim[D], stride, twiddle_basis, chunk);

        #else

        for(unsigned int i=0; i<dim[D]; i++)
        {
            buffer[i] = chunk[i*stride];
        }
        
        //perform the strided FFT in-place
        PermuteArrayStrided(dim[D], 1, buffer);
        FFTRadixTwo_DITStridedCached(dim[D], 1, twiddle_basis, buffer);
        
        for(unsigned int i=0; i<dim[D]; i++)
        {
            chunk[i*stride] = buffer[i]; 
        }

        #endif
    }

}

#endif /* MHO_MultidimensionalFastFourierTransform_Defined_H */
