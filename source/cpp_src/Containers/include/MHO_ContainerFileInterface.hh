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

#include "MHO_ContainerLibrary.hh"


//space saving macros
#define CheckAndRead(TYPE,ID) if(MatchesType<TYPE>(ID)){return Read<TYPE>(); }



namespace hops 
{

class MHO_ContainerFileInterface: public MHO_ContainerDictionary
{
    public:

        MHO_ContainerFileInterface();
        virtual ~MHO_ContainerFileInterface();

        void SetFilename(std::string filename);

        //optional, if we don't have an index file, the regular file will be
        //read to extract the keys first, before extracting the objects
        void SetIndexFileName(std::string index_filename);

        void PopulateLibraryFromFile(MHO_ContainerLibrary& lib);

    private:

        std::string fFilename;
        std::string fIndexFilename;
        MHO_BinaryFileInterface fFileInterface;

};

} //end namespace

#endif /* end of include guard: MHO_ContainerFileInterface */