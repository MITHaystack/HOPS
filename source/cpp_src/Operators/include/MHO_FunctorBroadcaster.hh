#ifndef MHO_FunctorBroadcaster_HH__
#define MHO_FunctorBroadcaster_HH__

#include <algorithm>

#include "MHO_Message.hh"
#include "MHO_NDArrayWrapper.hh"
#include "MHO_UnaryOperator.hh"

namespace hops
{

/*!
 *@file MHO_FunctorBroadcaster.hh
 *@class MHO_FunctorBroadcaster
 *@author J. Barrett - barrettj@mit.edu
 *@date Fri Nov 20 17:44:28 2020 -0500
 *@brief Applys a functor (XFunctorType) across all the elements of a multidimensional array
 */

/**
 * @brief Class MHO_FunctorBroadcaster
 */
template< class XArrayType, class XFunctorType > class MHO_FunctorBroadcaster: public MHO_UnaryOperator< XArrayType >
{
    public:
        MHO_FunctorBroadcaster() { fInitialized = false; }

        virtual ~MHO_FunctorBroadcaster(){};

        //access for configuration
        /**
         * @brief Getter for functor class object
         * 
         * @return Pointer to XFunctorType
         */
        XFunctorType* GetFunctor() { return &fFunctor; };

    protected:
        /**
         * @brief Initializes in-place operation flag if input is not nullptr.
         * 
         * @param in Input XArrayType pointer for initialization.
         * @return Current state of fInitialized boolean.
         * @note This is a virtual function.
         */
        virtual bool InitializeInPlace(XArrayType* in) override
        {
            if(in != nullptr)
            {
                fInitialized = true;
            }
            return fInitialized;
        }

        /**
         * @brief Applies functor to input array in-place if initialized.
         * 
         * @param in Input array of type XArrayType*
         * @return Boolean indicating whether processing was successful.
         * @note This is a virtual function.
         */
        virtual bool ExecuteInPlace(XArrayType* in) override
        {
            if(fInitialized)
            {
                //same array so only increment a single iter
                auto in_iter = in->begin();
                auto in_iter_end = in->end();
                while(in_iter != in_iter_end)
                {
                    fFunctor(in_iter);
                    ++in_iter;
                }
                fInitialized = true;
            }
            return fInitialized;
        }

        /**
         * @brief Function InitializeOutOfPlace - initialization for out-of-place transformation
         * 
         * @param in (const XArrayType*)
         * @param out (XArrayType*)
         * @return Return value (bool)
         * @note This is a virtual function.
         */
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
                    for(std::size_t i = 0; i < XArrayType::rank::value; i++)
                    {
                        if(out_dim[i] != in_dim[i])
                        {
                            have_to_resize = true;
                            break;
                        }
                    }

                    if(have_to_resize)
                    {
                        out->Resize(in_dim);
                    }
                }
                fInitialized = true;
            }
            return fInitialized;
        }

        /**
         * @brief Executes an out-of-place operation using a functor and input/output iterators.
         * 
         * @param in Const input array of type XArrayType
         * @param out Output array of type XArrayType
         * @return True if operation is successful, false otherwise.
         * @note This is a virtual function.
         */
        virtual bool ExecuteOutOfPlace(const XArrayType* in, XArrayType* out) override
        {
            //note: this implicitly assumes both intput/output are the same total size
            if(fInitialized)
            {
                auto in_iter = in->cbegin();
                auto in_iter_end = in->cend();
                auto out_iter = out->begin();
                auto out_iter_end = out->end();
                while(in_iter != in_iter_end && out_iter != out_iter_end)
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

} // namespace hops

#endif /*! MHO_FunctorBroadcaster_H__ */
