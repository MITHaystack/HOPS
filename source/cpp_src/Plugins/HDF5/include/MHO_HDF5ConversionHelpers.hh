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




template < typename XValueType >
inline void make_scalar_dataset(hid_t file_id, hid_t dataset_id, const std::string& name, XValueType value) 
{
    hid_t TYPE_CODE = MHO_HDF5TypeCode<XValueType>();
    hid_t space_id = H5Screate(H5S_SCALAR);
    dataset_id = H5Dcreate(file_id, name.c_str(), TYPE_CODE, space_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    H5Dwrite(dataset_id, TYPE_CODE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &value);
    H5Dclose(dataset_id);
    H5Sclose(space_id);
}


template<>
inline void make_scalar_dataset< std::string >(hid_t file_id, hid_t dataset_id, const std::string& name, std::string value) 
{
    hid_t TYPE_CODE = H5Tcopy(H5T_C_S1);
    H5Tset_size(TYPE_CODE, value.size());
    H5Tset_strpad(TYPE_CODE, H5T_STR_NULLTERM);
    hid_t space_id = H5Screate(H5S_SCALAR);
    dataset_id = H5Dcreate(file_id, name.c_str(), TYPE_CODE, space_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    H5Dwrite(dataset_id, TYPE_CODE, H5S_ALL, H5S_ALL, H5P_DEFAULT, value.c_str());
    H5Dclose(dataset_id);
    H5Sclose(space_id);
    H5Tclose(TYPE_CODE);
}


//generic  - write a scalar attribute
template < typename XValueType >
inline void write_attribute(const std::string& key, XValueType value, hid_t dataset_id)
{
    hid_t attr_space = H5Screate(H5S_SCALAR);
    hid_t TYPE_CODE = MHO_HDF5TypeCode<XValueType>();
    hid_t attr_id = H5Acreate(dataset_id, key.c_str(), TYPE_CODE, attr_space, H5P_DEFAULT, H5P_DEFAULT);
    H5Awrite(attr_id, TYPE_CODE, &value);
    H5Aclose(attr_id);
    H5Sclose(attr_space);
}

//specialization for std::string
template<>
inline void write_attribute< std::string >(const std::string& key, std::string value, hid_t dataset_id)
{
    hid_t attr_space = H5Screate(H5S_SCALAR);
    hid_t TYPE_CODE = H5Tcopy(H5T_C_S1);
    H5Tset_size(TYPE_CODE, strlen(value.c_str() ));
    H5Tset_strpad(TYPE_CODE, H5T_STR_NULLTERM);
    hid_t attr_id = H5Acreate(dataset_id, key.c_str(), TYPE_CODE, attr_space, H5P_DEFAULT, H5P_DEFAULT);
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
                             hid_t dataset_id)
{
    herr_t status;
    hsize_t dims[1];
    dims[0] = data->size();
    hid_t attr_space = H5Screate_simple(1, dims, NULL);
    //get the type code
    hid_t TYPE_CODE = MHO_HDF5TypeCode<XDataType>();
    hid_t attr_id = H5Acreate(dataset_id, key.c_str(), TYPE_CODE, attr_space, H5P_DEFAULT, H5P_DEFAULT);
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
                             hid_t dataset_id)
{
    herr_t status;
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
    hid_t attr_id = H5Acreate(dataset_id, key.c_str(), TYPE_CODE, attr_space, H5P_DEFAULT, H5P_DEFAULT);
    status = H5Awrite(attr_id, TYPE_CODE, cstrs.data() );
    H5Aclose(attr_id);
    H5Sclose(attr_space);

    return status;
}

//wrapper function to write attributes in a json object
inline void make_attribute(const std::string& key, const mho_json& value, hid_t dataset_id)
{
    msg_debug("hdf5interface", "creating attribute with key: "<< key << eom);
    //for now we can only handle scalar types
    hid_t attr_space = H5Screate(H5S_SCALAR);
    hid_t TYPE_CODE;
    void* attr_data;

    if (value.is_string()) 
    {
        std::string strv = value.get<std::string>();
        write_attribute(key, strv, dataset_id);
    } 
    else if (value.is_number_integer()) 
    {
        int v = value.get<int>();
        write_attribute(key,v,dataset_id);
    } 
    else if (value.is_number_unsigned()) 
    {
        unsigned int v = value.get<unsigned int>();
        write_attribute(key,v,dataset_id);
    } 
    else if (value.is_number_float()) 
    {
        double v = value.get<double>();
        write_attribute(key,v,dataset_id);
    }
    else if (value.is_boolean()) 
    {
        uint8_t v = value.get<bool>() ? 1 : 0;
        write_attribute(key,v,dataset_id);
    } 
    else if ( value.is_array() && value.size() != 0) 
    {
        //all of these assume we are not mixing types in the array --
        //this is a safe assumption for HOPS4 data containers 
        if(value.begin()->is_number_integer())
        {
            std::vector<int> data = value.get< std::vector<int> >();
            make_vector_attribute(key, &data, dataset_id);
        }
        if(value.begin()->is_number_unsigned())
        {
            std::vector<unsigned int> data = value.get< std::vector<unsigned int> >();
            make_vector_attribute(key, &data, dataset_id);
        }
        if(value.begin()->is_number_float())
        {
            std::vector<double> data = value.get< std::vector<double> >();
            make_vector_attribute(key, &data, dataset_id);
        }
        if(value.begin()->is_string())
        {
            std::vector<std::string> data = value.get< std::vector< std::string > >();
            make_vector_attribute(key, &data, dataset_id);
        }
    } 
    else 
    {
        //for composite objects, we dump them into a string
        //since we can't represent the arbitrary complexity/nesting of json objects 
        //in HDF5 without a ton of work, this unfortunately means this data 
        //will be opque to most HDF5 inspection tools (string needs to parsed back into json to access)
        std::stringstream ss;
        ss << value.dump();
        std::string sval = ss.str();
        write_attribute(key,sval,dataset_id);
        return;
    }
}


//template function for writing a dimensions scale (axis)
template< typename XDataType >
herr_t 
inline make_scale(hid_t file_id, hid_t dataset_id, std::size_t axis_idx,
                  const std::string& name,
                  const std::vector< XDataType >* data, 
                  const mho_json& metadata)
{
    herr_t status;
    hsize_t dims[1];
    dims[0] = data->size();
    hid_t dataspace_id = H5Screate_simple(1, dims, NULL);

    //get the type code
    hid_t TYPE_CODE = MHO_HDF5TypeCode<XDataType>();
    //create axis dataset
    hid_t axis_dset_id = H5Dcreate(file_id, name.c_str(), TYPE_CODE, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    //write the data
    status = H5Dwrite(axis_dset_id, TYPE_CODE, H5S_ALL, H5S_ALL, H5P_DEFAULT, data->data() );

    H5DSset_scale(axis_dset_id, name.c_str());
    H5DSattach_scale(dataset_id, axis_dset_id, axis_idx);  // attach to appropriate index

    //attach meta data in 'tags'
    if( metadata.contains("tags"))
    {
        for(auto it = metadata["tags"].begin(); it != metadata["tags"].end(); ++it) 
        {
            const std::string& key = it.key();
            const mho_json& value = it.value();
            make_attribute(key, value, axis_dset_id);
        }
    }
    //attach class name and uuid if they exist
    if( metadata.contains("class_uuid"))
    {
        std::string key = "class_uuid";
        mho_json value = metadata[key];
        make_attribute(key, value, axis_dset_id);
    }
    if( metadata.contains("class_name"))
    {
        std::string key = "class_name";
        mho_json value = metadata[key];
        make_attribute(key, value, axis_dset_id);
    }

    return status;
}

///specialization a dimension axis which contains strings
template<>
herr_t 
inline make_scale< std::string>(hid_t file_id, hid_t dataset_id, std::size_t axis_idx,
                         const std::string& name,
                         const std::vector< std::string >* data, 
                         const mho_json& metadata) 
{
    herr_t status;
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

    hid_t dataspace_id = H5Screate_simple(1, dims, NULL);

    //create axis dataset
    hid_t axis_dset_id = H5Dcreate(file_id, name.c_str(), TYPE_CODE, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    status = H5Dwrite(axis_dset_id, TYPE_CODE, H5S_ALL, H5S_ALL, H5P_DEFAULT, cstrs.data() );

    H5DSset_scale(axis_dset_id, name.c_str());
    H5DSattach_scale(dataset_id, axis_dset_id, axis_idx);  // attach to appropriate index

    //attach meta data
    if( metadata.contains("tags"))
    {
        for(auto it = metadata["tags"].begin(); it != metadata["tags"].end(); ++it) 
        {
            const std::string& key = it.key();
            //std::string key = name + "/" + it.key();
            const mho_json& value = it.value();
            make_attribute(key, value, axis_dset_id);
        }
    }
    //attach class name and uuid if they exist
    if( metadata.contains("class_uuid"))
    {
        std::string key = "class_uuid";
        mho_json value = metadata[key];
        make_attribute(key, value, axis_dset_id);
    }
    if( metadata.contains("class_name"))
    {
        std::string key = "class_name";
        mho_json value = metadata[key];
        make_attribute(key, value, axis_dset_id);
    }

    return status;
}


//write an ND-array data object
template< typename XDataType >
herr_t 
inline make_dataset(hid_t file_id, hid_t& dataset_id, 
             const std::string& name, hsize_t rank, hsize_t* dims,
             const XDataType* data, const mho_json& metadata) 
{
    herr_t status;
    hid_t dataspace_id = -1;
    //return the dataset_id in reference so we can attach attributes later
    dataset_id = -1;
    //create dataspace
    dataspace_id = H5Screate_simple(rank, dims, NULL);
    if (dataspace_id < 0)
    {
        msg_error("main", "could not create dataspace" << eom);
        return -1;
    }
    //get the type code
    hid_t TYPE_CODE = MHO_HDF5TypeCode<XDataType>();
    //create dataset
    dataset_id = H5Dcreate(file_id, name.c_str(), TYPE_CODE, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if (dataset_id < 0) 
    {
        msg_error("main", "could not create data set" << eom);
        H5Sclose(dataspace_id);
        return -1;
    }

    //write data
    status = H5Dwrite(dataset_id, TYPE_CODE, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);

    //attach meta data in 'tags'
    if( metadata.contains("tags"))
    {
        for(auto it = metadata["tags"].begin(); it != metadata["tags"].end(); ++it) 
        {
            const std::string& key = it.key();
            const mho_json& value = it.value();
            make_attribute(key, value, dataset_id);
        }
    }
    //attach class name and uuid if they exist
    if( metadata.contains("class_uuid"))
    {
        std::string key = "class_uuid";
        mho_json value = metadata[key];
        make_attribute(key, value, dataset_id);
    }
    if( metadata.contains("class_name"))
    {
        std::string key = "class_name";
        mho_json value = metadata[key];
        make_attribute(key, value, dataset_id);
    }

    //clean up
    H5Dclose(dataset_id);
    H5Sclose(dataspace_id);

    return status;
}


//1-D string dataset 
herr_t 
inline make_string_vector_dataset(hid_t file_id, hid_t dataset_id,
                                  const std::string& name,
                                  const std::vector< std::string >* data, 
                                  const mho_json& metadata) 
{
    herr_t status;
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

    hid_t dataspace_id = H5Screate_simple(1, dims, NULL);

    //create axis dataset
    dataset_id = H5Dcreate(file_id, name.c_str(), TYPE_CODE, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    status = H5Dwrite(dataset_id, TYPE_CODE, H5S_ALL, H5S_ALL, H5P_DEFAULT, cstrs.data() );

    //attache the metadata
    for(auto it = metadata.begin(); it != metadata.end(); ++it) 
    {
        const std::string& key = it.key();
        const mho_json& value = it.value();
        make_attribute(key, value, dataset_id);
    }

    //clean up
    H5Dclose(dataset_id);
    H5Sclose(dataspace_id);


    return status;
}



} // namespace hops

#endif
