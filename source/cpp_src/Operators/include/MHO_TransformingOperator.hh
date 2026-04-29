#ifndef MHO_TransformingOperator_HH__
#define MHO_TransformingOperator_HH__

#include "MHO_ArgumentCarrier.hh"

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

template< class XArgType1, class XArgType2 >
class MHO_TransformingOperator: public MHO_ArgumentCarrier< const XArgType1*, XArgType2* >
{
    public:
        MHO_TransformingOperator(){};
        virtual ~MHO_TransformingOperator(){};

        void SetArgs(const XArgType1* in, XArgType2* out)
        {
            this->fArgs = std::make_tuple(in, out);
        }

        virtual bool Initialize() override
        {
            return this->Apply(
                [this](const XArgType1* in, XArgType2* out) { return InitializeImpl(in, out); });
        }

        virtual bool Execute() override
        {
            return this->Apply(
                [this](const XArgType1* in, XArgType2* out) { return ExecuteImpl(in, out); });
        }

    protected:
        virtual bool InitializeImpl(const XArgType1* /*in*/, XArgType2* /*out*/) { return true; }
        virtual bool ExecuteImpl(const XArgType1* in, XArgType2* out) = 0;
};

} // namespace hops

#endif /*! __MHO_TransformingOperator_HH__ */
