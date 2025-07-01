#ifndef MHO_UnaryInPlaceOperator_HH__
#define MHO_UnaryInPlaceOperator_HH__

#include "MHO_Operator.hh"

#include <tuple>

namespace hops
{

/*!
 *@file MHO_UnaryInPlaceOperator.hh
 *@class MHO_UnaryInPlaceOperator
 *@author J. Barrett - barrettj@mit.edu
 *@date Mon Jul 31 14:54:52 2023 -0400
 *@brief
 */

//only operates on a single array, input = ouput
/**
 * @brief Class MHO_UnaryInPlaceOperator
 */
template< class XArgType > class MHO_UnaryInPlaceOperator: public MHO_Operator
{
    public:
        MHO_UnaryInPlaceOperator() { std::get< 0 >(fInPlaceArgs) = nullptr; };

        virtual ~MHO_UnaryInPlaceOperator(){};

        /**
         * @brief Setter for args
         * 
         * @param in Input pointer to XArgType array
         * @note This is a virtual function.
         */
        virtual void SetArgs(XArgType* in) { fInPlaceArgs = std::make_tuple(in); }

        /**
         * @brief Initializes the system by calling InitializeInPlace with the first argument from fInPlaceArgs.
         * 
         * @return bool indicating success/failure of initialization
         * @note This is a virtual function.
         */
        virtual bool Initialize() override { return InitializeInPlace(std::get< 0 >(fInPlaceArgs)); }

        /**
         * @brief Executes FFTW plan in-place using provided arguments.
         * 
         * @return bool indicating success of execution.
         * @note This is a virtual function.
         */
        virtual bool Execute() override { return ExecuteInPlace(std::get< 0 >(fInPlaceArgs)); }

    protected:
        /**
         * @brief Initializes in-place operation using input argument.
         * 
         * @param in Input argument of type XArgType* for initialization.
         * @return Boolean indicating success of initialization.
         * @note This is a virtual function.
         */
        virtual bool InitializeInPlace(XArgType* in) = 0;
        /**
         * @brief Executes an operation in-place using input arguments.
         * 
         * @param in Input argument for executing operation.
         * @return True if execution is successful, false otherwise.
         * @note This is a virtual function.
         */
        virtual bool ExecuteInPlace(XArgType* in) = 0;

        std::tuple< XArgType* > fInPlaceArgs;
};

} // namespace hops

#endif /*! __MHO_UnaryInPlaceOperator_HH__ */
