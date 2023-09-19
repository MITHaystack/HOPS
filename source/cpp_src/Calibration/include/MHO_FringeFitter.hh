#ifndef MHO_FringeFitter_HH__
#define MHO_FringeFitter_HH__

/*
*File: MHO_FringeFitter.hh
*Class: MHO_FringeFitter
*Author:
*Email:
*Date: Tue Sep 19 04:11:24 PM EDT 2023
*Description: Abstract base class for a basic fringe fitter
*/

//global messaging util
#include "MHO_Message.hh"

//data/config passing classes
#include "MHO_ParameterStore.hh"
#include "MHO_ContainerStore.hh"
#include "MHO_OperatorToolbox.hh"
#include "MHO_JSONHeaderWrapper.hh"

//needed to read hops files and extract objects from scan dir
#include "MHO_ScanDataStore.hh"

namespace hops 
{


class MHO_FringeFitter 
{
    public:
        MHO_FringeFitter();
        virtual ~MHO_FringeFitter();
        
        MHO_ParameterStore* GetParameterStore(){return &fParameterStore;}
        MHO_ContainerStore* GetContainerStore(){return &fContainerStore;}
        MHO_OperatorToolbox* GetOperatorToolbox(){return &fOperatorToolbox;}

        //should we expose these?
        mho_json GetVex(){return scanStore.GetRootFileData();} 
        MHO_ScanDataStore* GetScanDataStore(){return &fScanStore;}

        //basic run scheme: configure, init, while(!IsFinished() ){ pre-run, run, post-run }
        virtual void Configure() = 0;
        virtual void Initialize() = 0;
        virtual void PreRun() = 0;
        virtual void Run() = 0;
        virtual void PostRun() = 0;
        virtual bool IsFinished() = 0;

    private:
        /* data */
        
        //provide necessary objects for operation
        MHO_ParameterStore fParameterStore; //stores various parameters using string keys
        MHO_ScanDataStore fScanStore; //provides access to data associated with this scan
        MHO_ContainerStore fContainerStore; //stores data containers for in-use data
        MHO_OperatorToolbox fOperatorToolbox; //stores the data operator objects


};

}

#endif /* end of include guard: MHO_FringeFitter_HH__ */
