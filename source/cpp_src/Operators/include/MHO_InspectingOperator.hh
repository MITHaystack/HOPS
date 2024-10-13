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
template< class XArgType > class MHO_InspectingOperator: public MHO_Operator
{
    public:
        MHO_InspectingOperator(): fArg(nullptr){};
        virtual ~MHO_InspectingOperator(){};

        virtual void SetArgs(const XArgType* in) { fArg = in; }

        virtual bool Initialize() override { return InitializeImpl(fArg); }

        virtual bool Execute() override { return ExecuteImpl(fArg); }

    protected:
        virtual bool InitializeImpl(const XArgType* in) = 0;
        virtual bool ExecuteImpl(const XArgType* in) = 0;

        const XArgType* fArg;
};

} // namespace hops

#endif /*! __MHO_InspectingOperator_HH__ */
