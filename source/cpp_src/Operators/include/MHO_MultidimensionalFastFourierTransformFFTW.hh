#ifndef MHO_MultidimensionalFastFourierTransformFFTW_HH__
#define MHO_MultidimensionalFastFourierTransformFFTW_HH__

#include <cstring>
#include <fftw3.h>

#include "MHO_Message.hh"
#include "MHO_NDArrayWrapper.hh"
#include "MHO_FastFourierTransform.hh"

int fftw_alignment_of(double*) __attribute__((weak));  //avoids "no-args depending on template parameter error"

namespace hops
{

template<size_t RANK>
class MHO_MultidimensionalFastFourierTransformFFTW:
    public MHO_NDArrayOperator< MHO_NDArrayWrapper< std::complex<double>, RANK >,
                             MHO_NDArrayWrapper< std::complex<double>, RANK > >
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
                fftw_destroy_plan(fPlanForward);
                fftw_destroy_plan(fPlanBackward);
                fftw_destroy_plan(fPlanForwardInPlace);
                fftw_destroy_plan(fPlanBackwardInPlace);
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
                if( ( fftw_alignment_of( reinterpret_cast<double*>(this->fInput->GetData() ) ) ==
                      fftw_alignment_of( reinterpret_cast<double*>(fInPtr) ) ) &&
                    ( fftw_alignment_of( reinterpret_cast<double*>(this->fOutput->GetData() ) ) ==
                      fftw_alignment_of( reinterpret_cast<double*>(fOutPtr) ) ) )
                {
                    if( this->fInput->GetData() != this->fOutput->GetData() )
                    {
                        //transform is out-of-place
                        if(fForward)
                        {
                            fftw_execute_dft(fPlanForward,
                                         reinterpret_cast<fftw_complex*>(this->fInput->GetData() ),
                                         reinterpret_cast<fftw_complex*>(this->fOutput->GetData() ) );
                        }
                        else
                        {
                            fftw_execute_dft(fPlanBackward,
                                         reinterpret_cast<fftw_complex*>(this->fInput->GetData() ),
                                         reinterpret_cast<fftw_complex*>(this->fOutput->GetData() ) );
                        }
                    }
                    else
                    {
                        //we have to execute an in-place transform
                        if(fForward)
                        {
                            fftw_execute_dft(fPlanForwardInPlace,
                                         reinterpret_cast<fftw_complex*>(this->fInput->GetData() ),
                                         reinterpret_cast<fftw_complex*>(this->fOutput->GetData() ) );
                        }
                        else
                        {
                            fftw_execute_dft(fPlanBackwardInPlace,
                                         reinterpret_cast<fftw_complex*>(this->fInput->GetData() ),
                                         reinterpret_cast<fftw_complex*>(this->fOutput->GetData() ) );
                        }
                    }
                }
                else
                {
                    //alignment doesn't match so we need to use memcpy
                    std::memcpy( fInPtr, this->fInput->GetData() , fTotalArraySize*sizeof(fftw_complex) );
                    if(fForward)
                    {
                        fftw_execute(fPlanForward);
                    }
                    else
                    {
                        fftw_execute(fPlanBackward);
                    }
                    std::memcpy(this->fOutput->GetData(), fOutPtr, fTotalArraySize*sizeof(fftw_complex) );
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
            fInPtr = fftw_alloc_complex(fTotalArraySize);
            fOutPtr = fftw_alloc_complex(fTotalArraySize);
            fInPlacePtr = fftw_alloc_complex(fTotalArraySize);
        }

        virtual void DealocateWorkspace()
        {
            fftw_free(fInPtr);
            fftw_free(fOutPtr);
            fftw_free(fInPlacePtr);
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

            fPlanForward = fftw_plan_guru_dft(rank, fDims, howmany_rank, &fHowManyDims,
                                       fInPtr, fOutPtr, FFTW_FORWARD, FFTW_EXHAUSTIVE);

            fPlanBackward = fftw_plan_guru_dft(rank, fDims, howmany_rank, &fHowManyDims,
                                       fInPtr, fOutPtr, FFTW_BACKWARD, FFTW_EXHAUSTIVE);

            fPlanForwardInPlace = fftw_plan_guru_dft(rank, fDims, howmany_rank, &fHowManyDims,
                                       fInPlacePtr, fInPlacePtr, FFTW_FORWARD, FFTW_EXHAUSTIVE);

            fPlanBackwardInPlace = fftw_plan_guru_dft(rank, fDims, howmany_rank, &fHowManyDims,
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

        fftw_iodim fDims[RANK];
        fftw_iodim fHowManyDims;
        fftw_plan fPlanForward;
        fftw_plan fPlanBackward;
        fftw_plan fPlanForwardInPlace;
        fftw_plan fPlanBackwardInPlace;
        fftw_complex* fInPtr;
        fftw_complex* fOutPtr;
        fftw_complex* fInPlacePtr;

};


}

#endif /* MHO_MultidimensionalFastFourierTransformFFTW_H__ */
