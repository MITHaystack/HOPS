#ifndef MHOReducer_HH__
#define MHOReducer_HH__

#include <algorithm>

#include "MHOMessage.hh"
#include "MHOArrayWrapper.hh"
#include "MHOArrayOperator.hh"
#include "MHOCompoundReductions.hh" //for operator type definitions

/*
*File: MHOReducer.hh
*Class: MHOReducer
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description: Reduce a multi-dimensional array via a templated operation
*(e.g. summation) along the (runtime) specified dimensions. The output array has the same
dimensionality (i.e. RANK). However, the axes over which reduction occured
will have a size of 1. The output array will be resized if/as needed.
*/


namespace hops
{

template< typename XItemType, template<typename> class XOperatorType, std::size_t RANK>
class MHOReducer:
    public MHOArrayOperator< MHOArrayWrapper< XItemType, RANK>,
                             MHOArrayWrapper< XItemType, RANK> >
{
    public:

        MHOReducer():
            fInitialized(false)
        {
            for(std::size_t i=0;i<RANK;i++){fAxesToReduce[i] = 0;}
        }

        virtual ~MHOReducer(){};

        //set the indices of the axes over which we run the reduction.
        //This must be set before we can initialize/execute
        //for example for a 3D array, if we wanted to reduce along the
        //last axis only we would call this->ReduceAxis(2), or alternatively if we
        //wanted to reduce along both the first and last axis
        //we would call this->ReduceAxis(0), this->ReduceAxis(2)
        void
        ReduceAxis(std::size_t axis_index)
        {
            if(axis_index < RANK){fAxesToReduce[axis_index] = 1;}
            else
            {
                msg_error("operators", "Cannot reduce axis with index: " <<
                          axis_index << "for array with rank: " << RANK << eom);
            }
        }

        //de-select all axes
        void ClearAxisSelection()
        {
            for(std::size_t i=0; i<RANK; i++)
            {
                fAxesToReduce[i] = 0;
            }
        }

        virtual bool Initialize() override
        {
            fInitialized = false;
            if(this->fInput != nullptr && this->fOutput != nullptr)
            {
                std::size_t in_dim[RANK];
                std::size_t out_dim[RANK];
                std::size_t current_out_dim[RANK];
                this->fInput->GetDimensions(in_dim);
                this->fOutput->GetDimensions(current_out_dim);

                //first figure out what the remaining dim sizes are to be
                //after the array is contracted along the specified dimensions
                std::size_t count = 0;
                for(std::size_t i=0; i<RANK; i++)
                {
                    if(fAxesToReduce[i]){out_dim[i] = 1;}
                    else{out_dim[i] = in_dim[i];}
                }

                bool have_to_resize = false;
                for(std::size_t i=0; i<RANK; i++)
                {
                    if(out_dim[i] != current_out_dim[i]){have_to_resize = true; break;}
                }

                if(have_to_resize){this->fOutput->Resize(out_dim);}

                //must set the entire output array to the identity of the currently
                //templated
                this->fOutput->SetArray( this->fReductionOperator.identity );
                fInitialized = true;
            }
            return fInitialized;
        }

        virtual bool ExecuteOperation() override
        {
            if(fInitialized)
            {
                std::size_t in_dim[RANK];
                std::size_t out_dim[RANK];
                this->fInput->GetDimensions(in_dim);
                this->fOutput->GetDimensions(out_dim);

                const std::size_t* in_loc;
                std::size_t out_loc[RANK];
                std::size_t in_size = this->fInput->GetSize();

                auto iter_begin = this->fInput->begin();
                auto iter_end = this->fInput->end();
                for(auto iter = iter_begin; iter != iter_end; ++iter)
                {
                    in_loc = iter.GetIndices();
                    for(std::size_t i=0; i<RANK; i++){out_loc[i] = std::min(in_loc[i], out_dim[i]-1);}
                    std::size_t m = MHOArrayMath::OffsetFromRowMajorIndex<RANK>(out_dim, out_loc);
                    //execute the reduction operator +=  or *= or user-defined
                    fReductionOperator( (*(this->fOutput))[m], *iter);
                }

                // for(std::size_t n=0; n < in_size; n++)
                // {
                //     //while this is the most general method to contract the array
                //     //the following call to RowMajorIndexFromOffset is expensive,
                //     //and this algorithm probably doesn't have good cache locality
                //     //TODO - find a better way to do this, or provide optimized
                //     //versions for special cases (e.g. contract on single dimension,
                //     //or do the outermost dimension(s) only)
                //     MHOArrayMath::RowMajorIndexFromOffset<RANK>(n, in_dim, in_loc);
                //     for(std::size_t i=0; i<RANK; i++){out_loc[i] = std::min(in_loc[i], out_dim[i]-1);}
                //     std::size_t m = MHOArrayMath::OffsetFromRowMajorIndex<RANK>(out_dim, out_loc);
                //     //execute the reduction operator +=  or *= or user-defined
                //     fReductionOperator((*(this->fOutput))[m], (*(this->fInput))[n]);
                // }
                return true;
            }
            return false;
        }

    private:

        bool fInitialized;
        std::size_t fAxesToReduce[RANK];
        XOperatorType< XItemType > fReductionOperator;

};

}


#endif /* MHOReducer_H__ */
