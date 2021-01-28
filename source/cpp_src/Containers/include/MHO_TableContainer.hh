#ifndef MHO_TableContainer_HH__
#define MHO_TableContainer_HH__

/*
*File: MHO_TableContainer.hh
*Class: MHO_TableContainer
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date: 2020-05-15T20:22:00.867Z
*Description:
*/


#include <tuple>
#include <utility>
#include <string>

#include "MHO_Units.hh"
#include "MHO_MultiTypeMap.hh"
#include "MHO_NDArrayWrapper.hh"
#include "MHO_VectorContainer.hh"
#include "MHO_AxisPack.hh"


namespace hops
{

template< typename XValueType, typename XAxisPackType >
class MHO_TableContainer: public MHO_NDArrayWrapper< XValueType, XAxisPackType::NAXES::value>, public XAxisPackType, public MHO_Named
{
    public:

        MHO_TableContainer():
            MHO_NDArrayWrapper<XValueType, XAxisPackType::NAXES::value>(),
            XAxisPackType(),
            MHO_Named()
        {};

        MHO_TableContainer(const std::size_t* dim):
            MHO_NDArrayWrapper<XValueType, XAxisPackType::NAXES::value>(dim),
            XAxisPackType(dim),
            MHO_Named()
        {};

        //copy constructor
        MHO_TableContainer(const MHO_TableContainer& obj):
            MHO_NDArrayWrapper<XValueType, XAxisPackType::NAXES::value>(obj),
            XAxisPackType(obj),
            MHO_Named(obj)
        {};

        //clone functionality
        MHO_TableContainer* Clone(){ return new MHO_TableContainer(*this); }

        virtual ~MHO_TableContainer(){};

        using MHO_Named::IsNamed;
        using MHO_Named::GetName;
        using MHO_Named::SetName;



        //modify the Resize function to also resize the axes
        using XAxisPackType::resize_axis_pack;

        virtual void Resize(const std::size_t* dim) override
        {
            MHO_NDArrayWrapper< XValueType, XAxisPackType::NAXES::value>::Resize(dim);
            resize_axis_pack(dim);
        }

        //have to make base class functions visible
        using MHO_NDArrayWrapper<XValueType,XAxisPackType::NAXES::value>::Resize;
        using MHO_NDArrayWrapper<XValueType,XAxisPackType::NAXES::value>::GetData;
        using MHO_NDArrayWrapper<XValueType,XAxisPackType::NAXES::value>::GetSize;
        using MHO_NDArrayWrapper<XValueType,XAxisPackType::NAXES::value>::GetDimensions;
        using MHO_NDArrayWrapper<XValueType,XAxisPackType::NAXES::value>::GetDimension;
        using MHO_NDArrayWrapper<XValueType,XAxisPackType::NAXES::value>::GetOffsetForIndices;

        using MHO_NDArrayWrapper<XValueType,XAxisPackType::NAXES::value>::operator();
        using MHO_NDArrayWrapper<XValueType,XAxisPackType::NAXES::value>::operator[];

    protected:

        using MHO_NDArrayWrapper<XValueType,XAxisPackType::NAXES::value>::fData;
        using MHO_NDArrayWrapper<XValueType,XAxisPackType::NAXES::value>::fDimensions;
        using MHO_NDArrayWrapper<XValueType,XAxisPackType::NAXES::value>::fTotalArraySize;

    public:

        //temporary -- for testing
        MHO_MultiTypeMap< std::string, std::string, int, double > fTags;

};

}//end of namespace

#endif /* end of include guard: MHO_TableContainer_HH__ */
