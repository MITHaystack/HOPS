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

/**
 * @brief Class MHO_ContainerFileInterface - a MHO_ContainerDictionary which can read/write objects to file
 */
class MHO_ContainerFileInterface: public MHO_ContainerDictionary
{
    public:
        MHO_ContainerFileInterface();
        virtual ~MHO_ContainerFileInterface();

        /**
         * @brief Setter for filename
         * 
         * @param filename New filename to set as a std::string
         */
        void SetFilename(std::string filename);


        /**
         * @brief Setter for index file name
         * @details index file is optional, if we don't have an index file, the regular file will be
         * read in 2-passes, first to extract the keys, then to extract the objects
         * likewise, when writing a store to file, if there is no index file specified, none will be created
         * @param index_filename The new index file name to set
         */
        void SetIndexFileName(std::string index_filename);


        /**
         * @brief Populates a store from a file, optionally clearing it first.
         * @details currently this function reads the file keys and then the all the file objects
         * note: we may want to split this functionality so we can inspect the file first
         * and then only read the objects of interest
         * @param store Reference to MHO_ContainerStore object
         * @param do_clear_store Boolean flag indicating whether to clear the store before populating
         */
        void PopulateStoreFromFile(MHO_ContainerStore& store, bool do_clear_store = false);

        /**
         * @brief Writes object in a MHO_ContainerStore to file using MHO_BinaryFileInterface and factory map.
         * 
         * @param store Reference to MHO_ContainerStore containing objects to write.
         */
        void WriteStoreToFile(MHO_ContainerStore& store);

        /**
         * @brief Converts a container store to JSON representation with specified detail level.
         * 
         * @param store Reference to MHO_ContainerStore object containing data to convert.
         * @param json_obj Reference to mho_json object that will hold the converted JSON data.
         * @param level_of_detail Integer specifying the level of detail for the JSON conversion.
         */
        void ConvertStoreToJSON(MHO_ContainerStore& store, mho_json& json_obj, int level_of_detail = eJSONBasic);

        /**
         * @brief Converts a specific object in store to JSON representation at given detail level.
         * 
         * @param store Reference to MHO_ContainerStore for accessing objects and types.
         * @param obj_uuid UUID of the object to convert to JSON.
         * @param json_obj Reference to mho_json where converted object will be stored.
         * @param level_of_detail Detail level for converting the object.
         */
        void ConvertObjectInStoreToJSON(MHO_ContainerStore& store, const MHO_UUID& obj_uuid, mho_json& json_obj,
                                        int level_of_detail = eJSONBasic);
                                        
        //also provides access to the raw bytes of table container data (for hops2flat)
        /**
         * @brief Converts an object in store to JSON and raw data, providing access to raw bytes for hops2flat.
         * 
         * @param store Reference to MHO_ContainerStore for accessing objects
         * @param obj_uuid UUID of the object to convert
         * @param json_obj Output mho_json object containing converted JSON representation
         * @param rank Output rank of the converted object (specialization for RANK-0)
         * @param raw_data Output pointer to raw data bytes (null if not available)
         * @param raw_data_byte_size Output size of raw data in bytes
         * @param raw_data_descriptor Output string describing raw data
         * @param level_of_detail Input level of detail for conversion
         */
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
