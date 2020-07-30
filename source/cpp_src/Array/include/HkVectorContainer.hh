#ifndef HkVectorContainer_HH__
#define HkVectorContainer_HH__

/*
*File: HkVectorContainer.hh
*Class: HkVectorContainer
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date: 2020-05-15T20:22:00.867Z
*Description:
*/


#include <string>

#include "HkUnits.hh"
#include "HkNamed.hh"

namespace hops
{


template< typename XValueType, typename XUnitType = HEmptyUnit >
class HkVectorContainer: public HNamed, public HkArrayWrapper< XValueType, 1>
{
    public:

        HkVectorContainer(): HkArrayWrapper<XValueType, 1>(){};

        HkVectorContainer( XValueType* data, const std::size_t* dim):
            HkArrayWrapper<XValueType,1>(data, dim)
        {

        };


        HkVectorContainer(): HkArrayWrapper<XValueType, 1>(size_t ) {};
        virtual ~HkVectorContainer(){};


        //declare the unit type (not implemented for now)
        using unit = XUnitType;

        //have to make base class functions visible
        using HkArrayWrapper<XValueType,1>::SetData;
        using HkArrayWrapper<XValueType,1>::GetData;

        using HkArrayWrapper<XValueType,1>::GetArraySize;
        using HkArrayWrapper<XValueType,1>::SetArrayDimensions;

(const std::size_t* array_dim)
        {
            for(std::size_t i=0; i<RANK; i++)
            {
                fDimensions[i] = array_dim[i];
            }
            fTotalArraySize = HkArrayMath::TotalArraySize<RANK>(fDimensions);
        }

        void GetArrayDimensions(std::size_t* array_dim) const
        {
            for(std::size_t i=0; i<RANK; i++)
            {
                array_dim[i] = fDimensions[i];
            }
        }

        const std::size_t* GetArrayDimensions() const
        {
            return fDimensions;
        }

        std::size_t GetArrayDimension(std::size_t dim_index) const
        {
            return fDimensions[dim_index];
        }

        std::size_t GetOffsetForIndices(const size_t* index)
        {
            return HkArrayMath::OffsetFromRowMajorIndex<RANK>(fDimensions, index);
        }







        std::size_t GetArraySize() const {return 1;};

    protected:

        using HkArrayWrapper<XValueType,1>::fData;

        XValueType* fData; //raw pointer to multidimensional array
        std::size_t fDimensions[RANK]; //size of each dimension
        std::size_t fTotalArraySize; //total size of array

};

}//end of hops namespace

#endif /* end of include guard: HkVectorContainer_HH__ */
