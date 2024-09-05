#ifndef MHO_FringeData_HH__
#define MHO_FringeData_HH__


//global messaging util
#include "MHO_Message.hh"

//data/config passing classes
#include "MHO_ParameterStore.hh"
#include "MHO_ContainerStore.hh"
//needed to read hops files and extract objects from scan dir
#include "MHO_ScanDataStore.hh"

#include "MHO_JSONHeaderWrapper.hh"

namespace hops
{


/*!
*@file MHO_FringeData.hh
*@class MHO_FringeData
*@author
*@date Sun Feb 4 20:25:51 2024 -0500
*@brief simple class to contain fringe data objects
*/

class MHO_FringeData
{
    public:

        MHO_FringeData(){};
        virtual ~MHO_FringeData(){};

        MHO_ParameterStore* GetParameterStore(){return &fParameterStore;}
        MHO_ContainerStore* GetContainerStore(){return &fContainerStore;}
        MHO_ScanDataStore* GetScanDataStore(){return &fScanStore;}

        //should we expose these?
        mho_json GetVex() const {return fScanStore.GetRootFileData();}

        //access to the control format and parsed control statements
        mho_json& GetControlFormat(){return fControlFormat;}
        mho_json& GetControlStatements(){return fControlStatements;}

        //TODO remove this hack in favor of 'plotting'/'output' visitors
        mho_json& GetPlotData(){return fPlotData;}

        //write data objects to output file...perhaps we may want to move this elsewhere?
        int WriteOutput();

    protected:

        int WriteDataObjects(std::string filename);

        std::string ConstructFrngFileName(const std::string directory,
                                          const std::string& baseline,
                                          const std::string& ref_station,
                                          const std::string& rem_station,
                                          const std::string& frequency_group,
                                          const std::string& polprod,
                                          const std::string& root_code,
                                          const std::string& temp_id);

        //data objects
        MHO_ParameterStore fParameterStore; //stores various parameters using string keys
        MHO_ScanDataStore fScanStore; //provides access to data associated with this scan
        MHO_ContainerStore fContainerStore; //stores data containers for in-use data

        // mho_json fVexInfo;
        mho_json fControlFormat;
        mho_json fControlStatements;

        //plot data storage
        mho_json fPlotData;

};

}

#endif /*! end of include guard: MHO_FringeData_HH__ */
