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
*@date
*@brief
*/

//only operates on a single array, input = ouput
template<class XArgType>
class MHO_UnaryOperator: public MHO_Operator
{
    public:

        MHO_UnaryOperator()
        {
            fInPlace = false;
            std::get<0>(fInPlaceArgs) = nullptr;
            std::get<0>(fOutOfPlaceArgs) = nullptr;
            std::get<1>(fOutOfPlaceArgs) = nullptr;
        };

        virtual ~MHO_UnaryOperator(){};

        virtual void SetArgs(XArgType* in)
        {
            fInPlace = true;
            fInPlaceArgs = std::make_tuple(in);
        }

        //out-of-place operation, result stored in out, input unmodified
        virtual void SetArgs(const XArgType* in, XArgType* out)
        {
            fInPlace = false;
            fOutOfPlaceArgs = std::make_tuple(in, out);
        };


        virtual bool Initialize() override
        {
            if(fInPlace){ return InitializeInPlace( std::get<0>(fInPlaceArgs) ); }
            else{ return InitializeOutOfPlace( std::get<0>(fOutOfPlaceArgs), std::get<1>(fOutOfPlaceArgs) ); }
        }

        virtual bool Execute() override
        {
            if(fInPlace){ return ExecuteInPlace( std::get<0>(fInPlaceArgs) ); }
            else{ return ExecuteOutOfPlace( std::get<0>(fOutOfPlaceArgs), std::get<1>(fOutOfPlaceArgs) ); }
        }

    protected:

        virtual bool InitializeInPlace(XArgType* in) = 0;
        virtual bool InitializeOutOfPlace(const XArgType* in, XArgType* out) = 0;

        virtual bool ExecuteInPlace(XArgType* in) = 0;
        virtual bool ExecuteOutOfPlace(const XArgType* in, XArgType* out) = 0;


        //place for args to be stored for derived class to pick them up/modify
        bool fInPlace;
        std::tuple<XArgType*> fInPlaceArgs;
        std::tuple<const XArgType*, XArgType*> fOutOfPlaceArgs;

};


}//end of namespace

#endif /*! __MHO_UnaryOperator_HH__ */
