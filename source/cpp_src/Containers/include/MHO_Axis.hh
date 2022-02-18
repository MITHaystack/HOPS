#ifndef MHO_Axis_HH__
#define MHO_Axis_HH__

/*
*File: MHO_Axis.hh
*Class: MHO_Axis
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:
*/

#include "MHO_VectorContainer.hh"
#include "MHO_Interval.hh"
#include "MHO_IntervalLabel.hh"
#include "MHO_IntervalLabelTree.hh"

namespace hops
{

class MHO_AxisBase{};


template< typename XValueType >
class MHO_Axis:
    public MHO_AxisBase,
    public MHO_VectorContainer< XValueType >,
    public MHO_IntervalLabelTree
{

    public:
        MHO_Axis():
            MHO_VectorContainer<XValueType>(),
            MHO_IntervalLabelTree()
        {};


        MHO_Axis(std::size_t dim):
            MHO_VectorContainer< XValueType >(dim),
            MHO_IntervalLabelTree()
        {};

        //copy constructor
        MHO_Axis(const MHO_Axis& obj):
            MHO_VectorContainer< XValueType >(obj),
            MHO_IntervalLabelTree(obj)
        {};


        virtual ~MHO_Axis(){};

        virtual uint64_t GetSerializedSize() const override
        {
            uint64_t total_size = 0;
            total_size += sizeof(MHO_ClassVersion);
            total_size += MHO_VectorContainer< XValueType >::GetSerializedSize();
            total_size += MHO_IntervalLabelTree::GetSerializedSize();
            return total_size;
        }

        template<typename XStream> friend XStream& operator>>(XStream& s, MHO_Axis& aData)
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
                s >> static_cast< MHO_VectorContainer< XValueType >& >(aData);
                s >> static_cast< MHO_IntervalLabelTree& >(aData);
            }
            return s;
        }

        template<typename XStream> friend XStream& operator<<(XStream& s, const MHO_Axis& aData)
        {
            s << aData.GetVersion();
            s << static_cast< const MHO_VectorContainer< XValueType >& >(aData);
            s << static_cast< const MHO_IntervalLabelTree& >(aData);
            return s;
        }

    protected:

};

// ////////////////////////////////////////////////////////////////////////////////
// //using declarations for all basic 'plain-old-data' types (except bool!)
using MHO_AxisChar = MHO_Axis<char>;
using MHO_AxisUChar = MHO_Axis<unsigned char>;
using MHO_AxisShort = MHO_Axis<short>;
using MHO_AxisUShort = MHO_Axis<unsigned short>;
using MHO_AxisInt = MHO_Axis<int>;
using MHO_AxisUInt = MHO_Axis<unsigned int>;
using MHO_AxisLong = MHO_Axis<long>;
using MHO_AxisULong = MHO_Axis<unsigned long>;
using MHO_AxisLongLong = MHO_Axis<long long>;
using MHO_AxisULongLong = MHO_Axis<unsigned long long>;
using MHO_AxisFloat = MHO_Axis<float>;
using MHO_AxisDouble = MHO_Axis<double>;
using MHO_AxisLongDouble = MHO_Axis<long double>;
using MHO_AxisComplexFloat = MHO_Axis< std::complex<float> >;
using MHO_AxisComplexDouble = MHO_Axis< std::complex<double> >;
using MHO_AxisComplexLongDouble = MHO_Axis< std::complex<long double> >;
using MHO_AxisString = MHO_Axis< std::string >;
// ////////////////////////////////////////////////////////////////////////////////

}

#endif /* end of include guard: MHO_Axis */
