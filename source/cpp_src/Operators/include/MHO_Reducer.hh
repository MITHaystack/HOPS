#ifndef MHO_Reducer_HH__
#define MHO_Reducer_HH__

#include <algorithm>

#include "MHO_Message.hh"
#include "MHO_NDArrayWrapper.hh"
#include "MHO_NDArrayOperator.hh"
#include "MHO_CompoundReductions.hh" //for operator type definitions

/*
*File: MHO_Reducer.hh
*Class: MHO_Reducer
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
class MHO_Reducer:
    public MHO_NDArrayOperator< MHO_NDArrayWrapper< XItemType, RANK>,
                                MHO_NDArrayWrapper< XItemType, RANK> >
{
    public:

        MHO_Reducer():
            fInitialized(false)
        {
            for(std::size_t i=0;i<RANK;i++){fAxesToReduce[i] = 0;}
        }

        virtual ~MHO_Reducer(){};

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

        virtual bool Execute() override
        {
            if(fInitialized)
            {
                std::size_t in_dim[RANK];
                std::size_t out_dim[RANK];
                this->fInput->GetDimensions(in_dim);
                this->fOutput->GetDimensions(out_dim);

                std::size_t offset;
                std::size_t in_loc[RANK];
                std::size_t out_loc[RANK];
                auto iter_begin = this->fInput->begin();
                auto iter_end = this->fInput->end();
                for(auto iter = iter_begin; iter != iter_end; ++iter)
                {
                    //get the input indices for each dimension
                    offset = iter.GetOffset();
                    MHO_NDArrayMath::RowMajorIndexFromOffset<RANK>(offset, this->fInput->GetDimensions(), &(in_loc[0]) );

                    //set the output indices to collapse each index of the dimensions under reduction
                    for(std::size_t i=0; i<RANK; i++){out_loc[i] = std::min(in_loc[i], out_dim[i]-1);}
                    //find offset to location in output array
                    std::size_t m = MHO_NDArrayMath::OffsetFromRowMajorIndex<RANK>(out_dim, out_loc);
                    //execute the reduction operator +=  or *= or user-defined
                    fReductionOperator( (*(this->fOutput))[m], *iter);
                }

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


#endif /* MHO_Reducer_H__ */
