#ifndef MHO_ZeroPadder_HH__
#define MHO_ZeroPadder_HH__

#include <cstring>
#include <vector>
#include <bitset>
#include <set>

#include "MHO_Message.hh"
#include "MHO_NDArrayWrapper.hh"

namespace hops
{

template< typename XValueType, std::size_t RANK>
class MHO_ZeroPadder:
    public MHO_NDArrayOperator< MHO_NDArrayWrapper< XValueType, RANK >,
                                MHO_NDArrayWrapper< XValueType, RANK > >
{
    public:

        MHO_ZeroPadder()
        {
            fPaddingFactor = 1;
            for(size_t i=0; i<RANK; i++)
            {
                fInputDimensionSize[i] = 0;
                fOutputDimensionSize[i] = 0;
                fAxesToPad[i] = true;
            }

            fInitialized = false;
            fCentered = false;
            fFlipped = false; //only relevant for end-padded x-forms
        };

        virtual ~MHO_MultidimensionalPaddedFastFourierTransform(){};

        //factor M by which the new array will be extended (original array, length N, new array length NM)
        //this factor is the same in every dimension that is padded
        virtual void SetPaddingFactor(std::size_t factor){fPaddingFactor = factor;};

        virtual void SetCenterPadded(){fCentered = true; fFlipped = false;}; //symmetric zero padding about the center of the array (preserves conj. sym.)
        virtual void SetEndPadded(){fCentered = false; fFlipped = false;}; //zero padding from end of signal out to end of the array
        virtual void SetReverseEndPadded(){fCentered = false; fFlipped = true;}; //place signal at end of array and zero pad out to start

        //sometimes we may want to select/deselect particular dimensions of the x-form
        //default is to transform along every dimension, but that may not always be needed
        void SelectAllAxes(){for(std::size_t i=0; i<RANK; i++){fAxesToPad[i] = true;}}
        void DeselectAllAxes(){for(std::size_t i=0; i<RANK; i++){fAxesToPad[i] = false;}}
        void SelectAxis(std::size_t axis_index)
        {
            if(axis_index < RANK){fAxesToPad[axis_index] = true;}
            else
            {
                msg_error("operators", "Cannot pad axis with index: " <<
                          axis_index << "for array with rank: " << RANK << eom);
            }
        }


        virtual bool Initialize() override
        {
            //all output dimensions must be factor of fPaddingFactor bigger than input dims
            this->fInput->GetDimensions(fInputDimensionSize);
            this->fOutput->GetDimensions(fOutputDimensionSize);
            fInitialized = true;
            for(size_t i=0; i<RANK; i++)
            {
                if(fAxesToPad[i])
                {
                    if(fInputDimensionSize[i]*fPaddingFactor != fOutputDimensionSize[i])
                    {
                        fInitialized = false;
                        msg_error("operators", "failed to initialize zero-padded FFT, input/output dimension mismatch at index: "<<i<<"."<<eom);
                    }
                } 
                else 
                {
                    if(fInputDimensionSize[i] != fOutputDimensionSize[i])
                    {
                        fInitialized = false;
                        msg_error("operators", "failed to initialize zero-padded FFT, input/output dimension mismatch at index: "<<i<<"."<<eom);
                    }
                }
            }

            //make sure input and output are not the same arrays! 
            //we cannot do this in-place 
            if(this->fInput == this->fOutput)
            {
                fInitialized = false;
                msg_error("operators", "cannot execute a padded FFT in-place." << eom);
            }

            return fInitialized;
        }

        virtual bool ExecuteOperation() override
        {
            if(fInitialized)
            {
                size_t total_size = 1;
                for(size_t i=0; i<RANK; i++){total_size *= fOutputDimensionSize[i];}
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
                        const size_t* in_index;
                        std::vector< std::vector<size_t> > out_index;
                        out_index.resize(RANK);

                        in_index = in_iter.GetIndices();
                        for(size_t i=0; i<RANK; i++)
                        {
                            if(fAxesToPad[i])
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
                                    out_index[i].push_back( N*M - N/2 + (in_index[i] - N/2) );
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
                            size_t out_loc = MHO_NDArrayMath::OffsetFromRowMajorIndex<RANK>(fOutputDimensionSize, in_iter.GetIndices());
                            (*(this->fOutput))[out_loc] = *in_iter;
                            ++in_iter;
                        }
                    }
                    else 
                    {
                        //zero padding is placed at the start of the array
                        auto in_iter =  this->fInput->begin();
                        auto in_iter_end = this->fInput->end();
                        const size_t* in_index;
                        size_t out_index[RANK];
                        while( in_iter != in_iter_end)
                        {
                            //copy the input data to the flipped location in the output array 
                            in_index = in_iter.GetIndices()
                            for(size_t i=0; i<RANK; i++)
                            {
                                out_index[i] = in_index[i];
                                if(fAxesToPad[i])
                                {
                                    out_index[i] = (fOutputDimensionSize[i]-1) - in_index[i]; 
                                }
                            }
                            size_t out_loc = MHO_NDArrayMath::OffsetFromRowMajorIndex<RANK>(fOutputDimensionSize, out_index);
                            (*(this->fOutput))[out_loc] = *in_iter;
                            ++in_iter;
                        }
                    }
                }
            }
            else
            {
                //error
                msg_error("math", "zero-padding input/output array dimensions are not valid or intialization failed. Aborting padding." << eom);
                return false;
            }
        }


    private:

        bool fInitialized;
        bool fCentered;
        bool fFlipped;
        bool fInitialized;

        size_t fPaddingFactor;
        size_t fInputDimensionSize[RANK];
        size_t fOutputDimensionSize[RANK];
        bool fAxesToPad[RANK];

};


}

#endif /* MHO_ZeroPadder_H__ */
