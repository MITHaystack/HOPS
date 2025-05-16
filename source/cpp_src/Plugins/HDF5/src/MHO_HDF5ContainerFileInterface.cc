#include "MHO_HDF5ContainerFileInterface.hh"


namespace hops
{

MHO_HDF5ContainerFileInterface::MHO_HDF5ContainerFileInterface():MHO_ContainerFileInterface(){};
MHO_HDF5ContainerFileInterface::~MHO_HDF5ContainerFileInterface(){};


int MHO_HDF5ContainerFileInterface::ConvertStoreToHDF5(MHO_ContainerStore& store, std::string hdf5_filename)
{
    //open the file
    hid_t file_id = H5Fcreate(hdf5_filename.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    if (file_id < 0) 
    {
        std::cout<<"failed to create HDF5 file"<<std::endl;
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
                if(obj != nullptr)
                {
                    converter->second->SetObjectToConvert(obj);
                    converter->second->WriteToHDF5File(file_id);
                }
            }
        }
    }

    // Close file
    H5Fclose(file_id);
    std::cout<< "HDF5 file created successfully with dataset and metadata."<< std::endl;
    return 0;
};


} // namespace hops
