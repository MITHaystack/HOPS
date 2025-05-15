#include "MHO_ContainerDictionary.hh"
#include "MHO_ContainerFileInterface.hh"
#include "MHO_ContainerStore.hh"
#include "MHO_Message.hh"
#include "MHO_HDF5TypeCode.hh"

#include <stdio.h>
#include <utility>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "hdf5.h"

using namespace hops;

//option parsing and help text library
#include "CLI11.hpp"

herr_t 
attach_metadata(hid_t object_id, const std::string& attr_name, const std::string& metadata) 
{
    hid_t attr_id = -1;
    hid_t dataspace_id = -1;
    hid_t type_id = -1;
    herr_t status;

    //create a scalar dataspace (for a single string)
    hsize_t dims[1] = {1};
    dataspace_id = H5Screate_simple(1, dims, NULL);
    if (dataspace_id < 0)
    {
        msg_warn("main", "failed to create json string dataspace");
        return -1;
    }
    //create string data type
    type_id = H5Tcopy(H5T_C_S1);
    H5Tset_size(type_id, metadata.size());
    H5Tset_strpad(type_id, H5T_STR_NULLTERM);

    // Create attribute
    attr_id = H5Acreate(object_id, attr_name.c_str(), type_id, dataspace_id,
                        H5P_DEFAULT, H5P_DEFAULT);
    if(attr_id < 0) 
    {
        msg_warn("main", "failed to create json string meta data attribute");
        H5Sclose(dataspace_id);
        H5Tclose(type_id);
        return -1;
    }

    //write
    status = H5Awrite(attr_id, type_id, metadata.c_str());

    //clean up
    H5Aclose(attr_id);
    H5Sclose(dataspace_id);
    H5Tclose(type_id);

    return status;
}




template< typename XDataType >
herr_t 
make_dataset(hid_t file_id, const std::string& name, 
             hsize_t rank, hsize_t* dims,
             const XDataType* data, const std::string& metadata) 
{
    herr_t status;
    hid_t dataspace_id = -1;
    hid_t dataset_id = -1;
    
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
    dataset_id = H5Dcreate(file_id, name.c_str(), TYPE_CODE, 
                           dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if (dataset_id < 0) 
    {
        msg_error("main", "could not create data set" << eom);
        H5Sclose(dataspace_id);
        return -1;
    }

    //write data
    status = H5Dwrite(dataset_id, TYPE_CODE, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);

    //attach the metadata if it isn't empty 
    if(metadata != "")
    {
        std::string attr_mname = "metadata";
        attach_metadata(dataset_id, attr_mname, metadata);
    }

    //clean up
    H5Dclose(dataset_id);
    H5Sclose(dataspace_id);

    return status;
}


herr_t write_to_hdf5file(hid_t file_id, 
                       std::size_t rank,
                       const char* raw_data, 
                       std::size_t raw_data_byte_size,
                       const std::string& dataset_name,
                       const std::string& rdd, 
                       const mho_json& meta_obj)
{
    herr_t status = -1;
    herr_t mstatus = -1;
    if( !meta_obj[dataset_name].contains("dimensions"))
    {
        std::cout<< meta_obj[dataset_name]["dimensions"].dump(2) << std::endl;
        msg_warn("main", "meta data object does not contain a 'dimensions' item" << eom);
        return status;
    }

    //write out the raw table data (if it exists) in one big chunk
    if(raw_data != nullptr && raw_data_byte_size != 0 && rdd != "" && rank != 0)
    {
        //grab the dimensions
        hsize_t* dims = nullptr;
        dims = new hsize_t[rank];
        std::vector< std::size_t > dimensions = meta_obj[dataset_name]["dimensions"].get< std::vector<std::size_t> >();
        for(std::size_t i=0; i<rank; i++){dims[i] = dimensions[i];}

        //convert metadata to a string
        std::stringstream ss;
        ss << meta_obj;
        std::string meta_data = ss.str();
        
        if(rdd == "i1")
        {
            const int8_t* ptr = reinterpret_cast<const int8_t*>(raw_data);
            if(ptr != nullptr)
            {
                status = make_dataset<int8_t>(file_id, dataset_name, rank, dims, ptr, meta_data); 
            }
        }
        if(rdd == "u1")
        {
            const uint8_t* ptr = reinterpret_cast<const uint8_t*>(raw_data);
            if(ptr != nullptr){status = make_dataset<uint8_t>(file_id, dataset_name, rank, dims, ptr, meta_data); }
        }
        if(rdd == "i2")
        {
            const int16_t* ptr = reinterpret_cast<const int16_t*>(raw_data);
            if(ptr != nullptr){status = make_dataset<int16_t>(file_id, dataset_name, rank, dims, ptr, meta_data); }
        }
        if(rdd == "u1")
        {
            const uint16_t* ptr = reinterpret_cast<const uint16_t*>(raw_data);
            if(ptr != nullptr){status = make_dataset<uint16_t>(file_id, dataset_name, rank, dims, ptr, meta_data); }
        }
        if(rdd == "i4")
        {
            const int32_t* ptr = reinterpret_cast< const int32_t*>(raw_data);
            if(ptr != nullptr){status = make_dataset<int32_t>(file_id, dataset_name, rank, dims, ptr, meta_data); }
        }
        if(rdd == "u4")
        {
            const uint32_t* ptr = reinterpret_cast<const uint32_t*>(raw_data);
            if(ptr != nullptr){status = make_dataset<uint32_t>(file_id, dataset_name, rank, dims, ptr, meta_data); }
        }
        if(rdd == "i8")
        {
            const int64_t* ptr = reinterpret_cast<const int64_t*>(raw_data);
            if(ptr != nullptr){status = make_dataset<int64_t>(file_id, dataset_name, rank, dims, ptr, meta_data); }
        }
        if(rdd == "i8")
        {
            const uint64_t* ptr = reinterpret_cast<const uint64_t*>(raw_data);
            if(ptr != nullptr){status = make_dataset<uint64_t>(file_id, dataset_name, rank, dims, ptr, meta_data); }
        }
        if(rdd == "f4")
        {
            const float* ptr = reinterpret_cast<const float*>(raw_data);
            if(ptr != nullptr){status = make_dataset<float>(file_id, dataset_name, rank, dims, ptr, meta_data); }
        }
        if(rdd == "f8")
        {
            const double* ptr = reinterpret_cast< const double*>(raw_data);
            if(ptr != nullptr){status = make_dataset<double>(file_id, dataset_name, rank, dims, ptr, meta_data); }
        }
        if(rdd == "c8")
        {
            //not supported yet
        }
        if(rdd == "c16")
        {
            //not supported yet
        }
        //delete the dims object
        delete[] dims;
    }

    return status;

    // // Create a 1D dataset of doubles
    // if(H5LTmake_dataset_double(file_id, dataset_name.c_str(), 1, dims, data) < 0) 
    // {
    //     std::cout<<"failed to create dataset "<< std::endl;
    //     H5Fclose(file_id);
    //     return 1;
    // }
    // 
    // // Attach JSON metadata as an attribute
    // if (H5LTset_attribute_string(file_id, dataset_name.c_str(), attribute_name.c_str(), json_metadata.c_str()) < 0) 
    // {
    //     std::cout<<"failed to write attribute"<< std::endl;
    //     H5Fclose(file_id);
    //     return 1;
    // }



}




int main(int argc, char** argv)
{
    std::string input_file = "";
    std::string output_file = "";
    //meta data detail level
    int detail = eJSONAxesWithLabelsLevel;
    std::string uuid_string = "";
    std::string shortname = "";
    int message_level = 0;

    CLI::App app{"hops2hdf5"};
    app.add_option("-m,--message-level", message_level, "message level to be used, range: -2 (debug) to 5 (silent)");
    app.add_option("-u,--uuid", uuid_string, "specify and extract a single object by UUID");
    app.add_option("-s,--shortname", shortname,
                   "specify and extract a single object by shortname (returns first matching object) ")
        ->excludes("--uuid");
    app.add_option("input,-i,--input-file", input_file, "name of the input (hops) file to be converted")->required();
    app.add_option("output,-o,--output-file", output_file,
                   "name of the output file, if not given the result will be stored in <input-file>.hdf5");

    CLI11_PARSE(app, argc, argv);
    

    // app.add_option("-d,--detail", detail,
    //                 "level of detail to be used when generating the output, range: 0 (low) to 3 (high), default (3)");
    // //clamp detail level
    // if(detail > eJSONAxesWithLabelsLevel){detail = eJSONAxesWithLabelsLevel;}
    // if(detail < eJSONBasicLevel){detail = eJSONBasicLevel;}

    //clamp message level
    if(message_level > 5)
    {
        message_level = 5;
    }
    if(message_level < -2)
    {
        message_level = -2;
    }
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetLegacyMessageLevel(message_level);

    if(input_file == "")
    {
        msg_fatal("main", "input_file not set" << eom);
        return 1;
    }

    //set default output name, if not passed
    if(output_file == "")
    {
        output_file = input_file + ".hdf5";
    }

    //read in all the objects in a file
    MHO_ContainerStore conStore;
    MHO_ContainerFileInterface conInter;
    conInter.SetFilename(input_file);
    conInter.PopulateStoreFromFile(conStore);
    
    //if a specific object was requested -- convert given uuid string to MHO_UUID object
    bool single_object = false;
    MHO_UUID single_obj_uuid;
    if(shortname != "")
    {
        single_obj_uuid = conStore.GetObjectUUID(shortname);
        if(single_obj_uuid.is_empty())
        {
            msg_fatal("main", "object uuid for: "<< shortname << ", could not be determined" << eom);
            return 1;
        }
        single_object = true;
    }
    else if(uuid_string != "")
    {
        bool ok = single_obj_uuid.from_string(uuid_string);
        if(!ok)
        {
            msg_fatal("main", "could not convert given string into UUID key: " << uuid_string << eom);
            return 1;
        }
        single_object = true;
    }

    //open the file
    hid_t file_id = H5Fcreate(output_file.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    if (file_id < 0) 
    {
        std::cout<<"failed to create HDF5 file"<<std::endl;
        return 1;
    }

    //loop over all the objects in the container, and convert them 
    //into a meta-data .json file, and a flat binary file for the data table
    //get all the types in the container
    std::vector< MHO_UUID > type_ids;
    conStore.GetAllTypeUUIDs(type_ids);
    for(std::size_t tid=0; tid<type_ids.size(); tid++)
    {
        //get all the object uuids of this type
        std::vector< MHO_UUID > obj_ids;
        conStore.GetAllObjectUUIDsOfType(type_ids[tid], obj_ids);
        for(std::size_t oid=0; oid<obj_ids.size(); oid++)
        {
            MHO_UUID obj_uuid = obj_ids[oid];
            //bail out on this one if only a single object requested, and this is not a match
            if(single_object && obj_uuid != single_obj_uuid){continue;}

            std::string shortname = conStore.GetShortName(obj_uuid);
            std::string obj_ident = obj_uuid.as_string();
            
            //convert the selected object to json
            mho_json obj_json;
            
            //needed to extract raw table data (if we can)
            std::size_t rank;
            const char* raw_data;
            std::size_t raw_data_byte_size;
            std::string raw_data_descriptor;
            std::string dataset_name = obj_ident; //shortname + "." + obj_ident;
            conInter.ConvertObjectInStoreToJSONAndRaw(conStore, obj_uuid, obj_json, rank, raw_data, raw_data_byte_size, raw_data_descriptor, detail);

            if(raw_data != nullptr && raw_data_byte_size != 0 && raw_data_descriptor != "")
            {
                herr_t status = write_to_hdf5file(file_id, rank, raw_data, raw_data_byte_size, dataset_name, raw_data_descriptor, obj_json);
                std::cout<<"object: "<<obj_ident<<" status = "<<status<<std::endl;
            }
            
            if(raw_data_descriptor == "tags")
            {
                std::cout<< "tags object not yet supported" << std::endl;
            }
        }
    }


    // Close file
    H5Fclose(file_id);
    std::cout<< "HDF5 file created successfully with dataset and metadata."<< std::endl;
    return 0;


    msg_info("main", "done" << eom);

    return 0;
}
