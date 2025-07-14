#ifndef MHO_BinaryOperator_HH__
#define MHO_BinaryOperator_HH__

#include "MHO_Operator.hh"

#include <tuple>

namespace hops
{

/*!
 *@file MHO_BinaryOperator.hh
 *@class MHO_BinaryOperator
 *@author J. Barrett - barrettj@mit.edu
 *@date Fri Oct 15 12:58:01 2021 -0400
 *@brief an operator which takes to array types as input (XArgType1 and XArgType2), and writes to a single type as output (XArgType3)
 */

/**
 * @brief Class MHO_BinaryOperator
 */
template< class XArgType1, class XArgType2 = XArgType1, class XArgType3 = XArgType2 >
class MHO_BinaryOperator: public MHO_Operator
{
    public:
        MHO_BinaryOperator()
        {
            std::get< 0 >(fArgs) = nullptr;
            std::get< 1 >(fArgs) = nullptr;
            std::get< 2 >(fArgs) = nullptr;
        };

        virtual ~MHO_BinaryOperator(){};

        /**
         * @brief Setter for args, out-of-place operation, in1/in2 unmodified, result stored in out
         * 
         * @param in1 Input argument of type XArgType1
         * @param in2 Input argument of type XArgType2
         * @param out Output argument of type XArgType3
         * @note This is a virtual function.
         */
        virtual void SetArgs(const XArgType1* in1, const XArgType2* in2, XArgType3* out)
        {
            fArgs = std::make_tuple(in1, in2, out);
        };

        /**
         * @brief Initializes the object using arguments from fArgs tuple.
         * 
         * @return True if initialization succeeds, false otherwise.
         * @note This is a virtual function.
         */
        virtual bool Initialize() override
        {
            return InitializeImpl(std::get< 0 >(fArgs), std::get< 1 >(fArgs), std::get< 2 >(fArgs));
        }

        /**
         * @brief Executes operation using provided arguments and returns result.
         * 
         * @return bool indicating success/failure of execution.
         * @note This is a virtual function.
         */
        virtual bool Execute() override
        {
            return ExecuteImpl(std::get< 0 >(fArgs), std::get< 1 >(fArgs), std::get< 2 >(fArgs));
        }

    protected:
        // using type1 = XArgType1;
        // using type2 = XArgType2;
        // using type3 = XArgType3;

        /**
         * @brief Function InitializeImpl
         * 
         * @param !in1 input paratmer 1
         * @param !in2 input parameter 2
         * @param !out output parameter
         * @return Return value (bool)
         * @note This is a virtual function.
         */
        virtual bool InitializeImpl(const XArgType1* /*!in1*/, const XArgType2* /*!in2*/, XArgType3* /*!out*/) = 0;
        /**
         * @brief Function ExecuteImpl
         * 
         * @param !in1 input paratmer 1
         * @param !in2 input parameter 2
         * @param !out output parameter
         * @return Return value (bool)
         * @note This is a virtual function.
         */
        virtual bool ExecuteImpl(const XArgType1* /*!in1*/, const XArgType2* /*!in2*/, XArgType3* /*!out*/) = 0;

    protected:
        //place for args to be store for the derived class to pick them up/modify
        std::tuple< const XArgType1*, const XArgType2*, XArgType3* > fArgs;
};

} // namespace hops

#endif /*! __MHO_BinaryOperator_HH__ */
