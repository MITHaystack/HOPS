#ifndef MHO_FunctorBroadcaster_HH__
#define MHO_FunctorBroadcaster_HH__

#include <algorithm>

#include "MHO_Message.hh"
#include "MHO_NDArrayWrapper.hh"
#include "MHO_UnaryOperator.hh"



/*!
*@file MHO_FunctorBroadcaster.hh
*@class MHO_FunctorBroadcaster
*@author J. Barrett - barrettj@mit.edu 
*
*@date
*@brief
*/


namespace hops
{

template< class XArrayType, class XFunctorType >
class MHO_FunctorBroadcaster: public MHO_UnaryOperator<XArrayType>
{
    public:

        MHO_FunctorBroadcaster(){fInitialized = false;}
        virtual ~MHO_FunctorBroadcaster(){};

        //access for configuration
        XFunctorType* GetFunctor(){return &fFunctor;};

    protected:

        virtual bool InitializeInPlace(XArrayType* in) override
        {
            if(in != nullptr){fInitialized = true;}
            return fInitialized;
        }

        virtual bool ExecuteInPlace(XArrayType* in) override
        {
            if(fInitialized)
            {
                //same array so only increment a single iter
                auto in_iter =  in->begin();
                auto in_iter_end = in->end();
                while( in_iter != in_iter_end)
                {
                    fFunctor(in_iter);
                    ++in_iter;
                }
                fInitialized = true;
            }
            return fInitialized;
        }

        virtual bool InitializeOutOfPlace(const XArrayType* in, XArrayType* out) override
        {
            if(in != nullptr && out != nullptr)
            {
                //only need to change output size if in != out and size is different
                if(in != out)
                {
                    std::size_t in_dim[XArrayType::rank::value];
                    std::size_t out_dim[XArrayType::rank::value];
                    in->GetDimensions(in_dim);
                    out->GetDimensions(out_dim);

                    bool have_to_resize = false;
                    for(std::size_t i=0; i<XArrayType::rank::value; i++)
                    {
                        if(out_dim[i] != in_dim[i]){have_to_resize = true; break;}
                    }

                    if(have_to_resize){out->Resize(in_dim);}
                }
                fInitialized = true;
            }
            return fInitialized;
        }

        virtual bool ExecuteOutOfPlace(const XArrayType* in, XArrayType* out) override
        {
            //note: this implicitly assumes both intput/output are the same total size
            if(fInitialized)
            {
                auto in_iter =  in->cbegin();
                auto in_iter_end = in->cend();
                auto out_iter = out->begin();
                auto out_iter_end = out->end();
                while( in_iter != in_iter_end && out_iter != out_iter_end)
                {
                    fFunctor(in_iter, out_iter);
                    ++out_iter;
                    ++in_iter;
                }
                return true;
            }
            return false;
        }

    private:

        bool fInitialized;
        XFunctorType fFunctor; //expected to inherit from MHO_UnaryFunctor<XArrayType>

};

}


#endif /*! MHO_FunctorBroadcaster_H__ */
