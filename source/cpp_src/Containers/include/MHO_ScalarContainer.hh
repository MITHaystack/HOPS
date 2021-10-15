#ifndef MHO_ScalarContainer_HH__
#define MHO_ScalarContainer_HH__

/*
*File: MHO_ScalarContainer.hh
*Class: MHO_ScalarContainer
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date: 2020-05-15T20:22:06.227Z
*Description:
*/


#include <string>
#include <complex>

#include "MHO_Serializable.hh"
#include "MHO_NDArrayWrapper.hh"

namespace hops
{

class MHO_ScalarContainerBase{}; //only needed for SFINAE

template< typename XValueType >
class MHO_ScalarContainer:
    public MHO_ScalarContainerBase,
    public MHO_NDArrayWrapper< XValueType, 0>,
    virtual public MHO_Serializable
{
    public:
        MHO_ScalarContainer():
            MHO_NDArrayWrapper<XValueType, 0>()
        {};

        virtual ~MHO_ScalarContainer(){};

        virtual uint64_t GetSerializedSize() const override
        {
            uint64_t total_size = 0;
            total_size += sizeof(MHO_ClassVersion); //version
            total_size += sizeof(XValueType); //contents
            return total_size;
        }

        void SetValue(const XValueType& value){fData = value;};
        XValueType GetValue(){ return fData;};

        //have to make base class functions visible
        using MHO_NDArrayWrapper<XValueType,0>::SetData;
        using MHO_NDArrayWrapper<XValueType,0>::GetData;

        std::size_t GetSize() const {return 1;};

    protected:

        using MHO_NDArrayWrapper<XValueType,0>::fData;

        template<typename XStream> friend XStream& operator>>(XStream& s, MHO_ScalarContainer& aData)
        {
                MHO_ClassVersion vers;
                s >> vers;
                if( vers != aData.GetVersion() )
                {
                    MHO_ClassIdentity::ClassVersionErrorMsg(aData, vers);
                    //Flag this as an unknown object version so we can skip over this data
                    MHO_ObjectStreamState<XStream>::SetUnknown(s);
                }
                else
                {
                    s >> aData.fData;
                }
                return s;
        };

        template<typename XStream> friend XStream& operator<<(XStream& s, const MHO_ScalarContainer& aData)
        {
                s << aData.GetVersion();
                s << aData.fData;
                return s;
        };

};


////////////////////////////////////////////////////////////////////////////////
//using declarations for all basic 'plain-old-data' types
using MHO_ScalarBool = MHO_ScalarContainer<bool>;
using MHO_ScalarChar = MHO_ScalarContainer<char>;
using MHO_ScalarUChar = MHO_ScalarContainer<unsigned char>;
using MHO_ScalarShort = MHO_ScalarContainer<short>;
using MHO_ScalarUShort = MHO_ScalarContainer<unsigned short>;
using MHO_ScalarInt = MHO_ScalarContainer<int>;
using MHO_ScalarUInt = MHO_ScalarContainer<unsigned int>;
using MHO_ScalarLong = MHO_ScalarContainer<long>;
using MHO_ScalarULong = MHO_ScalarContainer<unsigned long>;
using MHO_ScalarLongLong = MHO_ScalarContainer<long long>;
using MHO_ScalarULongLong = MHO_ScalarContainer<unsigned long long>;
using MHO_ScalarFloat = MHO_ScalarContainer<float>;
using MHO_ScalarDouble = MHO_ScalarContainer<double>;
using MHO_ScalarLongDouble = MHO_ScalarContainer<long double>;
using MHO_ScalarComplexFloat = MHO_ScalarContainer< std::complex<float> >;
using MHO_ScalarComplexDouble = MHO_ScalarContainer< std::complex<double> >;
using MHO_ScalarComplexLongDouble = MHO_ScalarContainer< std::complex<long double> >;
using MHO_ScalarString = MHO_ScalarContainer< std::string >;

////////////////////////////////////////////////////////////////////////////////


}//end of hops namespace

#endif /* end of include guard: MHO_ScalarContainer */
