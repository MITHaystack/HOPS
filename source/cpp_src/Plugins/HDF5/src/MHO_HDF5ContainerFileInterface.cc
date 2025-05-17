#include "MHO_HDF5ContainerFileInterface.hh"


namespace hops
{

int MHO_HDF5ContainerFileInterface::ConvertStoreToHDF5(MHO_ContainerStore& store, std::string hdf5_filename)
{
    //open the file
    hid_t file_id = H5Fcreate(hdf5_filename.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    if(file_id < 0) 
    {
        std::cout<<"failed to create HDF5 file"<<std::endl;
        return 1;
    }

    hid_t group_id = H5Gcreate(file_id, fGroupPrefix.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if(group_id < 0) 
    {
        std::cout<<"failed to create HDF5 group"<<std::endl;
        return 1;
    }

    std::vector< MHO_UUID > type_ids;
    store.GetAllTypeUUIDs(type_ids);

    for(auto it = type_ids.begin(); it != type_ids.end(); it++)
    {
        auto converter = fHDF5ConverterMap.find(*it);
        if(converter != fHDF5ConverterMap.end())
        {
            std::vector< MHO_UUID > obj_ids;
            store.GetAllObjectUUIDsOfType(*it, obj_ids);
            for(auto it2 = obj_ids.begin(); it2 != obj_ids.end(); it2++)
            {
                MHO_Serializable* obj = store.GetObject(*it2);
                std::string shortname = store.GetShortName(*it2);
                if(obj != nullptr)
                {
                    std::cout<<"shortname = "<<shortname<<std::endl;
                    std::string category_prefix = fGroupPrefix + "/" + shortname;

                    if( H5Lexists(file_id, category_prefix.c_str(), H5P_DEFAULT) == 0) 
                    {
                        hid_t cat_id = H5Gcreate(file_id, category_prefix.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
                        if(group_id < 0) 
                        {
                            std::cout<<"failed to create HDF5 group"<<std::endl;
                            return 1;
                        }

                        //grab the meta-data labels, etc.
                        mho_json obj_mdata;
                        ConvertObjectInStoreToJSON(store, *it2, obj_mdata, eJSONWithLabels);
                        std::string obj_uuid = it2->as_string();

                        converter->second->SetObjectToConvert(obj);
                        converter->second->SetObjectMetaData(obj_mdata[obj_uuid]);
                        // converter->second->WriteToHDF5File(file_id, fGroupPrefix);
                        converter->second->WriteToHDF5File(file_id, category_prefix);

                        H5Gclose(cat_id);
                    }
                }
            }
        }
    }

    //close group 
    H5Gclose(group_id);

    //close file
    H5Fclose(file_id);
    std::cout<< "HDF5 file created successfully with dataset and metadata."<< std::endl;
    return 0;
};


} // namespace hops
