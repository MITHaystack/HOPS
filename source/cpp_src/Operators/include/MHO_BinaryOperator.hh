#ifndef MHO_BinaryOperator_HH__
#define MHO_BinaryOperator_HH__

#include "MHO_ArgumentCarrier.hh"

#include <tuple>

namespace hops
{

/*!
 *@file MHO_BinaryOperator.hh
 *@class MHO_BinaryOperator
 *@author J. Barrett - barrettj@mit.edu
 *@date Fri Oct 15 12:58:01 2021 -0400
 *@brief An operator that takes two array types as input (XArgType1 and XArgType2)
 *       and writes to a single output type (XArgType3)
 */

template< class XArgType1, class XArgType2 = XArgType1, class XArgType3 = XArgType2 >
class MHO_BinaryOperator: public MHO_ArgumentCarrier< const XArgType1*, const XArgType2*, XArgType3* >
{
    public:
        MHO_BinaryOperator(){};
        virtual ~MHO_BinaryOperator(){};

        void SetArgs(const XArgType1* in1, const XArgType2* in2, XArgType3* out)
        {
            this->fArgs = std::make_tuple(in1, in2, out);
        }

        virtual bool Initialize() override
        {
            return this->Apply([this](const XArgType1* in1, const XArgType2* in2, XArgType3* out) {
                return InitializeImpl(in1, in2, out);
            });
        }

        virtual bool Execute() override
        {
            return this->Apply([this](const XArgType1* in1, const XArgType2* in2, XArgType3* out) {
                return ExecuteImpl(in1, in2, out);
            });
        }

    protected:
        virtual bool InitializeImpl(const XArgType1* /*in1*/, const XArgType2* /*in2*/, XArgType3* /*out*/)
        {
            return true;
        }
        virtual bool ExecuteImpl(const XArgType1* in1, const XArgType2* in2, XArgType3* out) = 0;
};

} // namespace hops

#endif /*! __MHO_BinaryOperator_HH__ */
