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
*@date
*@brief
*/

template<class XArgType1, class XArgType2 = XArgType1, class XArgType3 = XArgType2>
class MHO_BinaryOperator: public MHO_Operator
{
    public:

        MHO_BinaryOperator()
        {
            std::get<0>(fArgs) = nullptr;
            std::get<1>(fArgs) = nullptr;
            std::get<2>(fArgs) = nullptr;
        };

        virtual ~MHO_BinaryOperator(){};

        //out-of-place operation, in1/in2 unmodified, result stored in out
        virtual void SetArgs(const XArgType1* in1, const XArgType2* in2, XArgType3* out)
        {
            fArgs = std::make_tuple(in1, in2, out);
        };

        virtual bool Initialize() override
        {
            return InitializeImpl( std::get<0>(fArgs), std::get<1>(fArgs), std::get<2>(fArgs) );
        }

        virtual bool Execute() override
        {
            return ExecuteImpl( std::get<0>(fArgs), std::get<1>(fArgs), std::get<2>(fArgs) );
        }

    protected:

        // using type1 = XArgType1;
        // using type2 = XArgType2;
        // using type3 = XArgType3;

        virtual bool InitializeImpl(const XArgType1* /*!in1*/, const XArgType2* /*!in2*/, XArgType3* /*!out*/) = 0;
        virtual bool ExecuteImpl(const XArgType1* /*!in1*/, const XArgType2* /*!in2*/, XArgType3* /*!out*/) = 0;

    protected:

        //place for args to be store for derived class to pick them up/modify
        std::tuple<const XArgType1*, const XArgType2*, XArgType3*>  fArgs;
};


}


#endif /*! __MHO_BinaryOperator_HH__ */
