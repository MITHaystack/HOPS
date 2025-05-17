#ifndef MHO_ContainerHDF5Converter_HH__
#define MHO_ContainerHDF5Converter_HH__

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

#include "hdf5.h"
#include "hdf5_hl.h" 

#include "MHO_NumpyTypeCode.hh"
#include "MHO_HDF5TypeCode.hh"

namespace hops
{

/*!
 *@file MHO_ContainerHDF5Converter.hh
 *@class MHO_ContainerHDF5Converter
 *@author J. Barrett - barrettj@mit.edu
 *@date Fri May 16 10:42:52 AM EDT 2025
 *@brief Converts a given ndarray-based container into a HDF5 representation - export only for now
 * 
 */


template< typename XDataType >
herr_t 
inline make_scale(hid_t file_id, hid_t dataset_id, std::size_t axis_idx,
                  const std::string& name,
                  const std::vector< XDataType >* data, 
                  const std::string& metadata)
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

    return status;
}

///specialization for strings
template<>
herr_t 
inline make_scale< std::string>(hid_t file_id, hid_t dataset_id, std::size_t axis_idx,
                         const std::string& name,
                         const std::vector< std::string >* data, 
                         const std::string& metadata) 
{
    std::cout<<"string axis"<<std::endl;
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

    // hid_t axis0_dset = H5Dcreate(file, "/axis_0_labels", str_type, axis0_space,
    //                              H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    hid_t dataspace_id = H5Screate_simple(1, dims, NULL);

    //create axis dataset
    hid_t axis_dset_id = H5Dcreate(file_id, name.c_str(), TYPE_CODE, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    //write the data
    //status = H5Dwrite(axis_dset_id, TYPE_CODE, H5S_ALL, H5S_ALL, H5P_DEFAULT, data->data() );

    // Write and mark as scale
    status = H5Dwrite(axis_dset_id, TYPE_CODE, H5S_ALL, H5S_ALL, H5P_DEFAULT, cstrs.data() );
    // H5DSset_scale(axis0_dset, "axis_0");

    H5DSset_scale(axis_dset_id, name.c_str());
    H5DSattach_scale(dataset_id, axis_dset_id, axis_idx);  // attach to appropriate index

    return status;

    // hsize_t dims[1] = {length};
    // hid_t dataspace_id = H5Screate_simple(1, dims, nullptr);
    // //get the type code
    // hid_t TYPE_CODE = MHO_HDF5TypeCode<XDataType>();
    // //create axis dataset
    // hid_t axis_dset_id = H5Dcreate(file_id, name.c_str(), TYPE_CODE, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    // //write the data
    // H5Dwrite(axis_dset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);
    // H5DSset_scale(axis_dset_id, name.c_str());
    // H5DSattach_scale(dataset_id, axis_dset_id, axis_idx);  // attach to appropriate dim
}

template< typename XDataType >
herr_t 
inline make_dataset(hid_t file_id, hid_t& dataset_id, 
             const std::string& name, hsize_t rank, hsize_t* dims,
             const XDataType* data, const std::string& metadata) 
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
    //clean up
    H5Dclose(dataset_id);
    H5Sclose(dataspace_id);

    return status;
}


class MHO_HDF5Converter
{
    public:
        MHO_HDF5Converter(){};
        virtual ~MHO_HDF5Converter(){};

        virtual void SetObjectToConvert(MHO_Serializable* /*!obj*/) = 0;
        virtual void WriteToHDF5File(hid_t /*file_id*/, std::string /*group_prefix*/) = 0;

    protected:

};

template< typename XContainerType > class MHO_ContainerHDF5Converter: public MHO_HDF5Converter
{
    public:
        MHO_ContainerHDF5Converter(): MHO_HDF5Converter() { fContainer = nullptr; }

        MHO_ContainerHDF5Converter(MHO_ExtensibleElement* element): MHO_HDF5Converter()
        {
            fContainer = dynamic_cast< XContainerType* >(element);
        }

        virtual ~MHO_ContainerHDF5Converter() {}

        virtual void SetObjectToConvert(MHO_Serializable* obj) { fContainer = dynamic_cast< XContainerType* >(obj); };

        virtual void WriteToHDF5File(hid_t file_id, std::string group_prefix)
        {
            if(fContainer != nullptr)
            {
                ConstructHDF5(file_id, group_prefix, fContainer);
            }
        }

    private:
        XContainerType* fContainer;

    protected:

        //unspecialized template doesn't do much
        template< typename XCheckType > void ConstructHDF5(hid_t /*file_id*/, std::string /*group_prefix*/, const XCheckType* obj)
        {
            std::string class_name = MHO_ClassIdentity::ClassName< XCheckType >();
            std::string class_uuid = MHO_ClassIdentity::GetUUIDFromClass< XCheckType >().as_string();
        };

        //use SFINAE to generate specialization for the rest of the container types
        //that we actually care about

        //scalar specialization
        template< typename XCheckType = XContainerType >
        typename std::enable_if< std::is_base_of< MHO_ScalarContainerBase, XCheckType >::value, void >::type
        ConstructHDF5(hid_t file_id, std::string group_prefix, const XContainerType* obj)
        {
            std::cout<<"scalar type"<<std::endl;
        };

        //vector specialization (but not an axis!)
        template< typename XCheckType = XContainerType >
        typename std::enable_if< (std::is_base_of< MHO_VectorContainerBase, XCheckType >::value &&
                                  !std::is_base_of< MHO_AxisBase, XCheckType >::value),
                                 void >::type
        ConstructHDF5(hid_t file_id, std::string group_prefix, const XContainerType* obj)
        {
            std::cout<<"vector type"<<std::endl;
            std::string class_name = MHO_ClassIdentity::ClassName< XContainerType >();
            std::string class_uuid = MHO_ClassIdentity::GetUUIDFromClass< XContainerType >().as_string();
            std::string object_uuid = obj->GetObjectUUID().as_string();

            //grab the rank and dimensions
            hsize_t rank = 1;
            hsize_t dims[1];
            auto dim_array = fContainer->GetDimensionArray();
            for(std::size_t i=0; i<rank; i++){dims[i] = dim_array[i];}
            std::string mdata = "";
            std::string item_group = group_prefix + "/" + object_uuid;

            if (H5Lexists(file_id, item_group.c_str(), H5P_DEFAULT) == 0) 
            {
                hid_t group_id = H5Gcreate(file_id, item_group.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
                if(group_id < 0) 
                {
                    std::cout<<"failed to create HDF5 group"<<std::endl;
                }

                std::string name = item_group + "/data";
                //write the data
                hid_t dataset_id = -1;
                herr_t status = 
                make_dataset< typename XContainerType::value_type >(file_id, dataset_id, name, rank, dims, fContainer->GetData(), mdata); 

                H5Gclose(group_id);
            }
        };

        //axis specialization
        template< typename XCheckType = XContainerType >
        typename std::enable_if< std::is_base_of< MHO_AxisBase, XCheckType >::value, void >::type
        ConstructHDF5(hid_t file_id, std::string group_prefix, const XContainerType* obj)
        {
            std::cout<<"axis type"<<std::endl;
            std::string class_name = MHO_ClassIdentity::ClassName< XContainerType >();
            std::string class_uuid = MHO_ClassIdentity::GetUUIDFromClass< XContainerType >().as_string();
            std::string object_uuid = obj->GetObjectUUID().as_string();

            //grab the rank and dimensions
            hsize_t rank = 1;
            hsize_t dims[1];
            auto dim_array = fContainer->GetDimensionArray();
            for(std::size_t i=0; i<rank; i++){dims[i] = dim_array[i];}
            std::string mdata = "";
            std::string item_group = group_prefix + "/" + object_uuid;

            if (H5Lexists(file_id, item_group.c_str(), H5P_DEFAULT) == 0) 
            {
                hid_t group_id = H5Gcreate(file_id, item_group.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
                if(group_id < 0) 
                {
                    std::cout<<"failed to create HDF5 group"<<std::endl;
                }

                std::string name = item_group + "/data";
                //write the data
                hid_t dataset_id = -1;
                herr_t status = 
                make_dataset< typename XContainerType::value_type >(file_id, dataset_id, name, rank, dims, fContainer->GetData(), mdata); 

                H5Gclose(group_id);
            }


            // std::string name = group_prefix + "/" + object_uuid + "/data";
            // 
            // //write the data
            // hid_t dataset_id = -1;
            // herr_t status = 
            // make_dataset< typename XContainerType::value_type >(file_id, dataset_id, name, rank, dims, fContainer->GetData(), mdata); 
        };

        //table specialization
        template< typename XCheckType = XContainerType >
        typename std::enable_if< std::is_base_of< MHO_TableContainerBase, XCheckType >::value, void >::type
        ConstructHDF5(hid_t file_id, std::string group_prefix, const XContainerType* obj)
        {
            std::cout<<"table type"<<std::endl;
            std::string class_name = MHO_ClassIdentity::ClassName< XContainerType >();
            std::string class_uuid = MHO_ClassIdentity::GetUUIDFromClass< XContainerType >().as_string();
            std::string object_uuid = obj->GetObjectUUID().as_string();

            //grab the rank and dimensions
            hsize_t rank = XContainerType::rank::value;
            hsize_t dims[XContainerType::rank::value];
            auto dim_array = obj->GetDimensionArray();
            for(std::size_t i=0; i<rank; i++){dims[i] = dim_array[i];}
            std::string mdata = "";
            // std::string name = group_prefix + "/" + object_uuid + "/data";
            // 
            // //write the table data
            // hid_t dataset_id = -1;
            // herr_t status = 
            // make_dataset< typename XContainerType::value_type >(file_id, dataset_id, name, rank, dims, fContainer->GetData(), mdata); 

            std::string item_group = group_prefix + "/" + object_uuid;

            if (H5Lexists(file_id, item_group.c_str(), H5P_DEFAULT) == 0) 
            {
                hid_t group_id = H5Gcreate(file_id, item_group.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
                if(group_id < 0) 
                {
                    std::cout<<"failed to create HDF5 group"<<std::endl;
                }

                std::string name = item_group + "/data";
                //write the data
                hid_t dataset_id = -1;
                herr_t status = 
                make_dataset< typename XContainerType::value_type >(file_id, dataset_id, name, rank, dims, fContainer->GetData(), mdata); 

                //now attach the table axes
                name = item_group;
                AxisDumper axis_dumper(file_id, dataset_id, name);
                for(std::size_t idx = 0; idx < obj->GetRank(); idx++)
                {
                    axis_dumper.SetIndex(idx);
                    apply_at< typename XContainerType::axis_pack_tuple_type, AxisDumper >(*obj, idx, axis_dumper);
                }

                H5Gclose(group_id);
            }

        };

        class AxisDumper
        {
            public:
                AxisDumper(hid_t file_id, hid_t ds_id, const std::string& parent): 
                    fFileID(file_id), 
                    fDataSetID(ds_id), 
                    fParentName(parent){};

                AxisDumper(): fFileID(-1), fDataSetID(-1), fParentName(""){};
                ~AxisDumper(){};
        
                void SetIndex(std::size_t idx) { fIndex = idx; }
                void SetFileID(hid_t file_id){ fFileID = file_id; }
                void SetDataSetID(hid_t ds_id){ fDataSetID = ds_id; }
        
                template< typename XAxisType > void operator()(const XAxisType& axis)
                {
                    mho_json j;
                    std::string class_name = MHO_ClassIdentity::ClassName< XAxisType >();
                    std::string class_uuid = MHO_ClassIdentity::GetUUIDFromClass< XAxisType >().as_string();

                    std::stringstream ss;
                    ss << fParentName << "/axis_";
                    ss << fIndex;
                    std::string name = ss.str();
                    std::string mdata = "";

                    std::cout<<"ax name = "<<name<<std::endl;

                    //copy the data into a temporary vector
                    std::size_t n = axis.GetSize();
                    std::vector< typename XAxisType::value_type > axis_data;
                    for(std::size_t i=0; i<n; i++){axis_data.push_back( axis.at(i) ); }
        
                    hsize_t dims[1];
                    dims[0] = n;

                    std::cout<<"vtype = "<<MHO_ClassIdentity::ClassName< typename XAxisType::value_type >()<<std::endl;

                    hid_t dataset_id = -1;
                    herr_t status = 
                    make_scale< typename XAxisType::value_type >(fFileID, fDataSetID, fIndex, name, &axis_data, mdata);
                };

            private:

                hid_t fFileID;
                hid_t fDataSetID;
                std::string fParentName;
                std::size_t fIndex;

        };
};








template<> class MHO_ContainerHDF5Converter< MHO_ObjectTags >: public MHO_HDF5Converter
{
    public:
        MHO_ContainerHDF5Converter(): MHO_HDF5Converter() { fContainer = nullptr; }

        MHO_ContainerHDF5Converter(MHO_ExtensibleElement* element): MHO_HDF5Converter()
        {
            fContainer = dynamic_cast< MHO_ObjectTags* >(element);
        }

        virtual ~MHO_ContainerHDF5Converter() {}

        virtual void SetObjectToConvert(MHO_Serializable* obj) { fContainer = dynamic_cast< MHO_ObjectTags* >(obj); };

        virtual void WriteToHDF5File(hid_t file_id, std::string group_prefix)
        {
            if(fContainer != nullptr)
            {
                ConstructHDF5(file_id, group_prefix, fContainer);
            }
        }

    private:

        void ConstructHDF5(hid_t file_id, std::string group_prefix, const MHO_ObjectTags* obj)
        {
            std::cout<<"tag type" << std::endl;
        };

        MHO_ObjectTags* fContainer;
};

} // namespace hops

#endif
