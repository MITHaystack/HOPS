#ifndef MHO_MultidimensionalPaddedFastFourierTransform_HH__
#define MHO_MultidimensionalPaddedFastFourierTransform_HH__

#include <cstring>
#include <vector>
#include <bitset>
#include <set>

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
            fFlipped = false; //only relevant for end-padded x-forms
            fInitialized = false;
            fForward = true;
        };

        virtual ~MHO_MultidimensionalPaddedFastFourierTransform()
        {
            DealocateWorkspace();
        };

        //factor M by which the new array will be extended (original array, length N, new array length NM)
        virtual void SetPaddingFactor(std::size_t factor){fPaddingFactor = factor;};

        virtual void SetCenterPadded(){fCentered = true; fFlipped = false;}; //symmetric zero padding about the center of the array (preserves conj. sym.)
        virtual void SetEndPadded(){fCentered = false; fFlipped = false;}; //zero padding from end of signal out to end of the array
        virtual void SetReverseEndPadded(){fCentered = false; fFlipped = true;}; //place signal at end of array and zero pad out to start

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
                if(fAxesToXForm[i])
                {
                    if(fInputDimensionSize[i]*fPaddingFactor != fOutputDimensionSize[i])
                    {
                        fIsValid = false;
                        msg_error("operators", "failed to initialize zero-padded FFT, input/output dimension mismatch at index: "<<i<<"."<<eom);
                    }

                    //for now we only support an implementation for even lengths
                    //as described on p. 751 of "Understanding Digital Signal Processing" by R.G. Lyons
                    //odd-length implementations are possible, by not needed for now
                    if(fInputDimensionSize[i]%2 != 0)
                    {
                        fIsValid = false;
                        msg_error("operators", "zero-padded FFT is only supported for even length dimensions."<<eom);
                    }
                }
                else
                {
                    if(fInputDimensionSize[i] != fOutputDimensionSize[i])
                    {
                        fIsValid = false;
                        msg_error("operators", "failed to initialize zero-padded FFT, input/output dimension mismatch at index: "<<i<<"."<<eom);
                    }
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
                //XFloatType total_input_size = 1.0;
                size_t total_size = 1;
                for(size_t i=0; i<RANK; i++)
                {
                    //total_input_size *= fInputDimensionSize[i];
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
                    //zero padding is placed symmetrically in the center of transform
                    auto in_iter =  this->fInput->begin();
                    auto in_iter_end = this->fInput->end();

                    //loop over the input array, determining where it should be copied to in
                    //the output array. For each index which is at a mid-point, the value should
                    //be split in that dimensions (if the point is located at the the mid-point
                    //of Q dimensions it will be copied into 2^Q locations in the output).
                    while( in_iter != in_iter_end)
                    {
                        std::array<std::size_t, RANK> in_index;
                        std::vector< std::vector<size_t> > out_index;
                        out_index.resize(RANK);

                        in_index = in_iter.GetIndexObject();
                        for(size_t i=0; i<RANK; i++)
                        {
                            if(fAxesToXForm[i])
                            {
                                size_t N = fInputDimensionSize[i];
                                size_t M = fPaddingFactor;
                                if(in_index[i] < N/2)
                                {
                                    out_index[i].push_back(in_index[i]);
                                }
                                else if (in_index[i] == N/2)
                                {
                                    //split the middle point
                                    out_index[i].push_back(N/2);
                                    out_index[i].push_back(N*M - N/2);
                                }
                                else
                                {
                                    out_index[i].push_back( (N*M - N/2 + 1) + (in_index[i] - (N/2+1)) );
                                }
                            }
                            else
                            {
                                out_index[i].push_back(in_index[i]);
                            }
                        }

                        //determine the number of indices over which this input point is split
                        size_t npts = 1;
                        for(size_t i=0; i<RANK; i++){npts *= out_index[i].size();}

                        if(npts == 1) //no splits
                        {
                            size_t out[RANK];
                            for(size_t i=0; i<RANK; i++){out[i] = *(out_index[i].begin()); }
                            //copy the input data to the same 'index' location in the output array
                            size_t out_loc = MHO_NDArrayMath::OffsetFromRowMajorIndex<RANK>(fOutputDimensionSize, out);
                            (*(this->fOutput))[out_loc] = *in_iter;
                        }
                        else
                        {
                            //we need to compute the cartesian product
                            //over the list of split indices here, to get the 2^Q locations
                            //over which to split this point in the output array
                            //to do this we construct all of the bit masks for every possible index combination
                            std::set< size_t > index_bitsets;
                            std::bitset<RANK> mask;
                            for(size_t i=0; i<RANK; i++)
                            {
                                mask[i] = 1;
                                if(out_index[i].size() == 1){mask[i] = 0;}
                            }
                            //compute all 2^RANK possibilities with mask applied
                            size_t n_possible = MHO_NDArrayMath::PowerOfTwo<RANK>::value;
                            for(size_t i=0; i<n_possible; i++)
                            {
                                std::bitset<RANK> val(i);
                                val &= mask; //binary 'and' with the mask
                                index_bitsets.insert( val.to_ulong() );
                            }

                            //now loop over the bitsets, inserting a fraction of this point in each place
                            size_t out[RANK];
                            double norm = 1.0/(double)npts;
                            for(auto it = index_bitsets.begin(); it != index_bitsets.end(); it++)
                            {
                                std::bitset<RANK> bits(*it);
                                for(size_t i=0; i<RANK; i++){out[i] = out_index[i][bits[i]];}
                                size_t out_loc = MHO_NDArrayMath::OffsetFromRowMajorIndex<RANK>(fOutputDimensionSize, out);
                                (*(this->fOutput))[out_loc] = (*in_iter)*norm;
                            }
                        }
                        ++in_iter;
                    }

                }
                else
                {
                    if(!fFlipped)
                    {
                        //zero padding is placed at the end of the array
                        auto in_iter =  this->fInput->begin();
                        auto in_iter_end = this->fInput->end();
                        while( in_iter != in_iter_end)
                        {
                            //copy the input data to the same 'index' location in the output array
                            std::array<std::size_t, RANK> in_indices = in_iter.GetIndexObject();
                            size_t out_loc = MHO_NDArrayMath::OffsetFromRowMajorIndex<RANK>(fOutputDimensionSize, &(in_indices[0]) );
                            (*(this->fOutput))[out_loc] = *in_iter;
                            ++in_iter;
                        }
                    }
                    else
                    {
                        //zero padding is placed at the start of the array
                        auto in_iter =  this->fInput->begin();
                        auto in_iter_end = this->fInput->end();
                        std::array<std::size_t, RANK> in_index;
                        size_t out_index[RANK];
                        while( in_iter != in_iter_end)
                        {
                            //copy the input data to the flipped location in the output array
                            in_index = in_iter.GetIndexObject();
                            for(size_t i=0; i<RANK; i++)
                            {
                                if(fAxesToXForm[i])
                                {
                                    //The way were are setting the indexes here
                                    //(with n=0 mapping to N/2 is done for compatibility with norm_fx)
                                    //TODO FIXME...figure out why it is done this way
                                    if(in_index[i] != 0)
                                    {
                                        out_index[i] = (fOutputDimensionSize[i]) - in_index[i];
                                    }
                                    else{out_index[i] = fOutputDimensionSize[i]/2;}
                                }
                                else{out_index[i] = in_index[i];}
                            }
                            size_t out_loc = MHO_NDArrayMath::OffsetFromRowMajorIndex<RANK>(fOutputDimensionSize, out_index);
                            (*(this->fOutput))[out_loc] = *in_iter;
                            ++in_iter;
                        }
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

                            // //normalize the output array
                            // XFloatType norm = 1.0/total_input_size;
                            // for(size_t i=0; i<total_size; i++){ (*(this->fOutput))[i] *= norm;}


                        }
                    }
                }
                //successful
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
        bool fFlipped;
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
