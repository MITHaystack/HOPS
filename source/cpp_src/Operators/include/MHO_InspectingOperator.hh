#ifndef MHO_InspectingOperator_HH__
#define MHO_InspectingOperator_HH__

#include "MHO_Operator.hh"

namespace hops
{

/*!
 *@file MHO_InspectingOperator.hh
 *@class MHO_InspectingOperator
 *@author J. Barrett - barrettj@mit.edu
 *@date Tue Jan 10 16:42:53 2023 -0500
 *@brief
 */

//operates on a single const array (just to inspect it)
/**
 * @brief Class MHO_InspectingOperator
 */
template< class XArgType > class MHO_InspectingOperator: public MHO_Operator
{
    public:
        MHO_InspectingOperator(): fArg(nullptr){};
        virtual ~MHO_InspectingOperator(){};

        /**
         * @brief Setter for args
         * 
         * @param in Input const array of XArgType to inspect
         * @note This is a virtual function.
         */
        virtual void SetArgs(const XArgType* in) { fArg = in; }

        /**
         * @brief Inspects and initializes a single const array.
         * 
         * @return True if initialization is successful, false otherwise.
         * @note This is a virtual function.
         */
        virtual bool Initialize() override { return InitializeImpl(fArg); }

        /**
         * @brief Executes the DFT using the provided function and arguments.
         * 
         * @return bool indicating success/failure of execution.
         * @note This is a virtual function.
         */
        virtual bool Execute() override { return ExecuteImpl(fArg); }

    protected:
        /**
         * @brief Inspects a single const array using XArgType.
         * 
         * @param in Input const array of type XArgType to inspect
         * @return True if inspection is successful, false otherwise
         * @note This is a virtual function.
         */
        virtual bool InitializeImpl(const XArgType* in) = 0;
        /**
         * @brief Inspects and executes operations on a single const array.
         * 
         * @param in Input const array of type XArgType to inspect.
         * @return Boolean indicating successful execution.
         * @note This is a virtual function.
         */
        virtual bool ExecuteImpl(const XArgType* in) = 0;

        const XArgType* fArg;
};

} // namespace hops

#endif /*! __MHO_InspectingOperator_HH__ */
