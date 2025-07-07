#ifndef MHO_FringeData_HH__
#define MHO_FringeData_HH__

//global messaging util
#include "MHO_Message.hh"

//data/config passing classes
#include "MHO_ContainerStore.hh"
#include "MHO_ParameterStore.hh"
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

/**
 * @brief Class MHO_FringeData
 */
class MHO_FringeData
{
    public:
        MHO_FringeData(){};
        virtual ~MHO_FringeData(){};

        /**
         * @brief Getter for parameter store
         * 
         * @return Pointer to MHO_ParameterStore
         */
        MHO_ParameterStore* GetParameterStore() { return &fParameterStore; }

        /**
         * @brief Getter for container store
         * 
         * @return MHO_ContainerStore& - Reference to the container store.
         */
        MHO_ContainerStore* GetContainerStore() { return &fContainerStore; }

        /**
         * @brief Getter for scan data store
         * 
         * @return Pointer to MHO_ScanDataStore
         */
        MHO_ScanDataStore* GetScanDataStore() { return &fScanStore; }

        //should we expose these?
        /**
         * @brief Getter for vex
         * 
         * @return The root file data as an mho_json object.
         */
        mho_json GetVex() const { return fScanStore.GetRootFileData(); }

        /**
         * @brief getter for the control format 
         * 
         * @return Reference to mho_json object representing control format
         */
        mho_json& GetControlFormat() { return fControlFormat; }

        /**
         * @brief Getter for parsed control statements
         * 
         * @return Reference to mho_json object containing control statements.
         */
        mho_json& GetControlStatements() { return fControlStatements; }

        //TODO remove this hack in favor of 'plotting'/'output' visitors
        /**
         * @brief Getter for plot data
         * 
         * @return Reference to mho_json object containing plot data
         */
        mho_json& GetPlotData() { return fPlotData; }

        //write data objects to output file...perhaps we may want to move this elsewhere?
        /**
         * @brief Writes output data to disk with a temporary unique name and renames it afterwards.
         * 
         * @return Returns an integer value indicating success or failure.
         */
        int WriteOutput();

    protected:
        /**
         * @brief Writes data objects to a file with given filename.
         * 
         * @param filename Filename to write data objects.
         * @return 0 on success, error code otherwise.
         */
        int WriteDataObjects(std::string filename);

        /**
         * @brief Constructs a fringe file name string from given parameters.
         * 
         * @param directory The directory path where the fringe file will be located.
         * @param baseline Baseline identifier for the fringe file.
         * @param ref_station Reference station identifier.
         * @param rem_station Remote station identifier.
         * @param frequency_group Frequency group identifier.
         * @param polprod Polarization product identifier.
         * @param root_code Root code identifier.
         * @param seq_no Sequence number for the fringe file.
         * @return A string representing the constructed fringe file name.
         */
        std::string ConstructFrngFileName(const std::string directory, const std::string& baseline,
                                          const std::string& ref_station, const std::string& rem_station,
                                          const std::string& frequency_group, const std::string& polprod,
                                          const std::string& root_code, int seq_no);

        /**
         * @brief Constructs a temporary file name using provided parameters and concatenation.
         * 
         * @param directory The directory where the file will be located.
         * @param baseline Baseline identifier for the file.
         * @param ref_station Reference station identifier.
         * @param rem_station Remote station identifier.
         * @param frequency_group Frequency group identifier.
         * @param polprod Polarization-production identifier.
         * @param root_code Root code identifier.
         * @param temp_id Temporary ID for the file.
         * @return The constructed temporary file name as a string.
         */
        std::string ConstructTempFileName(const std::string directory, const std::string& baseline,
                                          const std::string& ref_station, const std::string& rem_station,
                                          const std::string& frequency_group, const std::string& polprod,
                                          const std::string& root_code, const std::string& temp_id);

        //data objects
        MHO_ParameterStore fParameterStore; //stores various parameters using string keys
        MHO_ScanDataStore fScanStore;       //provides access to data associated with this scan
        MHO_ContainerStore fContainerStore; //stores data containers for in-use data

        // mho_json fVexInfo;
        mho_json fControlFormat;
        mho_json fControlStatements;

        //plot data storage
        mho_json fPlotData;
};

} // namespace hops

#endif /*! end of include guard: MHO_FringeData_HH__ */
