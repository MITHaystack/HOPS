#ifndef MHO_FunctorBroadcaster_HH__
#define MHO_FunctorBroadcaster_HH__

#include <algorithm>

#include "MHO_Message.hh"
#include "MHO_NDArrayWrapper.hh"
#include "MHO_NDArrayOperator.hh"
#include "MHO_NDArrayFunctor.hh"



/*
*File: MHO_FunctorBroadcaster.hh
*Class: MHO_FunctorBroadcaster
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:
*/


namespace hops
{

template< class XInputArrayType, class XOutputArrayType >
class MHO_FunctorBroadcaster: public MHO_NDArrayOperator<XInputArrayType, XOutputArrayType >
{
    public:

        static_assert(XOutputArrayType::rank::value == XInputArrayType::rank::value, "Input/Output array ranks are not equal.");

        MHO_FunctorBroadcaster():
            fInitialized(false),
            fFunctor(nullptr)
        {};

        virtual ~MHO_FunctorBroadcaster(){};

        void SetFunctor( MHO_NDArrayFunctor<XInputArrayType, XOutputArrayType>* functor){fFunctor = functor;}
        MHO_NDArrayFunctor<XInputArrayType, XOutputArrayType>* GetFunctor() {return fFunctor;};

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
                }
                //need to have functor set up too
                if(fFunctor != nullptr)
                {
                    fInitialized = true;
                }
            }
            return fInitialized;
        }

        virtual bool ExecuteOperation() override
        {
            //note: this implicitly assumes both intput/output are the same total size
            if(fInitialized)
            {
                if(this->fInput == this->fOutput) 
                {
                    //same array so only increment a single iter
                    auto in_iter =  this->fInput->begin();
                    auto in_iter_end = this->fInput->end();
                    while( in_iter != in_iter_end)
                    {
                        (*fFunctor)(in_iter, in_iter);
                        ++in_iter;
                    }
                    return true;
                }
                else 
                {
                    auto in_iter =  this->fInput->begin();
                    auto in_iter_end = this->fInput->end();
                    auto out_iter = this->fOutput->begin();
                    auto out_iter_end = this->fOutput->end();
                    while( in_iter != in_iter_end && out_iter != out_iter_end)
                    {
                        (*fFunctor)(in_iter, out_iter);
                        ++out_iter;
                        ++in_iter;
                    }
                    return true;
                }
            }
            return false;
        }


    private:

        bool fInitialized;
        MHO_NDArrayFunctor<XInputArrayType, XOutputArrayType>* fFunctor;

};

}


#endif /* MHO_FunctorBroadcaster_H__ */
