#ifndef MHOMultidimensionalFastFourierTransformFFTW_HH__
#define MHOMultidimensionalFastFourierTransformFFTW_HH__

#include <cstring>
#include <fftw3.h>

#include "MHOArrayWrapper.hh"
#include "MHOFastFourierTransform.hh"

int fftw_alignment_of(double*) __attribute__((weak));  // weak declaration must always be present

namespace hops
{

template<size_t NDIM>
class MHOMultidimensionalFastFourierTransformFFTW: public MHOUnaryArrayOperator< std::complex<double>, NDIM >
{
    public:
        MHOMultidimensionalFastFourierTransformFFTW()
        {
            fTotalArraySize = 0;
            fInPtr = NULL;
            fOutPtr = NULL;
            fInPlacePtr = NULL;
            for(size_t i=0; i<NDIM; i++)
            {
                fDimensionSize[i] = 0;
            }

            fIsValid = false;
            fInitialized = false;
            fForward = true;
        };

        virtual ~MHOMultidimensionalFastFourierTransformFFTW()
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

        virtual void Initialize()
        {
            std::cout<<"YES FFTW3"<<std::endl;
            if(DoInputOutputDimensionsMatch())
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
                fTotalArraySize = MHOArrayMath::TotalArraySize<NDIM>(fDimensionSize);
                AllocateWorkspace();
                bool success = ConstructPlan();
                fInitialized = success;
            }
        }

        virtual void ExecuteOperation()
        {
            if(fIsValid && fInitialized)
            {
                //check memory alignment to determine if we can avoid copying the data around
                if( ( fftw_alignment_of( reinterpret_cast<double*>(this->fInput->GetRawData() ) ) ==
                      fftw_alignment_of( reinterpret_cast<double*>(fInPtr) ) ) &&
                    ( fftw_alignment_of( reinterpret_cast<double*>(this->fOutput->GetRawData() ) ) ==
                      fftw_alignment_of( reinterpret_cast<double*>(fOutPtr) ) ) )
                {
                    if( this->fInput->GetRawData() != this->fOutput->GetRawData() )
                    {
                        //transform is out-of-place
                        if(fForward)
                        {
                            fftw_execute_dft(fPlanForward,
                                         reinterpret_cast<fftw_complex*>(this->fInput->GetRawData() ),
                                         reinterpret_cast<fftw_complex*>(this->fOutput->GetRawData() ) );
                        }
                        else
                        {
                            fftw_execute_dft(fPlanBackward,
                                         reinterpret_cast<fftw_complex*>(this->fInput->GetRawData() ),
                                         reinterpret_cast<fftw_complex*>(this->fOutput->GetRawData() ) );
                        }
                    }
                    else
                    {
                        //we have to execute an in-place transform
                        if(fForward)
                        {
                            fftw_execute_dft(fPlanForwardInPlace,
                                         reinterpret_cast<fftw_complex*>(this->fInput->GetRawData() ),
                                         reinterpret_cast<fftw_complex*>(this->fOutput->GetRawData() ) );
                        }
                        else
                        {
                            fftw_execute_dft(fPlanBackwardInPlace,
                                         reinterpret_cast<fftw_complex*>(this->fInput->GetRawData() ),
                                         reinterpret_cast<fftw_complex*>(this->fOutput->GetRawData() ) );
                        }
                    }
                }
                else
                {
                    //alignment doesn't match so we need to use memcpy
                    std::memcpy( fInPtr, this->fInput->GetRawData() , fTotalArraySize*sizeof(fftw_complex) );
                    if(fForward)
                    {
                        fftw_execute(fPlanForward);
                    }
                    else
                    {
                        fftw_execute(fPlanBackward);
                    }
                    std::memcpy(this->fOutput->GetRawData(), fOutPtr, fTotalArraySize*sizeof(fftw_complex) );
                }
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

            int rank = NDIM;
            //we force fftw to only do one fft at a time
            //but we could implement a batched interface also...
            int howmany_rank = 0; //zero disables more than one x-form
            fHowManyDims.n = 1;
            fHowManyDims.is = MHOArrayMath::TotalArraySize<NDIM>(fDimensionSize);
            fHowManyDims.os = MHOArrayMath::TotalArraySize<NDIM>(fDimensionSize);

            for(size_t i=0; i<NDIM; i++)
            {
                fDims[i].n = fDimensionSize[i];
                fDims[i].is = MHOArrayMath::StrideFromRowMajorIndex<NDIM>(i,fDimensionSize);
                fDims[i].os = MHOArrayMath::StrideFromRowMajorIndex<NDIM>(i,fDimensionSize);
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


        virtual bool DoInputOutputDimensionsMatch()
        {
            size_t in[NDIM];
            size_t out[NDIM];

            this->fInput->GetDimensions(in);
            this->fOutput->GetDimensions(out);

            for(size_t i=0; i<NDIM; i++)
            {
                if(in[i] != out[i])
                {
                    return false;
                }
            }
            return true;
        }

        bool fIsValid;
        bool fForward;
        bool fInitialized;

        size_t fTotalArraySize;
        size_t fDimensionSize[NDIM];

        fftw_iodim fDims[NDIM];
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

#endif /* MHOMultidimensionalFastFourierTransformFFTW_H__ */
