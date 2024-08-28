
#ifndef MHO_NDFFTBluestein_kernel_Defined__
#define MHO_NDFFTBluestein_kernel_Defined__

#include "hopsocl_defines.h"
#include "MHO_ArrayMath.cl"
#include "MHO_ComplexUtils.cl"
#include "MHO_FastFourierTransformUtilities.cl"

//compile time constants
//FFT_NDIM

// #ifdef FFT_USE_CONST_MEM
// NDFFTBluesteinStage(
//     unsigned int D, //d = 0, 1, ...FFT_NDIM-1 specifies the dimension/axis selected to be transformed
//     __global const unsigned int* dim_arr, //sizes of the array in each dimension
//     __local CL_TYPE2* twiddle_scratch, //scratch space for the twiddle factor basis
//     __global CL_TYPE2* data, // the data to be transformed
//     __constant CL_TYPE2* scale,
//     __constant CL_TYPE2* circulant,
//     __global CL_TYPE2* workspace)
// #else


//TODO HOW DO WE DETERMINE THIS FOR OUR DEVICE??
#define MAX_CONCURRENT_WORKGROUPS 4

__kernel void
NDFFTBluesteinStage(
    unsigned int D, //d = 0, 1, ...FFT_NDIM-1 specifies the dimension/axis selected to be transformed
    unsigned int isForward, //0 -> is a backward FFT, 1 -> is a forward FFT
    __global const unsigned int* dim_arr, //sizes of the array in each dimension
    __local CL_TYPE2* twiddle_scratch, //scratch space for the twiddle factor basis
    __global CL_TYPE2* data, // the data to be transformed
    __global CL_TYPE2* scale,
    __global CL_TYPE2* circulant,
    __global CL_TYPE2* workspace)
{
    //get the index of the current work item in the global list
    unsigned int i_global = get_global_id(0);
    unsigned int i_local = get_local_id(0);
    unsigned int workgroup_size = get_local_size(0);
    unsigned int i_workgroup = i_global;//(i_global/workgroup_size)%MAX_CONCURRENT_WORKGROUPS;
    unsigned int dim[FFT_NDIM];
    for(unsigned int i=0;i<FFT_NDIM; i++){dim[i] = dim_arr[i];}

    //work-item specific space for the twiddle factor basis
    __local CL_TYPE2* twiddle_basis = &(twiddle_scratch[workgroup_size*get_local_id(0)]);
    unsigned int log2N = LogBaseTwo(dim[D]);
    ComputeTwiddleFactorBasis(log2N, twiddle_basis);
    unsigned int M = ComputeBluesteinArraySize(dim[D]);

    CL_TYPE direction = FFT_BACKWARD;
    if(isForward){direction = FFT_FORWARD;}

    //pointer to the work-item's data chunk
    __global CL_TYPE2* chunk;

    //pointer to the FFT workspace
    __global CL_TYPE2* work_chunk;
    // work_chunk = &(workspace[  M*(workgroup_size*i_workgroup + i_local) ] );
    work_chunk = &(workspace[i_global*M]);

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
    RowMajorIndexFromOffset(FFT_NDIM-1, i_global, na_dimension_size, na_dimension_value, div_space);
    index[D] = 0; //for the current selected dimension, the index value is always zero
    for(unsigned int i=0; i<FFT_NDIM-1; i++)
    {
        //copy the value of the non-active dimensions in to the index array
        index[ na_dimension_index[i] ] = na_dimension_value[i];
    }
    //calculate the total memory offset to the start of the work-item's data block
    data_location = OffsetFromRowMajorIndex(FFT_NDIM, dim, index);
    chunk = &( data[data_location] );

    if(i_global < n_fft) //thread id must be less than total number of 1d fft's
    {
        FFTBluestein(dim[D], M, stride, direction, twiddle_basis, chunk, scale, circulant, work_chunk);
    }

}

#endif /* MHO_NDFFTBluestein_kernel_Defined__ */
