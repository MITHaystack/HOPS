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

template< typename XValueType >
class MHO_Axis:
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

};

}

#endif /* end of include guard: MHO_Axis */
