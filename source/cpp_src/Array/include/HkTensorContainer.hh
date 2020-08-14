#ifndef HkTensorContainer_HH__
#define HkTensorContainer_HH__

/*
*File: HkTensorContainer.hh
*Class: HkTensorContainer
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date: 2020-05-15T20:22:00.867Z
*Description:
*/


#include <tuple>
#include <utility>
#include <string>

#include "HkUnits.hh"
#include "HkMultiTypeMap.hh"
#include "HkArrayWrapper.hh"
#include "HkVectorContainer.hh"

namespace hops
{

template< typename...XAxisTypeS >
class HkAxisPack:  public std::tuple< XAxisTypeS... >
{
    public:

        HkAxisPack():std::tuple< XAxisTypeS... >(){};
        virtual ~HkAxisPack(){};

        template<size_t N = 0>
        typename std::enable_if< N == sizeof...(XAxisTypeS), void >::type
        resize_axis_pack( const size_t* /*dim*/)
        {
            //terminating case, do nothing
        }

        template<size_t N = 0>
        typename std::enable_if< N < sizeof...(XAxisTypeS), void>::type
        resize_axis_pack(const size_t* dim)
        {
            //resize the N-th element of the tuple
            std::get<N>(*this).Resize(dim[N]);
            //now run the induction
            resize_axis_pack<N + 1>(dim);
        }

};



template< typename XValueType, size_t RANK, typename XAxisPackType >
class HkTensorContainer: public HkArrayWrapper< XValueType, RANK>, public HkNamed
{
    public:

        HkTensorContainer():
            HkArrayWrapper<XValueType,RANK>(),
            HkNamed()
        {};

        HkTensorContainer(const size_t* dim):
            HkArrayWrapper<XValueType,RANK>(dim)
        {};

        virtual ~HkTensorContainer(){};

        using HkNamed::IsNamed;
        using HkNamed::GetName;
        using HkNamed::SetName;

        //have to make base class functions visible
        using HkArrayWrapper<XValueType,RANK>::Resize;
        using HkArrayWrapper<XValueType,RANK>::GetData;
        using HkArrayWrapper<XValueType,RANK>::GetRawData;
        using HkArrayWrapper<XValueType,RANK>::GetSize;
        using HkArrayWrapper<XValueType,RANK>::GetDimensions;
        using HkArrayWrapper<XValueType,RANK>::GetDimension;
        using HkArrayWrapper<XValueType,RANK>::GetOffsetForIndices;

        using HkArrayWrapper<XValueType,RANK>::operator();
        using HkArrayWrapper<XValueType,RANK>::operator[];

    protected:

        using HkArrayWrapper<XValueType,RANK>::fData;
        using HkArrayWrapper<XValueType,RANK>::fDimensions;
        using HkArrayWrapper<XValueType,RANK>::fTotalArraySize;

    public:

        XAxisPackType fAxes;

        // //experimental map for axis types
        // HkMultiTypeMap< size_t, XAxesTypeS... > fAxisMap;


};

}//end of namespace

#endif /* end of include guard: HkTensorContainer_HH__ */
