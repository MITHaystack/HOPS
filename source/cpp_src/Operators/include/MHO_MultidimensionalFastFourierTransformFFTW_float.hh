#ifndef MHO_MultidimensionalFastFourierTransformFFTW_float_HH__
#define MHO_MultidimensionalFastFourierTransformFFTW_float_HH__

#include <cstring>
#include <fftw3.h>

#include "MHO_Message.hh"
#include "MHO_NDArrayWrapper.hh"
#include "MHO_FastFourierTransform.hh"

int fftwf_alignment_of(float*) __attribute__((weak));  //avoids "no-args depending on template parameter error"

namespace hops
{

template<size_t RANK>
class MHO_MultidimensionalFastFourierTransformFFTW_float:
    public MHO_NDArrayOperator< MHO_NDArrayWrapper< std::complex<float>, RANK >,
                             MHO_NDArrayWrapper< std::complex<float>, RANK > >
{
    public:
        MHO_MultidimensionalFastFourierTransformFFTW_float()
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

        virtual ~MHO_MultidimensionalFastFourierTransformFFTW_float()
        {
            DealocateWorkspace();
            if(fInitialized)
            {
                fftwf_destroy_plan(fPlanForward);
                fftwf_destroy_plan(fPlanBackward);
                fftwf_destroy_plan(fPlanForwardInPlace);
                fftwf_destroy_plan(fPlanBackwardInPlace);
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
                if( ( fftwf_alignment_of( reinterpret_cast<float*>(this->fInput->GetData() ) ) ==
                      fftwf_alignment_of( reinterpret_cast<float*>(fInPtr) ) ) &&
                    ( fftwf_alignment_of( reinterpret_cast<float*>(this->fOutput->GetData() ) ) ==
                      fftwf_alignment_of( reinterpret_cast<float*>(fOutPtr) ) ) )
                {
                    if( this->fInput->GetData() != this->fOutput->GetData() )
                    {
                        //transform is out-of-place
                        if(fForward)
                        {
                            fftwf_execute_dft(fPlanForward,
                                         reinterpret_cast<fftwf_complex*>(this->fInput->GetData() ),
                                         reinterpret_cast<fftwf_complex*>(this->fOutput->GetData() ) );
                        }
                        else
                        {
                            fftwf_execute_dft(fPlanBackward,
                                         reinterpret_cast<fftwf_complex*>(this->fInput->GetData() ),
                                         reinterpret_cast<fftwf_complex*>(this->fOutput->GetData() ) );
                        }
                    }
                    else
                    {
                        //we have to execute an in-place transform
                        if(fForward)
                        {
                            fftwf_execute_dft(fPlanForwardInPlace,
                                         reinterpret_cast<fftwf_complex*>(this->fInput->GetData() ),
                                         reinterpret_cast<fftwf_complex*>(this->fOutput->GetData() ) );
                        }
                        else
                        {
                            fftwf_execute_dft(fPlanBackwardInPlace,
                                         reinterpret_cast<fftwf_complex*>(this->fInput->GetData() ),
                                         reinterpret_cast<fftwf_complex*>(this->fOutput->GetData() ) );
                        }
                    }
                }
                else
                {
                    //alignment doesn't match so we need to use memcpy
                    std::memcpy( fInPtr, this->fInput->GetData() , fTotalArraySize*sizeof(fftwf_complex) );
                    if(fForward)
                    {
                        fftwf_execute(fPlanForward);
                    }
                    else
                    {
                        fftwf_execute(fPlanBackward);
                    }
                    std::memcpy(this->fOutput->GetData(), fOutPtr, fTotalArraySize*sizeof(fftwf_complex) );
                }
                return true;
            }
            else
            {
                //error
                msg_error("math", "FFT input/output array dimensions are not valid or intialization failed. Aborting transform." << eom);
                return false;
            }
        }


    private:

        virtual void AllocateWorkspace()
        {
            fInPtr = fftwf_alloc_complex(fTotalArraySize);
            fOutPtr = fftwf_alloc_complex(fTotalArraySize);
            fInPlacePtr = fftwf_alloc_complex(fTotalArraySize);
        }

        virtual void DealocateWorkspace()
        {
            fftwf_free(fInPtr);
            fftwf_free(fOutPtr);
            fftwf_free(fInPlacePtr);
        }


        bool ConstructPlan()
        {
            if(fInPtr == NULL || fOutPtr == NULL || fInPlacePtr == NULL){return false;}

            int rank = RANK;
            //we force fftwf to only do one fft at a time
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

            fPlanForward = fftwf_plan_guru_dft(rank, fDims, howmany_rank, &fHowManyDims,
                                       fInPtr, fOutPtr, FFTW_FORWARD, FFTW_EXHAUSTIVE);

            fPlanBackward = fftwf_plan_guru_dft(rank, fDims, howmany_rank, &fHowManyDims,
                                       fInPtr, fOutPtr, FFTW_BACKWARD, FFTW_EXHAUSTIVE);

            fPlanForwardInPlace = fftwf_plan_guru_dft(rank, fDims, howmany_rank, &fHowManyDims,
                                       fInPlacePtr, fInPlacePtr, FFTW_FORWARD, FFTW_EXHAUSTIVE);

            fPlanBackwardInPlace = fftwf_plan_guru_dft(rank, fDims, howmany_rank, &fHowManyDims,
                                       fInPlacePtr, fInPlacePtr, FFTW_BACKWARD, FFTW_EXHAUSTIVE);

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

        fftwf_iodim fDims[RANK];
        fftwf_iodim fHowManyDims;
        fftwf_plan fPlanForward;
        fftwf_plan fPlanBackward;
        fftwf_plan fPlanForwardInPlace;
        fftwf_plan fPlanBackwardInPlace;
        fftwf_complex* fInPtr;
        fftwf_complex* fOutPtr;
        fftwf_complex* fInPlacePtr;

};


}

#endif /* MHO_MultidimensionalFastFourierTransformFFTW_float_H__ */
