#ifndef MHO_PyScanStoreInterface_HH__
#define MHO_PyScanStoreInterface_HH__

#include "MHO_ContainerDictionary.hh"
#include "MHO_ContainerDefinitions.hh"

#include "MHO_ScanDataStore.hh"
#include "MHO_ContainerStore.hh"
#include "MHO_PyTableContainer.hh"

//need these extras to be able to translate between nl:json and py:dict or py::object
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include "pybind11_json/pybind11_json.hpp"
namespace py = pybind11;
namespace nl = nlohmann;
using namespace pybind11::literals;


/*
*@file: MHO_PyScanStoreInterface.hh
*@class: MHO_PyScanStoreInterface
*@author: J. Barrett
*@email: barrettj@mit.edu
*@date: Fri Sep 15 10:03:38 PM EDT 2023
*@brief:
*/

namespace hops
{


class MHO_PyScanStoreInterface
{
    public:

        MHO_PyScanStoreInterface(){};
        virtual ~MHO_PyScanStoreInterface()
        {
            Clear();
        };

        void SetDirectory(std::string dir){fScanStore.SetDirectory(dir);}
        void Initialize(){fScanStore.Initialize();}; 
        bool IsValid(){return fScanStore.IsValid();}; 
        bool IsBaselinePresent(std::string bl){return fScanStore.IsBaselinePresent(bl);}
        bool IsStationPresent(std::string st){return fScanStore.IsStationPresent(st);}

        std::size_t GetNBaselines(){return fScanStore.GetNBaselines();};
        std::size_t GetNStations(){return fScanStore.GetNStations();};

        std::vector< std::string > GetBaselinesPresent() const {return fScanStore.GetBaselinesPresent();} 
        std::vector< std::string > GetStationsPresent() const { return fScanStore.GetStationsPresent();}

        //retieve file data (root, baseline, station)
        py::dict GetRootFileData(){return fScanStore.GetRootFileData();};
        std::string GetRootFileBasename(){return fScanStore.GetRootFileBasename();}
        std::string GetBaselineFilename(std::string baseline){return fScanStore.GetBaselineFilename(baseline);}
        std::string GetStationFilename(std::string station){return fScanStore.GetStationFilename(station);}

        int LoadBaseline(std::string baseline) //perhas we should have an optional force-reload parameter?
        {
            auto it = fBaselineContainers.find(baseline);
            if(it != fBaselineContainers.end() ){return 1;} //already loaded 

            MHO_ContainerStore* con = new MHO_ContainerStore();
            fScanStore.LoadBaseline(baseline, con);
            MHO_PyContainerStoreInterface* inter = new MHO_PyContainerStoreInterface(con);
            fBaselineContainers[baseline] = std::make_pair(con, inter);
            return 0;
        }

        int LoadStation(std::string station)
        {
            auto it = fStationContainers.find(station);
            if(it != fStationContainers.end() ){return 1;} //already loaded 

            MHO_ContainerStore* con = new MHO_ContainerStore();
            fScanStore.LoadStation(station, con);
            MHO_PyContainerStoreInterface* inter = new MHO_PyContainerStoreInterface(con);
            fStationContainers[station] = std::make_pair(con, inter);
            return 0;
        }

        std::vector< std::string > GetBaselinesLoaded() const 
        {
            std::vector< std::string > baselines;
            for(auto it = fBaselineContainers.begin(); it != fBaselineContainers.end(); it++)
            {
                baselines.push_back(it->first);
            }
            return baselines;
        }
 
        std::vector< std::string > GetStationsLoaded() const 
        {
            std::vector< std::string > stations;
            for(auto it = fStationContainers.begin(); it != fStationContainers.end(); it++)
            {
                stations.push_back(it->first);
            }
            return stations;
        }

        MHO_PyContainerStoreInterface& GetBaselineData(std::string baseline)
        {
            auto it = fBaselineContainers.find(baseline);
            if(it != fBaselineContainers.end() )
            {
                return *(it->second.second);
            }
            //TODO FIXME ...this should not be a fatal error, return and empty container if not loaded
            msg_fatal("python_bindings", "fatal error, baseline:"<<baseline<<" not loaded "<<eom);
            std::exit(1);
        }



        //deletes all loaded containers and resets the state for another scan.
        void Clear()
        {
            for(auto it = fBaselineContainers.begin(); it != fBaselineContainers.end(); it++)
            {
                delete it->second.second;
                delete it->second.first;
            }
            for(auto it = fStationContainers.begin(); it != fStationContainers.end(); it++)
            {
                delete it->second.second;
                delete it->second.first;
            }
            fBaselineContainers.clear();
            fStationContainers.clear();
        }

    private:

        //provides directory/data-loading interface
        MHO_ScanDataStore fScanStore;

        std::map< std::string, std::pair< MHO_ContainerStore*, MHO_PyContainerStoreInterface* > > fBaselineContainers;
        std::map< std::string, std::pair< MHO_ContainerStore*, MHO_PyContainerStoreInterface* > > fStationContainers;

};

void
DeclarePyScanStoreInterface(py::module &m, std::string pyclass_name)
{
    py::class_< MHO_PyScanStoreInterface >(m, pyclass_name.c_str() )
        .def(py::init<>())
        .def("SetDirectory", &hops::MHO_PyScanStoreInterface::SetDirectory)
        .def("Initialize", &hops::MHO_PyScanStoreInterface::Initialize)
        .def("IsValid", &hops::MHO_PyScanStoreInterface::IsValid)
        .def("IsBaselinePresent", &hops::MHO_PyScanStoreInterface::IsBaselinePresent)
        .def("IsBaselinePresent", &hops::MHO_PyScanStoreInterface::IsStationPresent)
        .def("GetNBaselines", &hops::MHO_PyScanStoreInterface::GetNBaselines)
        .def("GetNStations", &hops::MHO_PyScanStoreInterface::GetNStations)
        .def("GetBaselinesPresent", &hops::MHO_PyScanStoreInterface::GetBaselinesPresent)
        .def("GetStationsPresent", &hops::MHO_PyScanStoreInterface::GetStationsPresent)
        .def("GetRootFileData", &hops::MHO_PyScanStoreInterface::GetRootFileData)
        .def("GetRootFileBasename", &hops::MHO_PyScanStoreInterface::GetRootFileBasename)
        .def("GetBaselineFilename", &hops::MHO_PyScanStoreInterface::GetBaselineFilename)
        .def("GetStationFilename", &hops::MHO_PyScanStoreInterface::GetStationFilename)
        .def("LoadBaseline", &hops::MHO_PyScanStoreInterface::LoadBaseline)
        .def("LoadStation", &hops::MHO_PyScanStoreInterface::LoadStation)
        .def("GetBaselinesLoaded", &hops::MHO_PyScanStoreInterface::GetBaselinesLoaded)
        .def("GetStationsLoaded", &hops::MHO_PyScanStoreInterface::GetStationsLoaded)
        .def("GetBaselineData", &hops::MHO_PyScanStoreInterface::GetBaselineData)
        .def("Clear", &hops::MHO_PyScanStoreInterface::Clear);

}

}//end of namespace

#endif /* end of include guard: MHO_PyScanStoreInterface */
