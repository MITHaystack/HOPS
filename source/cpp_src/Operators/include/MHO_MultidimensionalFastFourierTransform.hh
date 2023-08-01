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

// #ifdef HOPS_USE_FFTW3
//     #include "MHO_MultidimensionalFastFourierTransformFFTW.hh"
// #else
//     #include "MHO_FastFourierTransform.hh"
// #endif

#include "MHO_TableContainer.hh"

namespace hops
{

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
        {
            // for(size_t i=0; i<XArgType::rank::value; i++)
            // {
            //     fWorkspaceWrapper[i] = NULL;
            //     fTransformCalculator[i] = NULL;
            // }
        };

        virtual ~MHO_MultidimensionalFastFourierTransform()
        {
            DeallocateWorkspace();
        };


    protected:

        virtual bool InitializeInPlace(XArgType* in) override
        {
            if( in != nullptr ){this->fIsValid = true;}
            else{this->fIsValid = false;}

            if(this->fIsValid)
            {
                //check if the current transform sizes are the same as input
                bool need_to_resize = false;
                for(std::size_t i=0; i<XArgType::rank::value; i++)
                {
                    if(this->fDimensionSize[i] != in->GetDimension(i)){need_to_resize = true; break;}
                }
                if(need_to_resize)
                {
                    in->GetDimensions(this->fDimensionSize);
                    DeallocateWorkspace();
                    AllocateWorkspace();
                }
                this->fInitialized = true;
            }
            return (this->fInitialized && this->fIsValid);
        }

        virtual bool InitializeOutOfPlace(const XArgType* in, XArgType* out) override
        {
            if( in != nullptr && out != nullptr ){this->fIsValid = true;}
            else{this->fIsValid = false;}

            if(this->fIsValid)
            {
                //check if the arrays have the same dimensions
                if( !HaveSameDimensions(in, out) )
                {
                    //resize the output array to match input
                    out->Resize( in->GetDimensions() );
                }
                //check if the current transform sizes are the same as input
                bool need_to_resize = false;
                for(std::size_t i=0; i<XArgType::rank::value; i++)
                {
                    if(this->fDimensionSize[i] != in->GetDimension(i)){need_to_resize = true; break;}
                }
                if(need_to_resize)
                {
                    in->GetDimensions(this->fDimensionSize);
                    DeallocateWorkspace();
                    AllocateWorkspace();
                }
                this->fInitialized = true;
            }
            return (this->fInitialized && this->fIsValid);
        }



        virtual bool ExecuteInPlace(XArgType* in) override
        {
            if(this->fIsValid && this->fInitialized)
            {
                size_t total_size = 1;
                for(size_t i=0; i<XArgType::rank::value; i++)
                {
                    total_size *= this->fDimensionSize[i];
                    // if(this->fForward)
                    // {
                    //     fTransformCalculator[i]->SetForward();
                    // }
                    // else
                    // {
                    //     fTransformCalculator[i]->SetBackward();
                    // }
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
                            //copy the row selected by the other dimensions
                            // for(size_t i=0; i<this->fDimensionSize[d]; i++)
                            // {
                                index[d] = 0;//i;
                                data_location = MHO_NDArrayMath::OffsetFromRowMajorIndex<XArgType::rank::value>(this->fDimensionSize, index);
                                data = &( (in->GetData())[data_location] );
                                //(*(fWorkspaceWrapper[d]))[i] = (*(in))[data_location];
                            // }

                            //compute the FFT of the row selected
                            unsigned int stride = MHO_NDArrayMath::StrideFromRowMajorIndex<XArgType::rank::value>(d, this->fDimensionSize);
                            if( fW[d].IsRadix2() ){ FFTRadix2(data, fW[d], this->fForward, stride); }
                            else{ FFTBluestein(data, fW[d], this->fForward, stride); }


                            // fTransformCalculator[d]->Execute();

                            // //copy the row selected back
                            // for(size_t i=0; i<this->fDimensionSize[d]; i++)
                            // {
                            //     index[d] = i;
                            //     data_location = MHO_NDArrayMath::OffsetFromRowMajorIndex<XArgType::rank::value>(this->fDimensionSize, index);
                            //     (*(in))[data_location] = (*(fWorkspaceWrapper[d]))[i];
                            // }
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
            if(in != out)
            {
                out->Copy(*in);
            }

            return ExecuteInPlace(out);
        }

    private:

        virtual void AllocateWorkspace()
        {
            for(size_t i=0; i<XArgType::rank::value; i++)
            {
                fW[i].Resize( this->fDimensionSize[i] );
            }
                // fWorkspaceWrapper[i] = new MHO_NDArrayWrapper< complex_value_type, 1 >(this->fDimensionSize[i]);
                // 
                // #ifdef HOPS_USE_FFTW3
                //         fTransformCalculator[i] = new MHO_MultidimensionalFastFourierTransformFFTW< MHO_NDArrayWrapper< complex_value_type, 1 > >();
                // #else
                //         fTransformCalculator[i] = new MHO_FastFourierTransform<floating_point_value_type>();
                // #endif
                
                // fTransformCalculator[i]->SetArgs(fWorkspaceWrapper[i]);
                // fTransformCalculator[i]->Initialize();
            //}
        }

        virtual void DeallocateWorkspace()
        {
            // for(size_t i=0; i<XArgType::rank::value; i++)
            // {
            //     delete fWorkspaceWrapper[i]; fWorkspaceWrapper[i] = NULL;
            //     delete fTransformCalculator[i]; fTransformCalculator[i] = NULL;
            // }
        }

        // #ifdef HOPS_USE_FFTW3
        //     MHO_MultidimensionalFastFourierTransformFFTW< MHO_NDArrayWrapper< complex_value_type, 1 > >* fTransformCalculator[XArgType::rank::value];
        // #else
        //     MHO_FastFourierTransform<floating_point_value_type>* fTransformCalculator[XArgType::rank::value];
        // #endif

        // MHO_NDArrayWrapper< complex_value_type, 1>* fWorkspaceWrapper[XArgType::rank::value];

        MHO_FastFourierTransformWorkspace<floating_point_value_type> fW[XArgType::rank::value];
};


}

#endif /* MHO_MultidimensionalFastFourierTransform_H__ */
