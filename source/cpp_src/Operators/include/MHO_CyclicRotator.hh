#ifndef MHO_CyclicRotator_HH__
#define MHO_CyclicRotator_HH__

#include <algorithm>
#include <cstdint>

#include "MHO_Message.hh"
#include "MHO_NDArrayWrapper.hh"
#include "MHO_NDArrayOperator.hh"



/*
*File: MHO_CyclicRotator.hh
*Class: MHO_CyclicRotator
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description: 
* Applies a cyclic rotation on the contents on a multidimensional array 
* by some specified offset for each dimension.
*/


namespace hops
{

template< class XInputArrayType, class XOutputArrayType >
class MHO_CyclicRotator: public MHO_NDArrayOperator<XInputArrayType, XOutputArrayType >
{
    public:

        static_assert(XOutputArrayType::rank::value == XInputArrayType::rank::value, "Input/Output array ranks are not equal.");

        MHO_CyclicRotator():
            fInitialized(false)
        {
            for(std::size_t i=0; i<XInputArrayType::rank::value; i++){fOffsets[i]=0;}
        };

        virtual ~MHO_CyclicRotator(){};

        //set the offset for the cyclic rotation in each dimension (default is zero...do nothing)
        void SetOffset(std::size_t dimension_index, int64_t offset_value)
        {
            if(dimension_index < XInputArrayType::rank::value){fOffsets[dimension_index] = offset_value;}
            else 
            {
                msg_error("operators", "error, offset for dimension: "<<dimension_index<<" exceeds array rank." << eom);
            }
        }

        virtual bool Initialize() override
        {
            fInitialized = false;
            if(this->fInput != nullptr && this->fOutput != nullptr)
            {
                //only need to change output size if in != out and size is different
                if(this->fInput != this->fOutput)
                {
                    std::size_t in_dim[XInputArrayType::rank::value];
                    std::size_t out_dim[XOutputArrayType::rank::value];
                    this->fInput->GetDimensions(in_dim);
                    this->fOutput->GetDimensions(out_dim);

                    bool have_to_resize = false;
                    for(std::size_t i=0; i<XInputArrayType::rank::value; i++)
                    {
                        if(out_dim[i] != in_dim[i]){have_to_resize = true; break;}
                    }

                    if(have_to_resize){this->fOutput->Resize(in_dim);}
                    fInitialized = true;
                }
                else 
                {
                    msg_warn("operators","In-place cyclic rotation no currently supported." << eom );
                    fInitialized = false;
                }

            }
            return fInitialized;
        }

        virtual bool ExecuteOperation() override
        {
            //note: this implicitly assumes both intput/output are the same total size
            //but not the same array
            if(fInitialized)
            {
                if(this->fInput == this->fOutput) 
                {
                    //have to deal with things differently for an in-place rotation 
                    //TODO FIXME 
                    //probably one way to deal with this is to make use of std:rotate
                    //however to do that we would need to write a strided iterator class for MHO_NDArrayWrapper
                    //on second though...a strided iterator would be pretty handy
                    return false;
                }
                else 
                {
                    //first get the indices of the input iterator
                    auto in_iter =  this->fInput->begin();
                    auto in_iter_end = this->fInput->end();
                    const std::size_t* out_dim = this->fOutput->GetDimensions();
                    while( in_iter != in_iter_end)
                    {
                        const std::size_t* in_loc = in_iter.GetIndices();

                        for(std::size_t i=0; i<XInputArrayType::rank::value;i++)
                        {
                            fWorkspace[i] = positive_modulo( in_loc[i] + fOffsets[i], out_dim[i]);
                        }
                        std::size_t out_loc = this->fOutput->GetOffsetForIndices(fWorkspace);
                        (*(this->fOutput))[out_loc] = *in_iter;
                        in_iter++;
                    }

                    return true;
                }
            }
            return false;
        }


    private:

        bool fInitialized;

        //note: using the modulo is a painfully slow to do this
        //TODO FIXME...we ought to check for uint64_t -> int64_t overflows!
        inline int64_t positive_modulo(int64_t i, int64_t n)
        {
            //std::cout<<"i, n, mod = "<<i<<", "<<n<<", "<< (i % n + n) % n <<std::endl;
            return (i % n + n) % n;
        }

        //offsets to for cyclic rotation
        int64_t fOffsets[XInputArrayType::rank::value];
        std::size_t fWorkspace[XInputArrayType::rank::value];
};

};




#endif /* MHO_CyclicRotator_H__ */
