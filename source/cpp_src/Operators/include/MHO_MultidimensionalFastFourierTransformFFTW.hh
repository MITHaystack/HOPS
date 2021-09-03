#ifndef MHO_MultidimensionalFastFourierTransformFFTW_HH__
#define MHO_MultidimensionalFastFourierTransformFFTW_HH__

#include <cstring>
#include <fftw3.h>

#include "MHO_Message.hh"
#include "MHO_NDArrayWrapper.hh"
#include "MHO_FFTWTypes.hh"

namespace hops
{


template< typename XFloatType, size_t RANK>
class MHO_MultidimensionalFastFourierTransformFFTW:
    public MHO_NDArrayOperator< MHO_NDArrayWrapper< std::complex<XFloatType>, RANK >,
                             MHO_NDArrayWrapper< std::complex<XFloatType>, RANK > >
{
    public:

        MHO_MultidimensionalFastFourierTransformFFTW()
        {
            fTotalArraySize = 0;
            fInPtr = NULL;
            fOutPtr = NULL;
            fInPlacePtr = NULL;
            for(size_t i=0; i<RANK; i++)
            {
                fDimensionSize[i] = 0;
            }

            fIsValid = false;
            fInitialized = false;
            fForward = true;
        };

        virtual ~MHO_MultidimensionalFastFourierTransformFFTW()
        {
            DealocateWorkspace();
            if(fInitialized)
            {
                MHO_FFTWTypes<XFloatType>::destroy_plan_func(fPlanForward);
                MHO_FFTWTypes<XFloatType>::destroy_plan_func(fPlanBackward);
                MHO_FFTWTypes<XFloatType>::destroy_plan_func(fPlanForwardInPlace);
                MHO_FFTWTypes<XFloatType>::destroy_plan_func(fPlanBackwardInPlace);
            }
        };

        virtual void SetForward(){fForward = true;}
        virtual void SetBackward(){fForward = false;};

        virtual bool Initialize() override
        {
            if( HaveSameDimensions(this->fInput, this->fOutput) )
            {
                fIsValid = true;
                this->fInput->GetDimensions(fDimensionSize);
            }
            else
            {
                fIsValid = false;
            }

            if(!fInitialized && fIsValid)
            {
                fTotalArraySize = MHO_NDArrayMath::TotalArraySize<RANK>(fDimensionSize);
                AllocateWorkspace();
                bool success = ConstructPlan();
                fInitialized = success;
            }
            return (fInitialized && fIsValid);
        }

        virtual bool ExecuteOperation() override
        {
            if(fIsValid && fInitialized)
            {
                //check memory alignment to determine if we can avoid copying the data around
                if( ( MHO_FFTWTypes<XFloatType>::alignment_of_func( reinterpret_cast<XFloatType*>(this->fInput->GetData() ) ) ==
                      MHO_FFTWTypes<XFloatType>::alignment_of_func( reinterpret_cast<XFloatType*>(fInPtr) ) ) &&
                    ( MHO_FFTWTypes<XFloatType>::alignment_of_func( reinterpret_cast<XFloatType*>(this->fOutput->GetData() ) ) ==
                      MHO_FFTWTypes<XFloatType>::alignment_of_func( reinterpret_cast<XFloatType*>(fOutPtr) ) ) )
                {
                    if( this->fInput->GetData() != this->fOutput->GetData() )
                    {
                        //transform is out-of-place
                        if(fForward)
                        {
                            MHO_FFTWTypes<XFloatType>::execute_dft_func(fPlanForward,
                            reinterpret_cast<typename MHO_FFTWTypes<XFloatType>::fftw_complex_type_ptr>(this->fInput->GetData() ),
                            reinterpret_cast<typename MHO_FFTWTypes<XFloatType>::fftw_complex_type_ptr>(this->fOutput->GetData() ) );
                        }
                        else
                        {
                            MHO_FFTWTypes<XFloatType>::execute_dft_func(fPlanBackward,
                            reinterpret_cast<typename MHO_FFTWTypes<XFloatType>::fftw_complex_type_ptr>(this->fInput->GetData() ),
                            reinterpret_cast<typename MHO_FFTWTypes<XFloatType>::fftw_complex_type_ptr>(this->fOutput->GetData() ) );
                        }
                    }
                    else
                    {
                        //we have to execute an in-place transform
                        if(fForward)
                        {
                            MHO_FFTWTypes<XFloatType>::execute_dft_func(fPlanForwardInPlace,
                            reinterpret_cast<typename MHO_FFTWTypes<XFloatType>::fftw_complex_type_ptr>(this->fInput->GetData() ),
                            reinterpret_cast<typename MHO_FFTWTypes<XFloatType>::fftw_complex_type_ptr>(this->fOutput->GetData() ) );
                        }
                        else
                        {
                            MHO_FFTWTypes<XFloatType>::execute_dft_func(fPlanBackwardInPlace,
                            reinterpret_cast<typename MHO_FFTWTypes<XFloatType>::fftw_complex_type_ptr>(this->fInput->GetData() ),
                            reinterpret_cast<typename MHO_FFTWTypes<XFloatType>::fftw_complex_type_ptr>(this->fOutput->GetData() ) );
                        }
                    }
                }
                else
                {
                    //alignment doesn't match so we need to use memcpy
                    std::memcpy( fInPtr, this->fInput->GetData() , fTotalArraySize*sizeof(typename MHO_FFTWTypes<XFloatType>::fftw_complex_type) );
                    if(fForward)
                    {
                        MHO_FFTWTypes<XFloatType>::execute_func(fPlanForward);
                    }
                    else
                    {
                        MHO_FFTWTypes<XFloatType>::execute_func(fPlanBackward);
                    }
                    std::memcpy(this->fOutput->GetData(), fOutPtr, fTotalArraySize*sizeof(typename MHO_FFTWTypes<XFloatType>::fftw_complex_type) );
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
            fInPtr = MHO_FFTWTypes<XFloatType>::alloc_func(fTotalArraySize);
            fOutPtr = MHO_FFTWTypes<XFloatType>::alloc_func(fTotalArraySize);
            fInPlacePtr = MHO_FFTWTypes<XFloatType>::alloc_func(fTotalArraySize);
        }

        virtual void DealocateWorkspace()
        {
            MHO_FFTWTypes<XFloatType>::free_func(fInPtr);
            MHO_FFTWTypes<XFloatType>::free_func(fOutPtr);
            MHO_FFTWTypes<XFloatType>::free_func(fInPlacePtr);
        }


        bool ConstructPlan()
        {
            if(fInPtr == NULL || fOutPtr == NULL || fInPlacePtr == NULL){return false;}

            int rank = RANK;
            //we force fftw to only do one fft at a time
            //but we could implement a batched interface also...
            int howmany_rank = 0; //zero disables more than one x-form
            fHowManyDims.n = 1;
            fHowManyDims.is = MHO_NDArrayMath::TotalArraySize<RANK>(fDimensionSize);
            fHowManyDims.os = MHO_NDArrayMath::TotalArraySize<RANK>(fDimensionSize);

            for(size_t i=0; i<RANK; i++)
            {
                fDims[i].n = fDimensionSize[i];
                fDims[i].is = MHO_NDArrayMath::StrideFromRowMajorIndex<RANK>(i,fDimensionSize);
                fDims[i].os = MHO_NDArrayMath::StrideFromRowMajorIndex<RANK>(i,fDimensionSize);
            }

            fPlanForward = MHO_FFTWTypes<XFloatType>::plan_guru_func(rank, fDims, howmany_rank, &fHowManyDims,
                                       fInPtr, fOutPtr, FFTW_FORWARD, FFTW_MEASURE);

            fPlanBackward = MHO_FFTWTypes<XFloatType>::plan_guru_func(rank, fDims, howmany_rank, &fHowManyDims,
                                       fInPtr, fOutPtr, FFTW_BACKWARD, FFTW_MEASURE);

            fPlanForwardInPlace = MHO_FFTWTypes<XFloatType>::plan_guru_func(rank, fDims, howmany_rank, &fHowManyDims,
                                       fInPlacePtr, fInPlacePtr, FFTW_FORWARD, FFTW_MEASURE);

            fPlanBackwardInPlace = MHO_FFTWTypes<XFloatType>::plan_guru_func(rank, fDims, howmany_rank, &fHowManyDims,
                                       fInPlacePtr, fInPlacePtr, FFTW_BACKWARD, FFTW_MEASURE);

            if(fPlanForward != NULL && fPlanBackward != NULL && fPlanBackwardInPlace != NULL && fPlanForwardInPlace != NULL)
            {
                return true;
            }
            else
            {
                return false;
            }
        }

        bool fIsValid;
        bool fForward;
        bool fInitialized;

        size_t fTotalArraySize;
        size_t fDimensionSize[RANK];

        typename MHO_FFTWTypes<XFloatType>::iodim_type fDims[RANK];
        typename MHO_FFTWTypes<XFloatType>::iodim_type fHowManyDims;
        typename MHO_FFTWTypes<XFloatType>::plan_type fPlanForward;
        typename MHO_FFTWTypes<XFloatType>::plan_type fPlanBackward;
        typename MHO_FFTWTypes<XFloatType>::plan_type fPlanForwardInPlace;
        typename MHO_FFTWTypes<XFloatType>::plan_type fPlanBackwardInPlace;
        typename MHO_FFTWTypes<XFloatType>::fftw_complex_type_ptr fInPtr;
        typename MHO_FFTWTypes<XFloatType>::fftw_complex_type_ptr fOutPtr;
        typename MHO_FFTWTypes<XFloatType>::fftw_complex_type_ptr fInPlacePtr;
};


}

#endif /* MHO_MultidimensionalFastFourierTransformFFTW_H__ */
