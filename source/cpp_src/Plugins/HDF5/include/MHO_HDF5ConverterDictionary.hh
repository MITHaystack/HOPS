#ifndef MHO_HDF5ConverterDictionary_HH__
#define MHO_HDF5ConverterDictionary_HH__

#include "MHO_Axis.hh"
#include "MHO_AxisPack.hh"
#include "MHO_ObjectTags.hh"
#include "MHO_ScalarContainer.hh"
#include "MHO_TableContainer.hh"
#include "MHO_VectorContainer.hh"

#include "MHO_ContainerHDF5Converter.hh"
#include "MHO_MD5HashGenerator.hh"
#include "MHO_UUID.hh"

// #include "MHO_ClassIdentityMap.hh"
// #include "MHO_ContainerDictionary.hh"

namespace hops
{

/*!
 *@file MHO_HDF5ConverterDictionary.hh
 *@class MHO_HDF5ConverterDictionary
 *@author J. Barrett - barrettj@mit.edu
 *@date Fri May 16 10:42:52 AM EDT 2025
 *@brief Dictionary of objects which do the conversion to HDF5 for each type
 */

class MHO_HDF5ConverterDictionary
{
    public:
        MHO_HDF5ConverterDictionary();

        virtual ~MHO_HDF5ConverterDictionary()
        {
            for(auto it = fHDF5ConverterMap.begin(); it != fHDF5ConverterMap.end(); it++)
            {
                delete it->second;
            }
        };

        template< typename XClassType > void AddHDF5ClassType()
        {
            fMD5Generator.Initialize();
            std::string name = MHO_ClassIdentity::ClassName< XClassType >();
            fMD5Generator << name;
            fMD5Generator.Finalize();
            MHO_UUID type_uuid = fMD5Generator.GetDigestAsUUID();

            auto it2 = fHDF5ConverterMap.find(type_uuid);
            if(it2 == fHDF5ConverterMap.end())
            {
                fHDF5ConverterMap.emplace(type_uuid, new MHO_ContainerHDF5Converter< XClassType >());
            }
        };

    protected:
        MHO_MD5HashGenerator fMD5Generator;
        std::map< MHO_UUID, MHO_HDF5Converter* > fHDF5ConverterMap;
};

} // namespace hops

#endif /*! end of include guard: MHO_HDF5ConverterDictionary */
