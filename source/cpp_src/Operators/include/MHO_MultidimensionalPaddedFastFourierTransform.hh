#ifndef MHO_MultidimensionalPaddedFastFourierTransform_HH__
#define MHO_MultidimensionalPaddedFastFourierTransform_HH__

#include <cstring>

#include "MHO_Message.hh"
#include "MHO_NDArrayWrapper.hh"
#include "MHO_FastFourierTransform.hh"

namespace hops
{

template< typename XFloatType, size_t RANK>
class MHO_MultidimensionalPaddedFastFourierTransform:
    public MHO_NDArrayOperator< MHO_NDArrayWrapper< std::complex<XFloatType>, RANK >,
                                MHO_NDArrayWrapper< std::complex<XFloatType>, RANK > >
{
    public:

        MHO_MultidimensionalPaddedFastFourierTransform()
        {
            fPaddingFactor = 1;
            for(size_t i=0; i<RANK; i++)
            {
                fInputDimensionSize[i] = 0;
                fOutputDimensionSize[i] = 0;
                fAxesToXForm[i] = true;
                fWorkspaceWrapper[i] = NULL;
                fTransformCalculator[i] = NULL;
            }

            fIsValid = false;
            fCentered = false;
            fInitialized = false;
            fForward = true;
        };

        virtual ~MHO_MultidimensionalPaddedFastFourierTransform()
        {
            DealocateWorkspace();
        };

        //factor M by which the new array will be extended (original array, length N, new array length NM)
        virtual void SetPaddingFactor(std::size_t factor){fPaddingFactor = factor;};

        virtual void SetCenterPadded(){fCentered = true;}; //symmetric zero padding about the center of the array (freq domain interp.)
        virtual void SetEndPadded(){fCentered = false;}; //zero padding form end of single out to end of the array (time domain interp.) 

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
            //all output dimensions must be factor of fPaddingFactor bigger than input dims
            this->fInput->GetDimensions(fInputDimensionSize);
            this->fOutput->GetDimensions(fOutputDimensionSize);
            fIsValid = true;
            for(size_t i=0; i<RANK; i++)
            {
                if(fInputDimensionSize[i]*fPaddingFactor != fOutputDimensionSize[i])
                {
                    fIsValid = false;
                    msg_error("operators", "failed to initialize zero-padded FFT, input/output dimesion mismatch at index: "<<i<<"."<<eom);
                }
            }

            //make sure input and output are not the same arrays! 
            //we cannot do this in-place 
            if(this->fInput == this->fOutput)
            {
                fIsValid = false;
                msg_error("operators", "cannot execute a padded FFT in-place." << eom);
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
                    total_size *= fOutputDimensionSize[i];
                    if(fForward)
                    {
                        fTransformCalculator[i]->SetForward();
                    }
                    else
                    {
                        fTransformCalculator[i]->SetBackward();
                    }
                }

                //zero out the output array
                for(size_t i=0; i<total_size; i++){ (*(this->fOutput))[i] = 0.0;}

                //Here we copy the input data over to the output data array. 
                //The way we do this depends on if this array needs to padded symmetrically (with the zeros in the center)
                //or if it is going to padded at the end (signal up front, zeros following)
                if(fCentered)
                {
                    msg_error("operators", "error this is not implemented."<<eom);
                }
                else 
                {
                    //zero padding is placed at the end of the transform 
                    auto in_iter =  this->fInput->begin();
                    auto in_iter_end = this->fInput->end();
                    while( in_iter != in_iter_end)
                    {
                        //copy the input data to the same 'index' location in the output array 
                        size_t out_loc = MHO_NDArrayMath::OffsetFromRowMajorIndex<RANK>(fOutputDimensionSize, in_iter.GetIndices());
                        (*(this->fOutput))[out_loc] = *in_iter;
                        ++in_iter;
                    }
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
                                n_fft *= fOutputDimensionSize[i];
                                non_active_dimension_index[count] = i;
                                non_active_dimension_size[count] = fOutputDimensionSize[i];
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
                            for(size_t i=0; i<fOutputDimensionSize[d]; i++)
                            {
                                index[d] = i;
                                data_location = MHO_NDArrayMath::OffsetFromRowMajorIndex<RANK>(fOutputDimensionSize, index);
                                (*(fWorkspaceWrapper[d]))[i] = (*(this->fOutput))[data_location];
                            }

                            //compute the FFT of the row selected
                            fTransformCalculator[d]->ExecuteOperation();

                            //copy the row selected back
                            for(size_t i=0; i<fOutputDimensionSize[d]; i++)
                            {
                                index[d] = i;
                                data_location = MHO_NDArrayMath::OffsetFromRowMajorIndex<RANK>(fOutputDimensionSize, index);
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
                fWorkspaceWrapper[i] = new MHO_NDArrayWrapper< std::complex<XFloatType>, 1 >(fOutputDimensionSize[i]);
                fTransformCalculator[i] = new MHO_FastFourierTransform<XFloatType>();
                fTransformCalculator[i]->SetSize(fOutputDimensionSize[i]);
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
        bool fCentered;
        bool fInitialized;

        size_t fPaddingFactor;
        size_t fInputDimensionSize[RANK];
        size_t fOutputDimensionSize[RANK];
        bool fAxesToXForm[RANK];

        MHO_FastFourierTransform<XFloatType>* fTransformCalculator[RANK];
        MHO_NDArrayWrapper<std::complex<XFloatType>, 1>* fWorkspaceWrapper[RANK];


};


}

#endif /* MHO_MultidimensionalPaddedFastFourierTransform_H__ */
