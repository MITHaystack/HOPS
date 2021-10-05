#ifndef MHO_PointwiseAdd_HH__
#define MHO_PointwiseAdd_HH__

#include <algorithm>

#include "MHO_Message.hh"
#include "MHO_NDArrayWrapper.hh"
#include "MHO_BinaryNDArrayOperator.hh"
#include "MHO_NDArrayFunctor.hh"



/*
*File: MHO_PointwiseAdd.hh
*Class: MHO_PointwiseAdd
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:
*/


namespace hops
{

template<class XInputArrayType1, class XInputArrayType2, class XOutputArrayType>
class MHO_PointwiseAdd: public MHO_BinaryNDArrayOperator<XInputArrayType1, XInputArrayType2, XOutputArrayType >
{
    public:

        static_assert(XInputArrayType1::rank::value == XInputArrayType2::rank::value, "Input array ranks are not equal.");
        static_assert(XOutputArrayType::rank::value == XInputArrayType1::rank::value, "Input/Output array ranks are not equal.");

        MHO_PointwiseAdd():
            fInitialized(false)
        {};

        virtual ~MHO_PointwiseAdd(){};

        virtual bool Initialize() override
        {
            fInitialized = false;
            if(this->fInput1 != nullptr && this->fInput2 != nullptr && this->fOutput != nullptr)
            {
                //check that the total size of the two inputs are the same
                //we may want to more strictly check that each dimension is the same length
                if(this->fInput1->GetSize() == this->fInput2->GetSize() )
                {
                    //check that the output size is OK...resize if needed
                    if( (this->fInput1 != this->fOutput) )
                    {
                        std::size_t in_dim[XInputArrayType1::rank::value];
                        std::size_t out_dim[XOutputArrayType::rank::value];
                        this->fInput1->GetDimensions(in_dim);
                        this->fOutput->GetDimensions(out_dim);

                        bool have_to_resize = false;
                        for(std::size_t i=0; i<XInputArrayType1::rank::value; i++)
                        {
                            if(out_dim[i] != in_dim[i]){have_to_resize = true; break;}
                        }
                        if(have_to_resize){this->fOutput->Resize(in_dim);}
                    }
                    fInitialized = true;
                }
            }
            return fInitialized;
        }

        virtual bool Execute() override
        {
            //note: this implicitly assumes both input/output are the same total size

            //TODO FIXME...if input1 or input2 == output, then we need to change 
            //the way we iterate, otherwise we are double or triple iterating 
            //(see MHO_FunctorBroadcaster class)
            if(fInitialized)
            {
                auto in1_iter =  this->fInput1->begin();
                auto in1_iter_end = this->fInput1->end();
                auto in2_iter =  this->fInput2->begin();
                auto in2_iter_end = this->fInput2->end();
                auto out_iter = this->fOutput->begin();
                auto out_iter_end = this->fOutput->end();
                while( in1_iter != in1_iter_end && in2_iter != in2_iter_end && out_iter != out_iter_end)
                {

                    *out_iter = (*in1_iter) + (*in2_iter);
                    ++out_iter;
                    ++in1_iter;
                    ++in2_iter;
                }
                return true;
            }
            return false;
        }


    private:

        bool fInitialized;

};

}


#endif /* MHO_PointwiseAdd_H__ */
