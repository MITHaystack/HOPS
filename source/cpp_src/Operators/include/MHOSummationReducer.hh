#ifndef MHOSummationReducer_HH__
#define MHOSummationReducer_HH__

#include <array>

#include "MHOMessage.hh"
#include "MHOArrayWrapper.hh"
#include "MHOArrayOperator.hh"

/*
*File: MHOSummationReducer.hh
*Class: MHOSummationReducer
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description: Reduce the dimensionality of an array by the difference of
* INPUT_RANK-OUTPUT_RANK, via summation along the (runtime) specified dimensions
*/


namespace hops
{

template< typename XItemType, std::size_t INPUT_RANK, std::size_t OUTPUT_RANK>
class MHOSummationReducer:
    public MHOArrayOperator< MHOArrayWrapper< XItemType, INPUT_RANK>,
                             MHOArrayWrapper< XItemType, OUTPUT_RANK> >
{
    public:

        static_assert(OUTPUT_RANK < INPUT_RANK, "Error INPUT_RANK is no less than OUTPUT_RANK");

        typedef std::integral_constant< std::size_t, INPUT_RANK > input_rank;
        typedef std::integral_constant< std::size_t, OUTPUT_RANK > output_rank;
        typedef std::integral_constant< std::size_t, INPUT_RANK-OUTPUT_RANK > n_dims_to_reduce;

        MHOSummationReducer()
        {
            fDimsSet = false;
            fAllowOutputResize = false;
            fDimensionIndicesToReduce.fill(0);
        }

        virtual ~MHOSummationReducer(){};

        void AllowOutputResize(){fAllowOutputResize = true;}
        void DisallowOutputResize(){fAllowOutputResize = false;}

        //set the indices over which we run the reduction
        //must be set before we can initialize/execute
        template <typename ...XDimSizeTypeS >
        typename std::enable_if< (sizeof...(XDimSizeTypeS) == INPUT_RANK-OUTPUT_RANK), void >::type
        SetDimensionIndicesToReduce(XDimSizeTypeS...dim)
        {
            //compile-time check that the number of arguments is the same as the number of dimensions to reduce
            const std::array<std::size_t, INPUT_RANK-OUTPUT_RANK> dim_loc = {{static_cast<size_t>(dim)...}}; //convert the arguments to an array
            fDimensionIndicesToReduce = dim_loc;
            fDimsSet = true;
        }

        virtual bool Initialize() override
        {
            if(this->fInput != nullptr && this->fOutput != nullptr)
            {
                std::size_t in_dim[INPUT_RANK];
                std::size_t out_dim[OUTPUT_RANK];
                std::size_t expected_out_dim[OUTPUT_RANK];
                this->fInput->GetDimensions(in_dim);
                this->fOutput->GetDimensions(out_dim);

                //first figure out what the remaining dim sizes are to be
                //after the array is contracted along the specified dimensions
                std::size_t count = 0;
                for(std::size_t i=0; i<INPUT_RANK; i++)
                {
                    bool is_to_be_reduced = false;
                    for(auto it=fDimensionIndicesToReduce.begin(); it != fDimensionIndicesToReduce.end(); it++)
                    {
                        if( i == *it){is_to_be_reduced = true; break;}
                    }

                    if(!is_to_be_reduced)
                    {
                        expected_out_dim[count] = in_dim[i];
                        count++;
                    }
                }

                bool size_mismatch = false;
                for(std::size_t j=0; j<OUTPUT_RANK; j++)
                {
                    if(expected_out_dim[j] != out_dim[j])
                    {
                        size_mismatch = true;
                        break;
                    }
                }

                if(size_mismatch)
                {
                    if(fAllowOutputResize)
                    {
                        this->fOutput->Resize(expected_out_dim);
                        fInitialized = true;
                    }
                    else
                    {
                        msg_warn("operators", "The output array is not of the correct size, but will not be resized.");
                        fInitialized = false;
                    }
                }
                else
                {
                    fInitialized = true;
                }

                return fInitialized;

            }
        }

        virtual bool ExecuteOperation() override
        {
            if(fInitialized)
            {
                
            }
        }

    private:

        bool fDimsSet;
        bool fAllowOutputResize;
        bool fInitialized;
        std::array<std::size_t, INPUT_RANK-OUTPUT_RANK> fDimensionIndicesToReduce;

};

}


#endif /* MHOSummationReducer_H__ */
