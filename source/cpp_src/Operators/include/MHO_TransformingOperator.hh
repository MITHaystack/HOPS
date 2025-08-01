#ifndef MHO_TransformingOperator_HH__
#define MHO_TransformingOperator_HH__

#include "MHO_Operator.hh"

#include <tuple>

namespace hops
{

/*!
 *@file MHO_TransformingOperator.hh
 *@class MHO_TransformingOperator
 *@author J. Barrett - barrettj@mit.edu
 *@date Fri Oct 15 12:58:01 2021 -0400
 *@brief Operator which changes one N-D array type into a different N-D array type
 */

/**
 * @brief Class MHO_TransformingOperator
 */
template< class XArgType1, class XArgType2 > class MHO_TransformingOperator: public MHO_Operator
{
    public:
        MHO_TransformingOperator()
        {
            std::get< 0 >(fArgs) = nullptr;
            std::get< 1 >(fArgs) = nullptr;
        };

        virtual ~MHO_TransformingOperator(){};

        /**
         * @brief Setter for args (operation transforms the original type into another)
         *
         * @param in Pointer to constant XArgType1
         * @param out Pointer to XArgType2
         * @note This is a virtual function.
         */
        virtual void SetArgs(const XArgType1* in, XArgType2* out) { fArgs = std::make_tuple(in, out); };

        /**
         * @brief Initializes the system by calling InitializeImpl with arguments from fArgs.
         *
         * @return bool indicating success/failure of initialization.
         * @note This is a virtual function.
         */
        virtual bool Initialize() override { return InitializeImpl(std::get< 0 >(fArgs), std::get< 1 >(fArgs)); }

        /**
         * @brief Executes transformation using provided arguments.
         *
         * @return bool indicating success/failure of execution.
         * @note This is a virtual function.
         */
        virtual bool Execute() override { return ExecuteImpl(std::get< 0 >(fArgs), std::get< 1 >(fArgs)); }

    protected:
        /**
         * @brief Initializes implementation using input and output arguments.
         *
         * @param in Input argument of type const XArgType1*
         * @param out Output argument of type XArgType2*
         * @return Boolean indicating success/failure of initialization.
         * @note This is a virtual function.
         */
        virtual bool InitializeImpl(const XArgType1* in, XArgType2* out) = 0;

        /**
         * @brief Executes an operation transforming input type to output type.
         *
         * @param in Input data of type XArgType1
         * @param out Output data of type XArgType2
         * @return True if execution is successful, false otherwise.
         * @note This is a virtual function.
         */
        virtual bool ExecuteImpl(const XArgType1* in, XArgType2* out) = 0;

        //place for args to be stored for derived class to pick them up/modify
        std::tuple< const XArgType1*, XArgType2* > fArgs;
};

} // namespace hops

#endif /*! __MHO_TransformingOperator_HH__ */
