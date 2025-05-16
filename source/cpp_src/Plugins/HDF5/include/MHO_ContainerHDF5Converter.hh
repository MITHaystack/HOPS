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

//verbosity controlling enum
enum MHO_HDF5VerbosityLevel : int
{
    eHDF5BasicLevel = 0,      //basic quantities (rank, dimensions, etc.)
    eHDF5TagsLevel,           //basic quantities plus tags
    eHDF5AxesLevel,           //basic quantities plus the axes (if the object is a table)
    eHDF5AxesWithLabelsLevel, //basic quantities plus axes with interval labels
    eHDF5AllLevel             //everything including the main data array
};

//short hand aliases
static const MHO_HDF5VerbosityLevel eHDF5Basic = MHO_HDF5VerbosityLevel::eHDF5BasicLevel;
static const MHO_HDF5VerbosityLevel eHDF5Tags = MHO_HDF5VerbosityLevel::eHDF5TagsLevel;
static const MHO_HDF5VerbosityLevel eHDF5WithAxes = MHO_HDF5VerbosityLevel::eHDF5AxesLevel;
static const MHO_HDF5VerbosityLevel eHDF5WithLabels = MHO_HDF5VerbosityLevel::eHDF5AxesWithLabelsLevel;
static const MHO_HDF5VerbosityLevel eHDF5All = MHO_HDF5VerbosityLevel::eHDF5AllLevel;

using hops::eHDF5All;
using hops::eHDF5Basic;
using hops::eHDF5Tags;
using hops::eHDF5WithAxes;
using hops::eHDF5WithLabels;

// inline void FillHDF5FromTaggable(const MHO_Taggable* map, mho_json& obj_tags)
// {
//     bool ok;
//     obj_tags = map->GetMetaDataAsHDF5();
// }


template< typename XDataType >
herr_t 
inline make_scale(hid_t file_id, hid_t dataset_id, std::size_t axis_idx,
                  const std::string& name,
                  const std::vector< XDataType >* data, 
                  const std::string& metadata)
{
    std::cout<<"file ="<<file_id<<std::endl;
    std::cout<<"dataset id = "<<dataset_id<<std::endl;
    std::cout<<"axis id ="<<axis_idx<<std::endl;
    std::cout<<"name = "<<name<<std::endl;


    herr_t status;
    hsize_t dims[1];
    dims[0] = data->size();
    hid_t dataspace_id = H5Screate_simple(1, dims, NULL);

    std::cout<<"new dataspace id = "<< dataspace_id<<std::endl;
    //get the type code
    hid_t TYPE_CODE = MHO_HDF5TypeCode<XDataType>();

    std::cout<<"type code = "<<TYPE_CODE<<std::endl;

    // if (H5Lexists(file_id, name.c_str(), H5P_DEFAULT) > 0) 
    // {
    //     std::cout<<"WHATTT?"<<std::endl;
    //     H5Ldelete(file_id, name.c_str(), H5P_DEFAULT);
    // }

    //create axis dataset
    hid_t axis_dset_id = H5Dcreate(file_id, name.c_str(), TYPE_CODE, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    std::cout<<"axis dset id ="<<axis_dset_id<<std::endl;

    //write the data
    status = H5Dwrite(axis_dset_id, TYPE_CODE, H5S_ALL, H5S_ALL, H5P_DEFAULT, data->data() );

    H5DSset_scale(axis_dset_id, name.c_str());

    H5DSattach_scale(dataset_id, axis_dset_id, axis_idx);  // attach to appropriate dim

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
    return -1;
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

    // //attach the metadata if it isn't empty 
    // if(metadata != "")
    // {
    //     std::string attr_mname = "metadata";
    //     attach_metadata(dataset_id, attr_mname, metadata);
    // }

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
        virtual void WriteToHDF5File(hid_t /*file_id*/) = 0;
        //virtual void ConstructHDF5Representation() = 0;

    protected:
        // 
        // //helper functions for generic data insertion for elements of a list
        // template< typename XValueType > void InsertElement(const XValueType& value, mho_json& data) { data.push_back(value); }
        // 
        // //specializations for complex<> element data insertion, needed b/c mho_json doesn't have a first-class complex type
        // void InsertElement(const std::complex< long double >& value, mho_json& data)
        // {
        //     data.push_back({value.real(), value.imag()});
        // }
        // 
        // void InsertElement(const std::complex< double >& value, mho_json& data)
        // {
        //     data.push_back({value.real(), value.imag()});
        // }
        // 
        // void InsertElement(const std::complex< float >& value, mho_json& data) { data.push_back({value.real(), value.imag()}); }

        // //data
        // int fLOD;
        // mho_json fHDF5;
        // 
        // //for extracting container raw data (with no meta data structures)
        // std::size_t fRank;
        // std::size_t fRawByteSize;
        // const char* fRawData;
        // std::string fRawDataDescriptor;
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


        virtual void WriteToHDF5File(hid_t file_id)
        {
            if(fContainer != nullptr)
            {
                ConstructHDF5(file_id, fContainer);
            }
        }

    private:
        XContainerType* fContainer;

    protected:

        //unspecialized template doesn't do much
        template< typename XCheckType > void ConstructHDF5(hid_t /*file_id*/, const XCheckType* obj)
        {
            std::string class_name = MHO_ClassIdentity::ClassName< XCheckType >();
            std::string class_uuid = MHO_ClassIdentity::GetUUIDFromClass< XCheckType >().as_string();
        };

        //use SFINAE to generate specialization for the rest of the container types
        //that we actually care about

        //scalar specialization
        template< typename XCheckType = XContainerType >
        typename std::enable_if< std::is_base_of< MHO_ScalarContainerBase, XCheckType >::value, void >::type
        ConstructHDF5(hid_t file_id, const XContainerType* obj)
        {
            std::cout<<"scalar type"<<std::endl;
            // fHDF5.clear();
            // std::string class_name = MHO_ClassIdentity::ClassName< XContainerType >();
            // std::string class_uuid = MHO_ClassIdentity::GetUUIDFromClass< XContainerType >().as_string();
            // if(fLOD >= eHDF5Basic)
            // {
            //     fHDF5["class_name"] = class_name;
            //     fHDF5["class_uuid"] = class_uuid;
            //     fHDF5["rank"] = fContainer->GetRank();
            //     fHDF5["total_size"] = fContainer->GetSize();
            // }
            // 
            // if(fLOD >= eHDF5Tags)
            // {
            //     mho_json jtags;
            //     // FillHDF5FromCommonMap(fContainer, jtags);
            //     FillHDF5FromTaggable(fContainer, jtags);
            //     fHDF5["tags"] = jtags;
            // }
            // 
            // //just one element
            // if(fLOD >= eHDF5All)
            // {
            //     mho_json data;
            //     InsertElement(fContainer->GetData(), data);
            //     fHDF5["data"] = data;
            // }
            
            // //for raw data extraction (not possible)
            // fRank = 0;
            // fRawByteSize = 0;
            // fRawData = nullptr;
            // fRawDataDescriptor = "";
        };

        //vector specialization (but not an axis!)
        template< typename XCheckType = XContainerType >
        typename std::enable_if< (std::is_base_of< MHO_VectorContainerBase, XCheckType >::value &&
                                  !std::is_base_of< MHO_AxisBase, XCheckType >::value),
                                 void >::type
        ConstructHDF5(hid_t file_id, const XContainerType* obj)
        {
            std::cout<<"vector type"<<std::endl;
            std::string class_name = MHO_ClassIdentity::ClassName< XContainerType >();
            std::string class_uuid = MHO_ClassIdentity::GetUUIDFromClass< XContainerType >().as_string();

            //grab the rank and dimensions
            hsize_t rank = XContainerType::rank::value;
            hsize_t* dims = nullptr;
            dims = new hsize_t[rank];
            auto dim_array = fContainer->GetDimensionArray();
            for(std::size_t i=0; i<rank; i++){dims[i] = dim_array[i];}
            std::string mdata = "";

            //write the data
            hid_t dataset_id = -1;
            herr_t status = 
            make_dataset< typename XContainerType::value_type >(file_id, dataset_id, class_name, rank, dims, fContainer->GetData(), mdata); 

            //dete
            delete[] dims;
            // fHDF5.clear();
            // std::string class_name = MHO_ClassIdentity::ClassName< XContainerType >();
            // std::string class_uuid = MHO_ClassIdentity::GetUUIDFromClass< XContainerType >().as_string();
            // if(fLOD >= eHDF5Basic)
            // {
            //     fHDF5["class_name"] = class_name;
            //     fHDF5["class_uuid"] = class_uuid;
            //     fHDF5["rank"] = fContainer->GetRank();
            //     fHDF5["total_size"] = fContainer->GetSize();
            //     fHDF5["dimensions"] = fContainer->GetDimensionArray();
            // }
            // 
            // if(fLOD >= eHDF5Tags)
            // {
            //     mho_json jtags;
            //     FillHDF5FromTaggable(fContainer, jtags);
            //     //FillHDF5FromCommonMap(fContainer ,jtags);
            //     fHDF5["tags"] = jtags;
            // }
            // 
            // //data goes out flat-packed into 1-d array
            // if(fLOD >= eHDF5All)
            // {
            //     mho_json data;
            //     for(auto it = fContainer->cbegin(); it != fContainer->cend(); it++)
            //     {
            //         InsertElement(*it, data);
            //     }
            //     fHDF5["data"] = data;
            // }
            
            // //for raw data extraction 
            // std::size_t elem_size = sizeof( typename XContainerType::value_type);
            // std::size_t n_elem = fContainer->GetSize();
            // fRank = XContainerType::rank::value;
            // fRawByteSize = elem_size*n_elem;
            // fRawData = reinterpret_cast<const char*>( fContainer->GetData() );
            // fRawDataDescriptor = MHO_NumpyTypeCode< typename XContainerType::value_type >();
        };

        //axis specialization
        template< typename XCheckType = XContainerType >
        typename std::enable_if< std::is_base_of< MHO_AxisBase, XCheckType >::value, void >::type
        ConstructHDF5(hid_t file_id, const XContainerType* obj)
        {
            std::cout<<"axis type"<<std::endl;
            std::string class_name = MHO_ClassIdentity::ClassName< XContainerType >();
            std::string class_uuid = MHO_ClassIdentity::GetUUIDFromClass< XContainerType >().as_string();

            //grab the rank and dimensions
            hsize_t rank = XContainerType::rank::value;
            hsize_t* dims = nullptr;
            dims = new hsize_t[rank];
            auto dim_array = fContainer->GetDimensionArray();
            for(std::size_t i=0; i<rank; i++){dims[i] = dim_array[i];}
            std::string mdata = "";

            //write the data
            hid_t dataset_id = -1;
            herr_t status = 
            make_dataset< typename XContainerType::value_type >(file_id, dataset_id, class_name, rank, dims, fContainer->GetData(), mdata); 

            //dete
            delete[] dims;

            // fHDF5.clear();
            // std::string class_name = MHO_ClassIdentity::ClassName< XContainerType >();
            // std::string class_uuid = MHO_ClassIdentity::GetUUIDFromClass< XContainerType >().as_string();
            // if(fLOD >= eHDF5Basic)
            // {
            //     fHDF5["class_name"] = class_name;
            //     fHDF5["class_uuid"] = class_uuid;
            //     fHDF5["rank"] = fContainer->GetRank();
            //     fHDF5["total_size"] = fContainer->GetSize();
            //     fHDF5["dimensions"] = fContainer->GetDimensionArray();
            // }
            // 
            // if(fLOD >= eHDF5Tags)
            // {
            //     mho_json jtags;
            //     FillHDF5FromTaggable(fContainer, jtags);
            //     //FillHDF5FromCommonMap(fContainer ,jtags);
            //     fHDF5["tags"] = jtags;
            // }
            // 
            // //data goes out flat-packed into 1-d array
            // if(fLOD >= eHDF5All)
            // {
            //     mho_json data;
            //     for(auto it = fContainer->cbegin(); it != fContainer->cend(); it++)
            //     {
            //         InsertElement(*it, data);
            //     }
            //     fHDF5["data"] = data;
            // }
            
            // //for raw data extraction 
            // std::size_t elem_size = sizeof( typename XContainerType::value_type);
            // std::size_t n_elem = fContainer->GetSize();
            // fRank = XContainerType::rank::value;
            // fRawByteSize = elem_size*n_elem;
            // fRawData = reinterpret_cast<const char*>( fContainer->GetData() );
            // fRawDataDescriptor = MHO_NumpyTypeCode< typename XContainerType::value_type >();
        };

        //table specialization
        template< typename XCheckType = XContainerType >
        typename std::enable_if< std::is_base_of< MHO_TableContainerBase, XCheckType >::value, void >::type
        ConstructHDF5(hid_t file_id, const XContainerType* obj)
        {
            std::cout<<"table type"<<std::endl;
            std::string class_name = MHO_ClassIdentity::ClassName< XContainerType >();
            std::string class_uuid = MHO_ClassIdentity::GetUUIDFromClass< XContainerType >().as_string();
            std::string object_uuid = obj->GetObjectUUID().as_string();

            //grab the rank and dimensions
            hsize_t rank = XContainerType::rank::value;
            hsize_t* dims = nullptr;
            dims = new hsize_t[rank];
            auto dim_array = obj->GetDimensionArray();
            for(std::size_t i=0; i<rank; i++){dims[i] = dim_array[i];}
            std::string mdata = "";

            //write the table data
            hid_t dataset_id = -1;
            herr_t status = 
            make_dataset< typename XContainerType::value_type >(file_id, dataset_id, object_uuid, rank, dims, fContainer->GetData(), mdata); 

            //dete
            delete[] dims;

            //now attach the table axes
            AxisDumper axis_dumper(file_id, dataset_id, object_uuid);
            for(std::size_t idx = 0; idx < obj->GetRank(); idx++)
            {
                axis_dumper.SetIndex(idx);
                apply_at< typename XContainerType::axis_pack_tuple_type, AxisDumper >(*obj, idx, axis_dumper);
            }

            //now attach all of the tag meta data attributes 



            // if(fLOD >= eHDF5Basic)
            // {
            //     fHDF5["class_name"] = class_name;
            //     fHDF5["class_uuid"] = class_uuid;
            //     fHDF5["rank"] = fContainer->GetRank();
            //     fHDF5["total_size"] = fContainer->GetSize();
            //     fHDF5["dimensions"] = fContainer->GetDimensionArray();
            //     fHDF5["strides"] = fContainer->GetStrideArray();
            // }
            // 
            // if(fLOD >= eHDF5Tags)
            // {
            //     mho_json jtags;
            //     FillHDF5FromTaggable(fContainer, jtags);
            //     //FillHDF5FromCommonMap(fContainer, jtags);
            //     fHDF5["tags"] = jtags;
            // }
            // 
            // //data goes out flat-packed into 1-d array
            // if(fLOD >= eHDF5All)
            // {
            //     mho_json data;
            //     for(auto it = fContainer->cbegin(); it != fContainer->cend(); it++)
            //     {
            //         InsertElement(*it, data);
            //     }
            //     fHDF5["data"] = data;
            // }
            // 
            // if(fLOD >= eHDF5WithAxes)
            // {

            // }


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

                    make_scale< typename XAxisType::value_type >(fFileID, fDataSetID, fIndex, name, &axis_data, mdata);

                    // if(fLOD >= eHDF5Basic)
                    // {
                    //     j["class_name"] = class_name;
                    //     j["class_uuid"] = class_uuid;
                    //     j["rank"] = axis.GetRank();
                    //     j["total_size"] = axis.GetSize();
                    //     j["dimensions"] = axis.GetDimensionArray();
                    // }
                    // 
                    // if(fLOD >= eHDF5Tags)
                    // {
                    //     mho_json jtags;
                    //     FillHDF5FromTaggable(&axis, jtags);
                    //     //FillHDF5FromCommonMap(&axis, jtags);
                    //     j["tags"] = jtags;
                    // }
                    // 
                    // //data goes out flat-packed into 1-d array
                    // mho_json data;
                    // for(auto it = axis.cbegin(); it != axis.cend(); it++)
                    // {
                    //     data.push_back(*it);
                    // }
                    // j["data"] = data;
                    // 
                    // std::stringstream ss;
                    // ss << "axis_" << fIndex;
                    // (*fAxisHDF5)[ss.str().c_str()] = j;
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

        virtual void WriteToHDF5File(hid_t file_id)
        {
            if(fContainer != nullptr)
            {
                ConstructHDF5(file_id, fContainer);
            }
        }

    private:

        void ConstructHDF5(hid_t file_id, const MHO_ObjectTags* obj)
        {
            std::cout<<"tag type" << std::endl;
            // fHDF5.clear();
            // std::string class_name = MHO_ClassIdentity::ClassName< MHO_ObjectTags >();
            // std::string class_uuid = MHO_ClassIdentity::GetUUIDFromClass< MHO_ObjectTags >().as_string();
            // fHDF5["class_name"] = class_name;
            // fHDF5["class_uuid"] = class_uuid;
            // 
            // std::vector< MHO_UUID > obj_uuids = obj->GetAllObjectUUIDs();
            // for(std::size_t i = 0; i < obj_uuids.size(); i++)
            // {
            //     fHDF5["object_uuids"].push_back(obj_uuids[i].as_string());
            // }
            // 
            // mho_json jtags;
            // // FillHDF5FromCommonMap(obj, jtags);
            // FillHDF5FromTaggable(obj, jtags);
            // fHDF5["tags"] = jtags;
            // 
            // //for raw data extraction (not possible)
            // fRank = 0;
            // fRawByteSize = 0;
            // fRawData = nullptr;
            // fRawDataDescriptor = "tags";
        };

        MHO_ObjectTags* fContainer;
};

} // namespace hops

#endif
