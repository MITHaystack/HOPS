#ifndef MHO_HDF5Attributes_HH__
#define MHO_HDF5Attributes_HH__

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

namespace hops
{

/*!
 *@file MHO_HDF5Attributes.hh
 *@class MHO_HDF5Attributes
 *@author J. Barrett - barrettj@mit.edu
 *@date Fri May 16 10:42:52 AM EDT 2025
 *@brief template functions to write attributes associated with hdf5 objects
 * 
 */

//generic  - write a scalar attribute
template < typename XValueType >
inline void make_attribute(const std::string& key, XValueType value, hid_t parent_dataset_id)
{
    if(H5Aexists(parent_dataset_id, key.c_str()) > 0)
    {
        msg_error("hdf5interface", "attribute: "<<key<<" already exists, skipping"<< eom);
        return;
    }

    hid_t TYPE_CODE = MHO_HDF5TypeCode<XValueType>();
    hid_t attr_space = H5Screate(H5S_SCALAR);
    hid_t attr_id = H5Acreate(parent_dataset_id, key.c_str(), TYPE_CODE, attr_space, H5P_DEFAULT, H5P_DEFAULT);
    if(attr_id < 0)
    {
        msg_error("hdf5interface", "could not create attribute: "<<key<<", id code is: "<<attr_id<< eom);
        return;
    }

    H5Awrite(attr_id, TYPE_CODE, &value);
    H5Aclose(attr_id);
    H5Sclose(attr_space);
}

//specialization for std::string
template<>
inline void make_attribute< std::string >(const std::string& key, std::string value, hid_t parent_dataset_id)
{
    if(H5Aexists(parent_dataset_id, key.c_str()) > 0)
    {
        msg_error("hdf5interface", "string attribute: "<<key<<" already exists, skipping"<< eom);
        return;
    }
    
    if(value.size() < 1)
    {
        msg_error("hdf5interface", "string attribute: "<<key<<" has size: "<<value.size()<<", skipping"<< eom);
        return;
    }
    
    hid_t attr_space = H5Screate(H5S_SCALAR);
    hid_t TYPE_CODE = H5Tcopy(H5T_C_S1);
    H5Tset_size(TYPE_CODE, value.size());
    H5Tset_strpad(TYPE_CODE, H5T_STR_NULLTERM);
    hid_t attr_id = H5Acreate(parent_dataset_id, key.c_str(), TYPE_CODE, attr_space, H5P_DEFAULT, H5P_DEFAULT);
    if(attr_id < 0)
    {
        msg_error("hdf5interface", "could not create string attribute: "<<key<<", id code is: "<<attr_id<< eom);
        return;
    }
    H5Awrite(attr_id, TYPE_CODE, value.c_str());
    H5Aclose(attr_id);
    H5Sclose(attr_space);
    H5Tclose(TYPE_CODE);
    return;
}

//generic - write a vector attribute
template< typename XDataType >
herr_t 
inline make_vector_attribute(const std::string& key,
                             const std::vector< XDataType >* data, 
                             hid_t parent_dataset_id)
{
    herr_t status = -1;
    if(H5Aexists(parent_dataset_id, key.c_str()) > 0)
    {
        msg_error("hdf5interface", "attribute: "<<key<<" already exists, skipping"<< eom);
        return status;
    }

    hsize_t dims[1];
    dims[0] = data->size();
    hid_t attr_space = H5Screate_simple(1, dims, NULL);
    //get the type code
    hid_t TYPE_CODE = MHO_HDF5TypeCode<XDataType>();
    hid_t attr_id = H5Acreate(parent_dataset_id, key.c_str(), TYPE_CODE, attr_space, H5P_DEFAULT, H5P_DEFAULT);
    if(attr_id < 0)
    {
        msg_error("hdf5interface", "could not create vector attribute: "<<key<<", id code is: "<<attr_id<< eom);
        return status;
    }
    status = H5Awrite(attr_id, TYPE_CODE, data->data() );
    H5Aclose(attr_id);
    H5Sclose(attr_space);
    return status;
}


//specialization for vector of strings
template<>
herr_t 
inline make_vector_attribute(const std::string& key,
                             const std::vector< std::string >* data, 
                             hid_t parent_dataset_id)
{
    herr_t status = -1;
    if(H5Aexists(parent_dataset_id, key.c_str()) > 0)
    {
        msg_error("hdf5interface", "attribute: "<<key<<" already exists, skipping"<< eom);
        return status;
    }

    hsize_t dims[1];
    dims[0] = data->size();
    hid_t TYPE_CODE = H5Tcopy(H5T_C_S1);
    H5Tset_size(TYPE_CODE, H5T_VARIABLE);

    //convert to const chars
    std::vector<const char*> cstrs;
    for(const auto& s : *data) 
    {
        cstrs.push_back(s.c_str());
    }
    hid_t attr_space = H5Screate_simple(1, dims, NULL);
    hid_t attr_id = H5Acreate(parent_dataset_id, key.c_str(), TYPE_CODE, attr_space, H5P_DEFAULT, H5P_DEFAULT);
    if(attr_id < 0)
    {
        msg_error("hdf5interface", "could not create string vector attribute: "<<key<<", id code is: "<<attr_id<< eom);
        return status;
    }
    status = H5Awrite(attr_id, TYPE_CODE, cstrs.data() );
    H5Aclose(attr_id);
    H5Sclose(attr_space);

    return status;
}

//wrapper function to write attributes in a json object
inline void make_attribute(const std::string& key, const mho_json& value, hid_t parent_dataset_id)
{
    msg_debug("hdf5interface", "creating attribute with key: "<< key << eom);
    //for now we can only handle scalar types
    hid_t attr_space = H5Screate(H5S_SCALAR);
    hid_t TYPE_CODE;
    void* attr_data;

    if (value.is_string()) 
    {
        std::string strv = value.get<std::string>();
        make_attribute(key, strv, parent_dataset_id);
    } 
    else if (value.is_number_integer()) 
    {
        int v = value.get<int>();
        make_attribute(key,v,parent_dataset_id);
    } 
    else if (value.is_number_unsigned()) 
    {
        unsigned int v = value.get<unsigned int>();
        make_attribute(key,v,parent_dataset_id);
    } 
    else if (value.is_number_float()) 
    {
        double v = value.get<double>();
        make_attribute(key,v,parent_dataset_id);
    }
    else if (value.is_boolean()) 
    {
        uint8_t v = value.get<bool>() ? 1 : 0;
        make_attribute(key,v,parent_dataset_id);
    } 
    else if ( value.is_array() && value.size() != 0) 
    {
        if( !(value.begin()->is_object() ) ) //no arrays of composite objects
        {
            //all of these assume we are not mixing types in the array --
            //this is a safe assumption for HOPS4 data containers 
            if(value.begin()->is_number_integer())
            {
                std::vector<int> data = value.get< std::vector<int> >();
                make_vector_attribute(key, &data, parent_dataset_id);
            }
            if(value.begin()->is_number_unsigned())
            {
                std::vector<unsigned int> data = value.get< std::vector<unsigned int> >();
                make_vector_attribute(key, &data, parent_dataset_id);
            }
            if(value.begin()->is_number_float())
            {
                std::vector<double> data = value.get< std::vector<double> >();
                make_vector_attribute(key, &data, parent_dataset_id);
            }
            if(value.begin()->is_string())
            {
                std::vector<std::string> data = value.get< std::vector< std::string > >();
                make_vector_attribute(key, &data, parent_dataset_id);
            }
        }
    } 
    else if( value.is_object() ) 
    {
        //for composite objects, we have to dump them into a string
        //HDF5 doesn't support nesting of attributes 
        msg_debug("hdf5interface", "composite object attribute: "<<key<<", will stored as string" << eom);
        std::stringstream ss;
        ss << value.dump();
        std::string sval = ss.str();
        make_attribute(key,sval,parent_dataset_id);
    }
}




} // namespace hops

#endif
