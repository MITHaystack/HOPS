#ifndef MHO_MultidimensionalFastFourierTransform_HH__
#define MHO_MultidimensionalFastFourierTransform_HH__

#include <cstring>

#include "MHO_Message.hh"
#include "MHO_NDArrayWrapper.hh"
#include "MHO_FastFourierTransform.hh"

namespace hops
{

template< typename XFloatType, size_t RANK>
class MHO_MultidimensionalFastFourierTransform:
    public MHO_NDArrayOperator< MHO_NDArrayWrapper< std::complex<XFloatType>, RANK >,
                             MHO_NDArrayWrapper< std::complex<XFloatType>, RANK > >
{
    public:

        MHO_MultidimensionalFastFourierTransform()
        {
            for(size_t i=0; i<RANK; i++)
            {
                fDimensionSize[i] = 0;
                fAxesToXForm[i] = true;
                fWorkspaceWrapper[i] = NULL;
                fTransformCalculator[i] = NULL;
            }

            fIsValid = false;
            fInitialized = false;
            fForward = true;
        };

        virtual ~MHO_MultidimensionalFastFourierTransform()
        {
            DealocateWorkspace();
        };

        virtual void SetForward(){fForward = true;}
        virtual void SetBackward(){fForward = false;};

        //sometimes we may want to select/deselect particular dimensions of the x-form
        //default is to transform along every dimension, but that may not always be needed
        void SelectAllAxes(){for(std::size_t i=0; i<RANK; i++){fAxesToXForm[i] = true;}}
        void DeselectAllAxes(){for(std::size_t i=0; i<RANK; i++){fAxesToXForm[i] = false;}}
        void SelectAxis(std::size_t axis_index)
        {
            if(axis_index < RANK){fAxesToXForm[axis_index] = true;}
            else
            {
                msg_error("operators", "Cannot transform axis with index: " <<
                          axis_index << "for array with rank: " << RANK << eom);
            }
        }


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
                DealocateWorkspace();
                AllocateWorkspace();

                fInitialized = true;
            }
            return (fInitialized && fIsValid);
        }

        virtual bool ExecuteOperation() override
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
                    std::memcpy( (void*) this->fOutput->GetData(),
                                 (void*) this->fInput->GetData(),
                                 total_size*sizeof(std::complex<XFloatType>) );
                }

                size_t index[RANK];
                size_t non_active_dimension_size[RANK-1];
                size_t non_active_dimension_value[RANK-1];
                size_t non_active_dimension_index[RANK-1];

                //select the dimension on which to perform the FFT
                for(size_t d = 0; d < RANK; d++)
                {
                    if(fAxesToXForm[d])
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
                            MHO_NDArrayMath::RowMajorIndexFromOffset<RANK-1>(n, non_active_dimension_size, non_active_dimension_value);

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
                                data_location = MHO_NDArrayMath::OffsetFromRowMajorIndex<RANK>(fDimensionSize, index);
                                (*(fWorkspaceWrapper[d]))[i] = (*(this->fOutput))[data_location];
                            }

                            //compute the FFT of the row selected
                            fTransformCalculator[d]->ExecuteOperation();

                            //copy the row selected back
                            for(size_t i=0; i<fDimensionSize[d]; i++)
                            {
                                index[d] = i;
                                data_location = MHO_NDArrayMath::OffsetFromRowMajorIndex<RANK>(fDimensionSize, index);
                                (*(this->fOutput))[data_location] = (*(fWorkspaceWrapper[d]))[i];
                            }
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
                fWorkspaceWrapper[i] = new MHO_NDArrayWrapper< std::complex<XFloatType>, 1 >(fDimensionSize[i]);
                fTransformCalculator[i] = new MHO_FastFourierTransform<XFloatType>();
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

        bool fIsValid;
        bool fForward;
        bool fInitialized;

        size_t fDimensionSize[RANK];
        bool fAxesToXForm[RANK];

        MHO_FastFourierTransform<XFloatType>* fTransformCalculator[RANK];
        MHO_NDArrayWrapper<std::complex<XFloatType>, 1>* fWorkspaceWrapper[RANK];


};


}

#endif /* MHO_MultidimensionalFastFourierTransform_H__ */
