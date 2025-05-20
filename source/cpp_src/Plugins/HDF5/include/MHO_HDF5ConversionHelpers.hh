#ifndef MHO_HDF5ConversionHelpers_HH__
#define MHO_HDF5ConversionHelpers_HH__

#include "MHO_ClassIdentity.hh"
#include "MHO_ExtensibleElement.hh"
#include "MHO_JSONHeaderWrapper.hh"

#include "MHO_Axis.hh"
#include "MHO_AxisPack.hh"
#include "MHO_ObjectTags.hh"
#include "MHO_ScalarContainer.hh"
#include "MHO_TableContainer.hh"
#include "MHO_Taggable.hh"
#include "MHO_VectorContainer.hh"

#include <sstream>

#include "hdf5.h"
#include "hdf5_hl.h" 

#include "MHO_NumpyTypeCode.hh"
#include "MHO_HDF5TypeCode.hh"
#include "MHO_HDF5Attributes.hh"
#include "MHO_HDF5Datasets.hh"

namespace hops
{

/*!
 *@file MHO_HDF5ConversionHelpers.hh
 *@class MHO_HDF5ConversionHelpers
 *@author J. Barrett - barrettj@mit.edu
 *@date Fri May 16 10:42:52 AM EDT 2025
 *@brief template functions to write various data types to HDF5
 * 
 */


inline void json_to_hdf5(const mho_json& j, hid_t parent_group) 
{
    for(auto it = j.begin(); it != j.end(); ++it) 
    {
        const std::string& key = it.key();
        const mho_json& value = it.value();

        if( value.is_object() ) 
        {
            hid_t subgroup = H5Gcreate(parent_group, key.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
            json_to_hdf5(value, subgroup);
            H5Gclose(subgroup);
        } 
        else if ( value.is_array() && value.size() != 0)  
        {
            if( !(value.begin()->is_object()) )
            {
                make_attribute(key, value, parent_group);
            }
            else 
            {
                hid_t array_group = H5Gcreate(parent_group, key.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
                int idx = 0;
                for (const auto& elem : value) 
                {
                    std::string item_name = std::to_string(idx++);
                    hid_t item_group = H5Gcreate(array_group, item_name.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
                    json_to_hdf5(elem, item_group);
                    H5Gclose(item_group);
                }
                H5Gclose(array_group);
            }
        } 
        else 
        {
            make_attribute(key, value, parent_group);
        }
    }
}



} // namespace hops

#endif
