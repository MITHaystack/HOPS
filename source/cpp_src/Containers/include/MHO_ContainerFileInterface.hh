#ifndef MHO_ContainerFileInterface_HH__
#define MHO_ContainerFileInterface_HH__

/*
*@file: MHO_ContainerFileInterface.hh
*@class: MHO_ContainerFileInterface
*@author: J. Barrett
*@email: barrettj@mit.edu
*@date:
*@brief:
*/

#include "MHO_Message.hh"
#include "MHO_ContainerDictionary.hh"
#include "MHO_BinaryFileInterface.hh"

#include "MHO_ChannelizedVisibilities.hh"
#include "MHO_StationCoordinates.hh"
#include "MHO_Visibilities.hh"

#include "MHO_ContainerStore.hh"


namespace hops 
{

class MHO_ContainerFileInterface: public MHO_ContainerDictionary
{
    public:

        MHO_ContainerFileInterface();
        virtual ~MHO_ContainerFileInterface();

        void SetFilename(std::string filename);

        //index file optional, if we don't have an index file, the regular file will be
        //read in 2-passes, first to extract the keys, then to extract the objects
        //likewise, when writing a storerary to file, if there is no index file specified, none will be created
        void SetIndexFileName(std::string index_filename);

        void PopulateStoreFromFile(MHO_ContainerStore& store);
        void WriteStoreToFile(MHO_ContainerStore& store);

        void ConvertStoreToJSON(MHO_ContainerStore& store, json& json_obj, int level_of_detail=eJSONBasic);

        void ConvertObjectInStoreToJSON(MHO_ContainerStore& store,  const MHO_UUID& obj_uuid, json& json_obj, int level_of_detail=eJSONBasic);

    private:

        std::string fFilename;
        std::string fIndexFilename;
        MHO_BinaryFileInterface fFileInterface;

};

} //end namespace

#endif /* end of include guard: MHO_ContainerFileInterface */