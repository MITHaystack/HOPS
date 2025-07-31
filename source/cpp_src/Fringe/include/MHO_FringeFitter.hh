#ifndef MHO_FringeFitter_HH__
#define MHO_FringeFitter_HH__

//global messaging util
#include "MHO_Message.hh"

//data/config passing classes
#include "MHO_ContainerStore.hh"
#include "MHO_FringeData.hh"
#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_OperatorToolbox.hh"
#include "MHO_ParameterStore.hh"

//needed to read hops files and extract objects from scan dir
#include "MHO_ScanDataStore.hh"

//initialization
#include "MHO_OperatorBuilderManager.hh"
#include "MHO_ParameterConfigurator.hh"
#include "MHO_ParameterManager.hh"

namespace hops
{

/*!
 *@file MHO_FringeFitter.hh
 *@class MHO_FringeFitter
 *@author J. Barrettj - barrettj@mit.edu
 *@date Tue Sep 19 16:26:35 2023 -0400
 *@brief Abstract base class for a basic fringe fitter
 */

//forward declare visitor
/**
 * @brief Class MHO_FringeFitterVisitor
 */
class MHO_FringeFitterVisitor;

/**
 * @brief Class MHO_FringeFitter
 */
class MHO_FringeFitter
{
    public:
        MHO_FringeFitter(MHO_FringeData* data): fFringeData(data)
        {
            fParameterStore = data->GetParameterStore();
            fScanStore = data->GetScanDataStore();
            fContainerStore = data->GetContainerStore();
            fOperatorBuildManager = nullptr;
        };

        virtual ~MHO_FringeFitter() { delete fOperatorBuildManager; };

        /**
         * @brief Getter for fringe data
         * 
         * @return MHO_FringeData* - Pointer to the fringe data.
         */
        MHO_FringeData* GetFringeData() { return fFringeData; }

        /**
         * @brief Getter for parameter store
         * 
         * @return Pointer to the MHO_ParameterStore object
         */
        MHO_ParameterStore* GetParameterStore() { return fParameterStore; }

        /**
         * @brief Getter for container store
         * 
         * @return MHO_ContainerStore*
         */
        MHO_ContainerStore* GetContainerStore() { return fContainerStore; }

        /**
         * @brief Getter for operator toolbox
         * 
         * @return Pointer to MHO_OperatorToolbox
         */
        MHO_OperatorToolbox* GetOperatorToolbox() { return &fOperatorToolbox; }

        //should we expose these?
        /**
         * @brief Getter for vex (ovex) data as JSON object
         * 
         * @return mho_json containing root file data
         */
        mho_json GetVex() { return fScanStore->GetRootFileData(); }

        /**
         * @brief Getter for scan data store
         * 
         * @return MHO_ScanDataStore* - Pointer to the scan data store.
         */
        MHO_ScanDataStore* GetScanDataStore() { return fScanStore; }

        /**
         * @brief Getter for ther operator build manager - only valid after 'Configure' is called
         * 
         * @return MHO_OperatorBuilderManager*
         */
        MHO_OperatorBuilderManager* GetOperatorBuildManager() { return fOperatorBuildManager; }

        //basic run scheme: configure, init, then while(!IsFinished() ){ pre-run, run, post-run }, then finalize
        /**
         * @brief Function Configure
         * @note This is a virtual function.
         */
        virtual void Configure() = 0;
        
        //TODO add a 'configure extension' function using visitor pattern to add things like pybind11/opencl etc.
        /**
         * @brief Function Initialize
         * @note This is a virtual function.
         */
        virtual void Initialize() = 0;
        
        /**
         * @brief Function PreRun
         * @note This is a virtual function.
         */
        virtual void PreRun() = 0;
        
        /**
         * @brief Function Run
         * 
         * @return Return value (void Pre)
         * @note This is a virtual function.
         */
        virtual void Run() = 0;
        
        /**
         * @brief Function PostRun
         * @note This is a virtual function.
         */
        virtual void PostRun() = 0;
        
        /**
         * @brief Function IsFinished
         * 
         * @return Return value (bool)
         * @note This is a virtual function.
         */
        virtual bool IsFinished() = 0;
        
        /**
         * @brief Function Finalize
         * @note This is a virtual function.
         */
        virtual void Finalize() = 0;

        /**
         * @brief Function Accept: accept a visitor...pure virtual, must be implemented in derived class
         * 
         * @param visitor (MHO_FringeFitterVisitor*)
         * @note This is a (pure) virtual function.
         */
        virtual void Accept(MHO_FringeFitterVisitor* visitor) = 0;

    protected:
        
        //optional caching mechanism (stash the configured visibilities/weights)
        //before they are modified by flagging/calibration/prefit operators
        virtual void Cache(){}; 
        virtual void Refresh(){};
        
        //data objects
        MHO_FringeData* fFringeData;

        MHO_ParameterStore* fParameterStore; //stores various parameters using string keys
        MHO_ScanDataStore* fScanStore;       //provides access to data associated with this scan
        MHO_ContainerStore* fContainerStore; //stores data containers for in-use data

        MHO_OperatorToolbox fOperatorToolbox; //stores the data operator objects

        //configuration/initialization managers
        MHO_OperatorBuilderManager* fOperatorBuildManager;
};


/**
 * @brief Class MHO_FringeFitterVisitor
 */
class MHO_FringeFitterVisitor
{
    public:
        MHO_FringeFitterVisitor(){}
        virtual ~MHO_FringeFitterVisitor(){};

        //pure virtual
        /**
         * @brief Function Visit
         * 
         * @param fitter (MHO_FringeFitter*)
         * @note This is a (pure) virtual function.
         */
        virtual void Visit(MHO_FringeFitter* fitter) = 0;
};



} // namespace hops

#endif /*! end of include guard: MHO_FringeFitter_HH__ */
