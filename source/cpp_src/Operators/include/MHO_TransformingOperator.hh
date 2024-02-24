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
*@date
*@brief
*/

template<class XArgType1, class XArgType2>
class MHO_TransformingOperator: public MHO_Operator
{
    public:

        MHO_TransformingOperator()
        {
            std::get<0>(fArgs) = nullptr;
            std::get<1>(fArgs) = nullptr;
        };

        virtual ~MHO_TransformingOperator(){};

        //operation transforms the original type into another
        virtual void SetArgs(const XArgType1* in, XArgType2* out)
        {
            fArgs = std::make_tuple(in, out);
        };

        virtual bool Initialize() override
        {
            return InitializeImpl( std::get<0>(fArgs), std::get<1>(fArgs) );
        }

        virtual bool Execute() override
        {
            return ExecuteImpl( std::get<0>(fArgs), std::get<1>(fArgs) );
        }

    protected:

        virtual bool InitializeImpl(const XArgType1* in, XArgType2* out) = 0;
        virtual bool ExecuteImpl(const XArgType1* in, XArgType2* out) = 0;

        //place for args to be stored for derived class to pick them up/modify
        std::tuple<const XArgType1*, XArgType2*> fArgs;

};


}//end of namespace

#endif /*! __MHO_TransformingOperator_HH__ */
