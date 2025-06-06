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
class MHO_FringeFitterVisitor;

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

        MHO_FringeData* GetFringeData() { return fFringeData; }

        MHO_ParameterStore* GetParameterStore() { return fParameterStore; }

        MHO_ContainerStore* GetContainerStore() { return fContainerStore; }

        MHO_OperatorToolbox* GetOperatorToolbox() { return &fOperatorToolbox; }

        //should we expose these?
        mho_json GetVex() { return fScanStore->GetRootFileData(); }

        MHO_ScanDataStore* GetScanDataStore() { return fScanStore; }

        //only valid after 'Configure' is called
        MHO_OperatorBuilderManager* GetOperatorBuildManager() { return fOperatorBuildManager; }

        //basic run scheme: configure, init, then while(!IsFinished() ){ pre-run, run, post-run }, then finalize
        virtual void Configure() = 0;
        //TODO add a 'configure extension' function using visitor pattern to add things like pybind11/opencl etc.
        virtual void Initialize() = 0;
        virtual void PreRun() = 0;
        virtual void Run() = 0;
        virtual void PostRun() = 0;
        virtual bool IsFinished() = 0;
        virtual void Finalize() = 0;

        //accept a visitor...pure virtual, must implement
        virtual void Accept(MHO_FringeFitterVisitor* visitor) = 0;

    protected:
        //data objects
        MHO_FringeData* fFringeData;

        MHO_ParameterStore* fParameterStore; //stores various parameters using string keys
        MHO_ScanDataStore* fScanStore;       //provides access to data associated with this scan
        MHO_ContainerStore* fContainerStore; //stores data containers for in-use data

        MHO_OperatorToolbox fOperatorToolbox; //stores the data operator objects

        //configuration/initialization managers
        MHO_OperatorBuilderManager* fOperatorBuildManager;
};


class MHO_FringeFitterVisitor
{
    public:
        MHO_FringeFitterVisitor(){}
        virtual ~MHO_FringeFitterVisitor(){};

        //pure virtual
        virtual void Visit(MHO_FringeFitter* fitter) = 0;
};



} // namespace hops

#endif /*! end of include guard: MHO_FringeFitter_HH__ */
