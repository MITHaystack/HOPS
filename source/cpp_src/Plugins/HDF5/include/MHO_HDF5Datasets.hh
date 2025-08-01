#ifndef MHO_HDF5Datasets_HH__
#define MHO_HDF5Datasets_HH__

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

#include "MHO_HDF5Attributes.hh"
#include "MHO_HDF5TypeCode.hh"
#include "MHO_NumpyTypeCode.hh"

namespace hops
{

/*!
 *@file MHO_HDF5Datasets.hh
 *@class MHO_HDF5Datasets
 *@author J. Barrett - barrettj@mit.edu
 *@date Fri May 16 10:42:52 AM EDT 2025
 *@brief template functions to write various data types to HDF5
 *
 */

template< typename XValueType >
inline void make_scalar_dataset(hid_t group_id, hid_t& dataset_id, const std::string& name, const XValueType& value)
{
    hid_t TYPE_CODE = MHO_HDF5TypeCode< XValueType >();
    hid_t space_id = H5Screate(H5S_SCALAR);
    dataset_id = H5Dcreate(group_id, name.c_str(), TYPE_CODE, space_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    H5Dwrite(dataset_id, TYPE_CODE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &value);
    H5Dclose(dataset_id);
    H5Sclose(space_id);
}

template<>
inline void make_scalar_dataset< std::string >(hid_t group_id, hid_t& dataset_id, const std::string& name,
                                               const std::string& value)
{
    hid_t TYPE_CODE = H5Tcopy(H5T_C_S1);
    H5Tset_size(TYPE_CODE, value.size());
    H5Tset_strpad(TYPE_CODE, H5T_STR_NULLTERM);
    hid_t space_id = H5Screate(H5S_SCALAR);
    dataset_id = H5Dcreate(group_id, name.c_str(), TYPE_CODE, space_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    H5Dwrite(dataset_id, TYPE_CODE, H5S_ALL, H5S_ALL, H5P_DEFAULT, value.c_str());
    H5Dclose(dataset_id);
    H5Sclose(space_id);
    H5Tclose(TYPE_CODE);
}

//template function for writing a dimension scale (coordinate axis)
template< typename XDataType >
herr_t inline make_scale(hid_t group_id, hid_t dataset_id, std::size_t axis_idx, const std::string& parent_name,
                         const std::string& name, const std::vector< XDataType >* data, const mho_json& metadata)
{
    herr_t status;
    hsize_t dims[1];
    dims[0] = data->size();
    hid_t dataspace_id = H5Screate_simple(1, dims, NULL);

    //get the type code
    hid_t TYPE_CODE = MHO_HDF5TypeCode< XDataType >();
    //create axis dataset
    hid_t axis_dset_id = H5Dcreate(group_id, name.c_str(), TYPE_CODE, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    //write the data
    status = H5Dwrite(axis_dset_id, TYPE_CODE, H5S_ALL, H5S_ALL, H5P_DEFAULT, data->data());

    H5DSset_scale(axis_dset_id, name.c_str());
    H5DSattach_scale(dataset_id, axis_dset_id, axis_idx); // attach to appropriate index

    //attach meta data in 'tags'
    if(metadata.contains("tags"))
    {
        for(auto it = metadata["tags"].begin(); it != metadata["tags"].end(); ++it)
        {
            const std::string& key = it.key();
            const mho_json& value = it.value();
            if(key == "index_labels")
            {
                hid_t tmp_dset_id = -1;
                std::string tmp_key = parent_name + "/" + key;
                std::string sval = value.dump();
                make_scalar_dataset(group_id, tmp_dset_id, tmp_key, sval);
            }
            else if(key == "interval_labels")
            {
                hid_t tmp_dset_id;
                std::string tmp_key = parent_name + "/" + key;
                std::string sval = value.dump();
                make_scalar_dataset(group_id, tmp_dset_id, tmp_key, sval);
            }
            else
            {
                make_attribute(key, value, axis_dset_id);
            }
        }
    }
    //attach class name and uuid if they exist
    if(metadata.contains("class_uuid"))
    {
        std::string key = "class_uuid";
        mho_json value = metadata[key];
        make_attribute(key, value, axis_dset_id);
    }
    if(metadata.contains("class_name"))
    {
        std::string key = "class_name";
        mho_json value = metadata[key];
        make_attribute(key, value, axis_dset_id);
    }

    return status;
}

///specialization a dimension/coordinate axis which contains strings
template<>
herr_t inline make_scale< std::string >(hid_t group_id, hid_t dataset_id, std::size_t axis_idx, const std::string& parent_name,
                                        const std::string& name, const std::vector< std::string >* data,
                                        const mho_json& metadata)
{
    herr_t status;
    hsize_t dims[1];
    dims[0] = data->size();
    hid_t TYPE_CODE = H5Tcopy(H5T_C_S1);
    H5Tset_size(TYPE_CODE, H5T_VARIABLE);

    //convert to const chars
    std::vector< const char* > cstrs;
    for(const auto& s : *data)
    {
        cstrs.push_back(s.c_str());
    }

    hid_t dataspace_id = H5Screate_simple(1, dims, NULL);

    //create axis dataset
    hid_t axis_dset_id = H5Dcreate(group_id, name.c_str(), TYPE_CODE, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    status = H5Dwrite(axis_dset_id, TYPE_CODE, H5S_ALL, H5S_ALL, H5P_DEFAULT, cstrs.data());

    H5DSset_scale(axis_dset_id, name.c_str());
    H5DSattach_scale(dataset_id, axis_dset_id, axis_idx); // attach to appropriate index

    //attach meta data
    if(metadata.contains("tags"))
    {
        for(auto it = metadata["tags"].begin(); it != metadata["tags"].end(); ++it)
        {
            const std::string& key = it.key();
            const mho_json& value = it.value();
            if(key == "index_labels")
            {
                hid_t tmp_dset_id = -1;
                std::string tmp_key = parent_name + "/" + key;
                std::string sval = value.dump();
                make_scalar_dataset(group_id, tmp_dset_id, tmp_key, sval);
            }
            else if(key == "interval_labels")
            {
                hid_t tmp_dset_id = -1;
                std::string tmp_key = parent_name + "/" + key;
                std::string sval = value.dump();
                make_scalar_dataset(group_id, tmp_dset_id, tmp_key, sval);
            }
            else
            {
                make_attribute(key, value, axis_dset_id);
            }
        }
    }
    //attach class name and uuid if they exist
    if(metadata.contains("class_uuid"))
    {
        std::string key = "class_uuid";
        mho_json value = metadata[key];
        make_attribute(key, value, axis_dset_id);
    }
    if(metadata.contains("class_name"))
    {
        std::string key = "class_name";
        mho_json value = metadata[key];
        make_attribute(key, value, axis_dset_id);
    }

    return status;
}

//write an ND-array data object
template< typename XDataType >
herr_t inline make_dataset(hid_t group_id, hid_t& dataset_id, const std::string& name, hsize_t rank, hsize_t* dims,
                           const XDataType* data, const mho_json& metadata)
{
    herr_t status;
    hid_t dataspace_id = -1;
    //return the dataset_id in reference so we can attach attributes later
    dataset_id = -1;
    //create dataspace
    dataspace_id = H5Screate_simple(rank, dims, NULL);
    if(dataspace_id < 0)
    {
        msg_error("main", "could not create dataspace" << eom);
        return -1;
    }
    //get the type code
    hid_t TYPE_CODE = MHO_HDF5TypeCode< XDataType >();
    //create dataset
    dataset_id = H5Dcreate(group_id, name.c_str(), TYPE_CODE, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if(dataset_id < 0)
    {
        msg_error("main", "could not create data set" << eom);
        H5Sclose(dataspace_id);
        return -1;
    }

    //write data
    status = H5Dwrite(dataset_id, TYPE_CODE, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);

    //attach meta data in 'tags'
    if(metadata.contains("tags"))
    {
        for(auto it = metadata["tags"].begin(); it != metadata["tags"].end(); ++it)
        {
            const std::string& key = it.key();
            const mho_json& value = it.value();
            make_attribute(key, value, dataset_id);
        }
    }
    //attach class name and uuid if they exist
    if(metadata.contains("class_uuid"))
    {
        std::string key = "class_uuid";
        mho_json value = metadata[key];
        make_attribute(key, value, dataset_id);
    }
    if(metadata.contains("class_name"))
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
herr_t inline make_string_vector_dataset(hid_t group_id, hid_t dataset_id, const std::string& name,
                                         const std::vector< std::string >* data, const mho_json& metadata)
{
    herr_t status;
    hsize_t dims[1];
    dims[0] = data->size();
    hid_t TYPE_CODE = H5Tcopy(H5T_C_S1);
    H5Tset_size(TYPE_CODE, H5T_VARIABLE);

    //convert to const chars
    std::vector< const char* > cstrs;
    for(const auto& s : *data)
    {
        cstrs.push_back(s.c_str());
    }

    hid_t dataspace_id = H5Screate_simple(1, dims, NULL);
    //create axis dataset
    dataset_id = H5Dcreate(group_id, name.c_str(), TYPE_CODE, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    status = H5Dwrite(dataset_id, TYPE_CODE, H5S_ALL, H5S_ALL, H5P_DEFAULT, cstrs.data());

    //attache the metadata
    for(auto it = metadata.begin(); it != metadata.end(); ++it)
    {
        const std::string& key = it.key();
        //exclude plot_data and parameters from attributes -- these are large objects
        if(key != "plot_data" && key != "parameters")
        {
            const mho_json& value = it.value();
            make_attribute(key, value, dataset_id);
        }
    }

    //clean up
    H5Dclose(dataset_id);
    H5Sclose(dataspace_id);

    return status;
}

} // namespace hops

#endif
