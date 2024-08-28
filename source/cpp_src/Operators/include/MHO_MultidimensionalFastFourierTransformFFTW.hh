#ifndef MHO_MultidimensionalFastFourierTransformFFTW_HH__
#define MHO_MultidimensionalFastFourierTransformFFTW_HH__

#include <cstring>
#include <fftw3.h>

#include "MHO_Message.hh"
#include "MHO_NDArrayWrapper.hh"
#include "MHO_UnaryOperator.hh"
#include "MHO_FFTWTypes.hh"


#define HOPS_FFTW_PLAN_ALGO FFTW_ESTIMATE
//could use FFTW_MEASURE instead


namespace hops
{


template< typename XArgType >
class MHO_MultidimensionalFastFourierTransformFFTW:
    public MHO_UnaryOperator< XArgType >
{
    public:

        static_assert( is_complex< typename XArgType::value_type >::value, "Array element type must be a complex floating point type." );

        using complex_value_type = typename XArgType::value_type;
        using floating_point_value_type = typename XArgType::value_type::value_type;

        MHO_MultidimensionalFastFourierTransformFFTW()
        {
            fTotalArraySize = 0;
            fInPtr = NULL;
            fOutPtr = NULL;
            fInPlacePtr = NULL;
            for(size_t i=0; i<XArgType::rank::value; i++)
            {
                fDimensionSize[i] = 0;
            }

            fIsValid = false;
            fInitialized = false;
            fForward = true;

            fPlanForward = NULL;
            fPlanBackward = NULL;
            fPlanForwardInPlace = NULL;
            fPlanBackwardInPlace = NULL;
        };

        virtual ~MHO_MultidimensionalFastFourierTransformFFTW()
        {
            DealocateWorkspace();
            DestructPlan();
        };

        virtual void SetForward(){fForward = true;}
        virtual void SetBackward(){fForward = false;};


    protected:

        virtual bool InitializeInPlace(XArgType* in) override
        {
            if( in != nullptr ){fIsValid = true;}
            else{fIsValid = false;}

            if(fIsValid)
            {
                //check if the current transform sizes are the same as input
                bool need_to_resize = false;
                for(std::size_t i=0; i<XArgType::rank::value; i++)
                {
                    if(fDimensionSize[i] != in->GetDimension(i)){need_to_resize = true; break;}
                }
                if(need_to_resize)
                {
                    in->GetDimensions(fDimensionSize);
                    fTotalArraySize = MHO_NDArrayMath::TotalArraySize<XArgType::rank::value>(fDimensionSize);
                    DealocateWorkspace();
                    AllocateWorkspace();
                    DestructPlan();
                    bool success = ConstructPlan();
                    fInitialized = success;
                }
                fInitialized = true;
            }
            return (fInitialized && fIsValid);
        }


        virtual bool InitializeOutOfPlace(const XArgType* in, XArgType* out) override
        {
            if( in != nullptr && out != nullptr ){fIsValid = true;}
            else{fIsValid = false;}
            if(fIsValid)
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
                    if(fDimensionSize[i] != in->GetDimension(i)){need_to_resize = true; break;}
                }
                if(need_to_resize)
                {
                    in->GetDimensions(fDimensionSize);
                    fTotalArraySize = MHO_NDArrayMath::TotalArraySize<XArgType::rank::value>(fDimensionSize);
                    DealocateWorkspace();
                    AllocateWorkspace();
                    DestructPlan();
                    bool success = ConstructPlan();
                    fInitialized = success;
                }
                fInitialized = true;
            }
            return (fInitialized && fIsValid);
        }



        virtual bool ExecuteInPlace(XArgType* in) override
        {
            if(fIsValid && fInitialized)
            {
                //check memory alignment to determine if we can avoid copying the data around
                if( HaveSameAlignment(in->GetData(), fInPtr) )
                {
                    //we have to execute an in-place transform
                    if(fForward)
                    {
                        MHO_FFTWTypes<floating_point_value_type>::execute_dft_func(fPlanForwardInPlace,
                        reinterpret_cast<typename MHO_FFTWTypes<floating_point_value_type>::fftw_complex_type_ptr>(in->GetData() ),
                        reinterpret_cast<typename MHO_FFTWTypes<floating_point_value_type>::fftw_complex_type_ptr>(in->GetData() ) );
                    }
                    else
                    {
                        MHO_FFTWTypes<floating_point_value_type>::execute_dft_func(fPlanBackwardInPlace,
                        reinterpret_cast<typename MHO_FFTWTypes<floating_point_value_type>::fftw_complex_type_ptr>(in->GetData() ),
                        reinterpret_cast<typename MHO_FFTWTypes<floating_point_value_type>::fftw_complex_type_ptr>(in->GetData() ) );
                    }
                }
                else
                {
                    //alignment doesn't match so we need to use memcpy
                    std::memcpy( fInPtr, in->GetData() , fTotalArraySize*sizeof(typename MHO_FFTWTypes<floating_point_value_type>::fftw_complex_type) );
                    if(fForward)
                    {
                        MHO_FFTWTypes<floating_point_value_type>::execute_func(fPlanForward);
                    }
                    else
                    {
                        MHO_FFTWTypes<floating_point_value_type>::execute_func(fPlanBackward);
                    }
                    std::memcpy(in->GetData(), fOutPtr, fTotalArraySize*sizeof(typename MHO_FFTWTypes<floating_point_value_type>::fftw_complex_type) );
                }
                return true;
            }
            else
            {
                //error
                msg_error("operators", "FFT input/output array dimensions are not valid or intialization failed. Aborting transform." << eom);
                return false;
            }
        }


        virtual bool ExecuteOutOfPlace(const XArgType* in, XArgType* out) override
        {
            if(fIsValid && fInitialized)
            {
                auto in_ptr = const_cast< complex_value_type* >( in->GetData() );// we promised not to modify it, so be careful
                //check memory alignment to determine if we can avoid copying the data around
                if( HaveSameAlignment( in_ptr, fInPtr) &&
                    HaveSameAlignment(out->GetData(), fOutPtr) )
                {
                    //transform is out-of-place
                    if(fForward)
                    {
                        MHO_FFTWTypes<floating_point_value_type>::execute_dft_func(fPlanForward,
                        reinterpret_cast<typename MHO_FFTWTypes<floating_point_value_type>::fftw_complex_type_ptr>( in_ptr ),
                        reinterpret_cast<typename MHO_FFTWTypes<floating_point_value_type>::fftw_complex_type_ptr>(out->GetData() ) );
                    }
                    else
                    {
                        MHO_FFTWTypes<floating_point_value_type>::execute_dft_func(fPlanBackward,
                        reinterpret_cast<typename MHO_FFTWTypes<floating_point_value_type>::fftw_complex_type_ptr>( in_ptr ),
                        reinterpret_cast<typename MHO_FFTWTypes<floating_point_value_type>::fftw_complex_type_ptr>(out->GetData() ) );
                    }
                }
                else
                {
                    //alignment doesn't match so we need to use memcpy
                    std::memcpy( fInPtr, in->GetData() , fTotalArraySize*sizeof(typename MHO_FFTWTypes<floating_point_value_type>::fftw_complex_type) );
                    if(fForward)
                    {
                        MHO_FFTWTypes<floating_point_value_type>::execute_func(fPlanForward);
                    }
                    else
                    {
                        MHO_FFTWTypes<floating_point_value_type>::execute_func(fPlanBackward);
                    }
                    std::memcpy(out->GetData(), fOutPtr, fTotalArraySize*sizeof(typename MHO_FFTWTypes<floating_point_value_type>::fftw_complex_type) );
                }
                return true;
            }
            else
            {
                //error
                msg_error("operators", "FFT input/output array dimensions are not valid or intialization failed. Aborting transform." << eom);
                return false;
            }
        }



    private:

        virtual void AllocateWorkspace()
        {
            fInPtr = MHO_FFTWTypes<floating_point_value_type>::alloc_func(fTotalArraySize);
            fOutPtr = MHO_FFTWTypes<floating_point_value_type>::alloc_func(fTotalArraySize);
            fInPlacePtr = MHO_FFTWTypes<floating_point_value_type>::alloc_func(fTotalArraySize);
        }

        virtual void DealocateWorkspace()
        {
            MHO_FFTWTypes<floating_point_value_type>::free_func(fInPtr);
            MHO_FFTWTypes<floating_point_value_type>::free_func(fOutPtr);
            MHO_FFTWTypes<floating_point_value_type>::free_func(fInPlacePtr);
        }


        void DestructPlan()
        {
            if(fInitialized)
            {
                MHO_FFTWTypes<floating_point_value_type>::destroy_plan_func(fPlanForward); fPlanForward = NULL;
                MHO_FFTWTypes<floating_point_value_type>::destroy_plan_func(fPlanBackward); fPlanBackward = NULL;
                MHO_FFTWTypes<floating_point_value_type>::destroy_plan_func(fPlanForwardInPlace); fPlanForwardInPlace = NULL;
                MHO_FFTWTypes<floating_point_value_type>::destroy_plan_func(fPlanBackwardInPlace); fPlanBackwardInPlace = NULL;
            }
        }

        bool ConstructPlan()
        {
            if(fInPtr == NULL || fOutPtr == NULL || fInPlacePtr == NULL){return false;}

            int rank = XArgType::rank::value;
            //we force fftw to only do one fft at a time
            //but we could implement a batched interface also...
            int howmany_rank = 0; //zero disables more than one x-form
            fHowManyDims.n = 1;
            fHowManyDims.is = MHO_NDArrayMath::TotalArraySize<XArgType::rank::value>(fDimensionSize);
            fHowManyDims.os = MHO_NDArrayMath::TotalArraySize<XArgType::rank::value>(fDimensionSize);

            for(size_t i=0; i<XArgType::rank::value; i++)
            {
                fDims[i].n = fDimensionSize[i];
                fDims[i].is = MHO_NDArrayMath::StrideFromRowMajorIndex<XArgType::rank::value>(i,fDimensionSize);
                fDims[i].os = MHO_NDArrayMath::StrideFromRowMajorIndex<XArgType::rank::value>(i,fDimensionSize);
            }

            fPlanForward = MHO_FFTWTypes<floating_point_value_type>::plan_guru_func(rank, fDims, howmany_rank, &fHowManyDims,
                                       fInPtr, fOutPtr, FFTW_FORWARD, HOPS_FFTW_PLAN_ALGO);

            fPlanBackward = MHO_FFTWTypes<floating_point_value_type>::plan_guru_func(rank, fDims, howmany_rank, &fHowManyDims,
                                       fInPtr, fOutPtr, FFTW_BACKWARD, HOPS_FFTW_PLAN_ALGO);

            fPlanForwardInPlace = MHO_FFTWTypes<floating_point_value_type>::plan_guru_func(rank, fDims, howmany_rank, &fHowManyDims,
                                       fInPlacePtr, fInPlacePtr, FFTW_FORWARD, HOPS_FFTW_PLAN_ALGO);

            fPlanBackwardInPlace = MHO_FFTWTypes<floating_point_value_type>::plan_guru_func(rank, fDims, howmany_rank, &fHowManyDims,
                                       fInPlacePtr, fInPlacePtr, FFTW_BACKWARD, HOPS_FFTW_PLAN_ALGO);

            if(fPlanForward != NULL && fPlanBackward != NULL && fPlanBackwardInPlace != NULL && fPlanForwardInPlace != NULL)
            {
                return true;
            }
            else
            {
                return false;
            }
        }

        template< typename XPtrType1, typename XPtrType2>
        bool HaveSameAlignment(XPtrType1 ptr1, XPtrType2 ptr2)
        {
            return ( MHO_FFTWTypes<floating_point_value_type>::alignment_of_func( reinterpret_cast<floating_point_value_type*>(ptr1) ) ==
                     MHO_FFTWTypes<floating_point_value_type>::alignment_of_func( reinterpret_cast<floating_point_value_type*>(ptr2) ) );
        }



        bool fIsValid;
        bool fForward;
        bool fInitialized;

        size_t fTotalArraySize;
        size_t fDimensionSize[XArgType::rank::value];

        typename MHO_FFTWTypes<floating_point_value_type>::iodim_type fDims[XArgType::rank::value];
        typename MHO_FFTWTypes<floating_point_value_type>::iodim_type fHowManyDims;
        typename MHO_FFTWTypes<floating_point_value_type>::plan_type fPlanForward;
        typename MHO_FFTWTypes<floating_point_value_type>::plan_type fPlanBackward;
        typename MHO_FFTWTypes<floating_point_value_type>::plan_type fPlanForwardInPlace;
        typename MHO_FFTWTypes<floating_point_value_type>::plan_type fPlanBackwardInPlace;
        typename MHO_FFTWTypes<floating_point_value_type>::fftw_complex_type_ptr fInPtr;
        typename MHO_FFTWTypes<floating_point_value_type>::fftw_complex_type_ptr fOutPtr;
        typename MHO_FFTWTypes<floating_point_value_type>::fftw_complex_type_ptr fInPlacePtr;
};


}

#endif /* MHO_MultidimensionalFastFourierTransformFFTW_H__ */
