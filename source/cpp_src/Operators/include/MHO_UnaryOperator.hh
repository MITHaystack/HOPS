#ifndef MHO_UnaryOperator_HH__
#define MHO_UnaryOperator_HH__

#include "MHO_Operator.hh"

#include <tuple>

namespace hops
{

/*!
 *@file MHO_UnaryOperator.hh
 *@class MHO_UnaryOperator
 *@author J. Barrett - barrettj@mit.edu
 *@date Fri Oct 15 12:58:01 2021 -0400
 *@brief operator which only operates on a single array, input = ouput, but the output can be a different object
 */

//
/**
 * @brief Class MHO_UnaryOperator
 */
template< class XArgType > class MHO_UnaryOperator: public MHO_Operator
{
    public:
        MHO_UnaryOperator()
        {
            fInPlace = false;
            std::get< 0 >(fInPlaceArgs) = nullptr;
            std::get< 0 >(fOutOfPlaceArgs) = nullptr;
            std::get< 1 >(fOutOfPlaceArgs) = nullptr;
        };

        virtual ~MHO_UnaryOperator(){};

        /**
         * @brief Setter for args
         *
         * @param in Pointer to input array of type XArgType.
         */
        void SetArgs(XArgType* in)
        {
            fInPlace = true;
            fInPlaceArgs = std::make_tuple(in);
        }

        //out-of-place operation, result stored in out, input unmodified
        /**
         * @brief Setter for args
         *
         * @param in Input array of type XArgType* (const)
         * @param out (XArgType*)
         */
        void SetArgs(const XArgType* in, XArgType* out)
        {
            fInPlace = false;
            fOutOfPlaceArgs = std::make_tuple(in, out);
        };

        /**
         * @brief Initializes the system using in-place or out-of-place arguments.
         *
         * @return True if initialization succeeds, false otherwise.
         * @note This is a virtual function.
         */
        virtual bool Initialize() override
        {
            if(fInPlace)
            {
                return InitializeInPlace(std::get< 0 >(fInPlaceArgs));
            }
            else
            {
                return InitializeOutOfPlace(std::get< 0 >(fOutOfPlaceArgs), std::get< 1 >(fOutOfPlaceArgs));
            }
        }

        /**
         * @brief Executes operation using provided arguments and return type.
         *
         * @return bool indicating success of execution.
         * @note This is a virtual function.
         */
        virtual bool Execute() override
        {
            if(fInPlace)
            {
                return ExecuteInPlace(std::get< 0 >(fInPlaceArgs));
            }
            else
            {
                return ExecuteOutOfPlace(std::get< 0 >(fOutOfPlaceArgs), std::get< 1 >(fOutOfPlaceArgs));
            }
        }

    protected:
        virtual bool InitializeInPlace(XArgType* /*in*/) { return true; }
        virtual bool InitializeOutOfPlace(const XArgType* /*in*/, XArgType* /*out*/) { return true; }

        virtual bool ExecuteInPlace(XArgType* in) = 0;

        virtual bool ExecuteOutOfPlace(const XArgType* in, XArgType* out)
        {
            out->Copy(*in);
            return ExecuteInPlace(out);
        }

        //place for args to be stored for derived class to pick them up/modify
        bool fInPlace;
        std::tuple< XArgType* > fInPlaceArgs;
        std::tuple< const XArgType*, XArgType* > fOutOfPlaceArgs;
};

} // namespace hops

#endif /*! __MHO_UnaryOperator_HH__ */
