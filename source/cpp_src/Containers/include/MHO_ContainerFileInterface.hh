#ifndef MHO_ContainerFileInterface_HH__
#define MHO_ContainerFileInterface_HH__

#include "MHO_BinaryFileInterface.hh"
#include "MHO_ContainerDictionary.hh"
#include "MHO_Message.hh"

#include "MHO_ContainerDefinitions.hh"
#include "MHO_ContainerStore.hh"

namespace hops
{

/*!
 *@file  MHO_ContainerFileInterface.hh
 *@class  MHO_ContainerFileInterface
 *@author  J. Barrett - barrettj@mit.edu
 *@date Sun Feb 13 15:48:42 2022 -0500
 *@brief
 */

class MHO_ContainerFileInterface: public MHO_ContainerDictionary
{
    public:
        MHO_ContainerFileInterface();
        virtual ~MHO_ContainerFileInterface();

        void SetFilename(std::string filename);

        //index file optional, if we don't have an index file, the regular file will be
        //read in 2-passes, first to extract the keys, then to extract the objects
        //likewise, when writing a store to file, if there is no index file specified, none will be created
        void SetIndexFileName(std::string index_filename);

        //currently this function reads the file keys and then the all the file objects
        //we may want to split this functionality so we can inspect the file first
        //and then only read the objects of interest
        void PopulateStoreFromFile(MHO_ContainerStore& store, bool do_clear_store = false);

        void WriteStoreToFile(MHO_ContainerStore& store);

        void ConvertStoreToJSON(MHO_ContainerStore& store, mho_json& json_obj, int level_of_detail = eJSONBasic);

        void ConvertObjectInStoreToJSON(MHO_ContainerStore& store, const MHO_UUID& obj_uuid, mho_json& json_obj,
                                        int level_of_detail = eJSONBasic);
                                        
        //also provides access to the raw bytes of table container data (for hops2flat)
        void ConvertObjectInStoreToJSONAndRaw(MHO_ContainerStore& store, 
                                        const MHO_UUID& obj_uuid,
                                        mho_json& json_obj,
                                        std::size_t& rank,
                                        const char*& raw_data,
                                        std::size_t& raw_data_byte_size,
                                        std::string& raw_data_descriptor,
                                        int level_of_detail = eJSONBasic);
                                        

    private:
        std::string fFilename;
        std::string fIndexFilename;
        MHO_BinaryFileInterface fFileInterface;
};

} // namespace hops

#endif /*! end of include guard: MHO_ContainerFileInterface */
