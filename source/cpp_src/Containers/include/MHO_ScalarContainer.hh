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

#include "MHO_Serializable.hh"
#include "MHO_NDArrayWrapper.hh"

namespace hops
{

template< typename XValueType >
class MHO_ScalarContainer:
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

}//end of hops namespace

#endif /* end of include guard: MHO_ScalarContainer */
