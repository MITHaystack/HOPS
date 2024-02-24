#ifndef MHO_MultidimensionalFastFourierTransform_HH__
#define MHO_MultidimensionalFastFourierTransform_HH__



#include <cstring>

#include "MHO_Meta.hh"
#include "MHO_Message.hh"
#include "MHO_NDArrayWrapper.hh"
#include "MHO_UnaryOperator.hh"

#include "MHO_FastFourierTransformUtilities.hh"
#include "MHO_FastFourierTransformWorkspace.hh"
#include "MHO_FastFourierTransformCalls.hh"

#include "MHO_MultidimensionalFastFourierTransformInterface.hh"
#include "MHO_TableContainer.hh"

namespace hops
{

/*!
*@file MHO_MultidimensionalFastFourierTransform.hh
*@class MHO_MultidimensionalFastFourierTransform
*@author J. Barrett - barrettj@mit.edu
*@date
*@brief
*/


template< typename XArgType >
class MHO_MultidimensionalFastFourierTransform:
    public MHO_UnaryOperator< XArgType >,
    public MHO_MultidimensionalFastFourierTransformInterface< XArgType >
{
    public:

        static_assert( is_complex< typename XArgType::value_type >::value, "Array element type must be a complex floating point type." );
        using complex_value_type = typename XArgType::value_type;
        using floating_point_value_type = typename XArgType::value_type::value_type;

        MHO_MultidimensionalFastFourierTransform():
            MHO_MultidimensionalFastFourierTransformInterface< XArgType >()
        {};

        virtual ~MHO_MultidimensionalFastFourierTransform(){};

    protected:

        virtual bool InitializeInPlace(XArgType* in) override
        {
            this->fInitialized = false;
            if( in == nullptr ){return false;}
            in->GetDimensions(this->fDimensionSize);
            AllocateWorkspace();
            this->fInitialized = true;
            #ifdef HOPS_ENABLE_DEBUG_MSG
            msg_debug("operators", "initialized a native FFT plan."<<eom);
            for(std::size_t i=0; i<XArgType::rank::value; i++)
            {
                msg_debug("operators", "fft plan dimension: "<<i<<" has size: "<<this->fDimensionSize[i]<<", enabled? "<<this->fAxesToXForm[i]<<"."<<eom);
            }
            #endif
            return true;
        }

        virtual bool InitializeOutOfPlace(const XArgType* in, XArgType* out) override
        {
            this->fInitialized = false;
            if( in == nullptr || out == nullptr ){return false;}
            if( !HaveSameDimensions(in, out) ){ out->Resize( in->GetDimensions() ); }
            in->GetDimensions(this->fDimensionSize);
            AllocateWorkspace();
            this->fInitialized = true;
            #ifdef HOPS_ENABLE_DEBUG_MSG
            msg_debug("operators", "initialized a native FFT plan."<<eom);
            for(std::size_t i=0; i<XArgType::rank::value; i++)
            {
                msg_debug("operators", "fft plan dimension: "<<i<<" has size: "<<this->fDimensionSize[i]<<", enabled? "<<this->fAxesToXForm[i]<<"."<<eom);
            }
            #endif
            return true;
        }

        virtual bool ExecuteInPlace(XArgType* in) override
        {
            if(this->fInitialized)
            {
                size_t total_size = 1;
                for(size_t i=0; i<XArgType::rank::value; i++)
                {
                    total_size *= this->fDimensionSize[i];
                }

                size_t index[XArgType::rank::value];
                size_t non_active_dimension_size[XArgType::rank::value-1];
                size_t non_active_dimension_value[XArgType::rank::value-1];
                size_t non_active_dimension_index[XArgType::rank::value-1];

                //select the dimension on which to perform the FFT
                for(size_t d = 0; d < XArgType::rank::value; d++)
                {
                    if(this->fAxesToXForm[d])
                    {
                        //now we loop over all dimensions not specified by d
                        //first compute the number of FFTs to perform
                        size_t n_fft = 1;
                        size_t count = 0;
                        for(size_t i = 0; i < XArgType::rank::value; i++)
                        {
                            if(i != d)
                            {
                                n_fft *= this->fDimensionSize[i];
                                non_active_dimension_index[count] = i;
                                non_active_dimension_size[count] = this->fDimensionSize[i];
                                count++;
                            }
                        }

                        //loop over the number of FFTs to perform
                        for(size_t n=0; n<n_fft; n++)
                        {
                            //invert place in list to obtain indices of block in array
                            MHO_NDArrayMath::RowMajorIndexFromOffset<XArgType::rank::value-1>(n, non_active_dimension_size, non_active_dimension_value);

                            //copy the value of the non-active dimensions in to index
                            for(size_t i=0; i<XArgType::rank::value-1; i++)
                            {
                                index[ non_active_dimension_index[i] ] = non_active_dimension_value[i];
                            }

                            size_t data_location;
                            complex_value_type* data;
                            index[d] = 0;
                            data_location = MHO_NDArrayMath::OffsetFromRowMajorIndex<XArgType::rank::value>(this->fDimensionSize, index);
                            data = &( (in->GetData())[data_location] );

                            //compute the FFT of the row selected
                            unsigned int stride = MHO_NDArrayMath::StrideFromRowMajorIndex<XArgType::rank::value>(d, this->fDimensionSize);
                            if( fW[d].IsRadix2() ){ FFTRadix2(data, fW[d], this->fForward, stride); }
                            else{ FFTBluestein(data, fW[d], this->fForward, stride); }
                        }
                    }

                    this->IfTableTransformAxis(in,d);
                }
                //successful
                return true;
            }
            else
            {
                //error
                msg_error("math", "FFT input/output array dimensions are not valid or intialization failed. Aborting transform." << eom);
                return false;
            }
        }


        virtual bool ExecuteOutOfPlace(const XArgType* in, XArgType* out) override
        {
            //if input and output point to the same array, don't bother copying data over
            if(in && out && in != out){out->Copy(*in);}
            return ExecuteInPlace(out);
        }

    private:

        void AllocateWorkspace()
        {
            for(size_t i=0; i<XArgType::rank::value; i++)
            {
                if(fW[i].fN != this->fDimensionSize[i]){ fW[i].Resize( this->fDimensionSize[i] ); }
            }
        }

        //workspace for transforms
        MHO_FastFourierTransformWorkspace<floating_point_value_type> fW[XArgType::rank::value];
};


}

#endif /*! MHO_MultidimensionalFastFourierTransform_H__ */
