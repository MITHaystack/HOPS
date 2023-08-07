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
    __constant const CL_TYPE2* twiddle, //fft twiddle factors
    __constant const unsigned int* permutation_array, //bit reversal permutation indices
    __global CL_TYPE2* data, // the data to be transformed
    __global CL_TYPE2* workspace // workspace 
)
{
    //get the index of the current thread
    unsigned int i_global = get_global_id(0);
    unsigned int i_local = get_local_id(0);
    unsigned int i_work = get_group_id(0);
    unsigned int n_group = get_num_groups(0);
    unsigned int group_size = get_local_size(0);

    __global CL_TYPE2* chunk;
    __global CL_TYPE2* buffer = &(workspace[(i_work*group_size+i_local)*array_dimensions[D]]);

    //assign a private variable the array dimensions
    unsigned int dim[FFT_NDIM];
    for(unsigned int i=0; i<FFT_NDIM; i++){dim[i] = array_dimensions[i];}

    unsigned int index[FFT_NDIM];
    unsigned int div_space[FFT_NDIM-1];
    unsigned int non_active_dimension_size[FFT_NDIM-1];
    unsigned int non_active_dimension_value[FFT_NDIM-1];
    unsigned int non_active_dimension_index[FFT_NDIM-1];

    //figure out the total number of 1-D FFTs to perform along this axis
    unsigned int n_fft = 1;
    unsigned int count = 0;
    for(unsigned int i = 0; i < FFT_NDIM; i++)
    {
        if(i != D)
        {
            n_fft *= dim[i];
            non_active_dimension_index[count] = i;
            non_active_dimension_size[count] = dim[i];
            count++;
        }
    }


    //figure out which chunk of the data this thread is responsible
    unsigned int offset = i_global;
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

        for(unsigned int i=0; i<dim[D]; i++)
        {
            buffer[i] = chunk[i*stride];
        }

        PermuteArray(dim[D], permutation_array, buffer);
        FFTRadixTwo_DIT(dim[D], twiddle, buffer);

        for(unsigned int i=0; i<dim[D]; i++)
        {
            chunk[i*stride] = buffer[i]; 
        }
    }

}

#endif /* MHO_MultidimensionalFastFourierTransform_Defined_H */