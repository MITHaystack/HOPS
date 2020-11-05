#ifndef MHOMultidimensionalFastFourierTransform_HH__
#define MHOMultidimensionalFastFourierTransform_HH__

#include <cstring>

#include "MHOMessage.hh"
#include "MHOArrayWrapper.hh"
#include "MHOFastFourierTransform.hh"

namespace hops
{

template<size_t RANK>
class MHOMultidimensionalFastFourierTransform: public MHOUnaryArrayOperator< std::complex<double>, RANK, RANK >
{
    public:
        MHOMultidimensionalFastFourierTransform()
        {
            for(size_t i=0; i<RANK; i++)
            {
                fDimensionSize[i] = 0;
                fWorkspaceWrapper[i] = NULL;
                fTransformCalculator[i] = NULL;
            }

            fIsValid = false;
            fInitialized = false;
            fForward = true;
        };

        virtual ~MHOMultidimensionalFastFourierTransform()
        {
            DealocateWorkspace();
        };

        virtual void SetForward(){fForward = true;}
        virtual void SetBackward(){fForward = false;};

        virtual bool Initialize() override
        {
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
                DealocateWorkspace();
                AllocateWorkspace();

                fInitialized = true;
            }
            return (fInitialized && fIsValid);
        }

        virtual void ExecuteOperation() override
        {
            if(fIsValid && fInitialized)
            {
                size_t total_size = 1;
                for(size_t i=0; i<RANK; i++)
                {
                    total_size *= fDimensionSize[i];
                    if(fForward)
                    {
                        fTransformCalculator[i]->SetForward();
                    }
                    else
                    {
                        fTransformCalculator[i]->SetBackward();
                    }
                }

                //if input and output point to the same array, don't bother copying data over
                if(this->fInput != this->fOutput)
                {
                    //the arrays are not identical so copy the input over to the output
                    std::memcpy( (void*) this->fOutput->GetData(), (void*) this->fInput->GetData(), total_size*sizeof(std::complex<double>) );
                }

                size_t index[RANK];
                size_t non_active_dimension_size[RANK-1];
                size_t non_active_dimension_value[RANK-1];
                size_t non_active_dimension_index[RANK-1];

                //select the dimension on which to perform the FFT
                for(size_t d = 0; d < RANK; d++)
                {
                    //now we loop over all dimensions not specified by d
                    //first compute the number of FFTs to perform
                    size_t n_fft = 1;
                    size_t count = 0;
                    for(size_t i = 0; i < RANK; i++)
                    {
                        if(i != d)
                        {
                            n_fft *= fDimensionSize[i];
                            non_active_dimension_index[count] = i;
                            non_active_dimension_size[count] = fDimensionSize[i];
                            count++;
                        }
                    }

                    //loop over the number of FFTs to perform
                    for(size_t n=0; n<n_fft; n++)
                    {
                        //invert place in list to obtain indices of block in array
                        MHOArrayMath::RowMajorIndexFromOffset<RANK-1>(n, non_active_dimension_size, non_active_dimension_value);

                        //copy the value of the non-active dimensions in to index
                        for(size_t i=0; i<RANK-1; i++)
                        {
                            index[ non_active_dimension_index[i] ] = non_active_dimension_value[i];
                        }

                        size_t data_location;
                        //copy the row selected by the other dimensions
                        for(size_t i=0; i<fDimensionSize[d]; i++)
                        {
                            index[d] = i;
                            data_location = MHOArrayMath::OffsetFromRowMajorIndex<RANK>(fDimensionSize, index);
                            (*(fWorkspaceWrapper[d]))[i] = (*(this->fOutput))[data_location];
                        }

                        //compute the FFT of the row selected
                        fTransformCalculator[d]->ExecuteOperation();

                        //copy the row selected back
                        for(size_t i=0; i<fDimensionSize[d]; i++)
                        {
                            index[d] = i;
                            data_location = MHOArrayMath::OffsetFromRowMajorIndex<RANK>(fDimensionSize, index);
                            (*(this->fOutput))[data_location] = (*(fWorkspaceWrapper[d]))[i];
                        }
                    }
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


    private:

        virtual void AllocateWorkspace()
        {
            for(size_t i=0; i<RANK; i++)
            {
                fWorkspaceWrapper[i] = new MHOArrayWrapper< std::complex<double>, 1 >(fDimensionSize[i]);
                fTransformCalculator[i] = new MHOFastFourierTransform();
                fTransformCalculator[i]->SetSize(fDimensionSize[i]);
                fTransformCalculator[i]->SetInput(fWorkspaceWrapper[i]);
                fTransformCalculator[i]->SetOutput(fWorkspaceWrapper[i]);
                fTransformCalculator[i]->Initialize();
            }
        }

        virtual void DealocateWorkspace()
        {
            for(size_t i=0; i<RANK; i++)
            {
                delete fWorkspaceWrapper[i]; fWorkspaceWrapper[i] = NULL;
                delete fTransformCalculator[i]; fTransformCalculator[i] = NULL;
            }
        }

        virtual bool DoInputOutputDimensionsMatch()
        {
            size_t in[RANK];
            size_t out[RANK];

            this->fInput->GetDimensions(in);
            this->fOutput->GetDimensions(out);

            for(size_t i=0; i<RANK; i++)
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

        size_t fDimensionSize[RANK];

        MHOFastFourierTransform* fTransformCalculator[RANK];
        MHOArrayWrapper<std::complex<double>, 1>* fWorkspaceWrapper[RANK];


};


}

#endif /* MHOMultidimensionalFastFourierTransform_H__ */
