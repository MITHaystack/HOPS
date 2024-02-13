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

        MHO_PyScanStoreInterface()
        {
            fInitialized = false;
            fEmptyContainer = new MHO_PyContainerStoreInterface(nullptr);
        };

        virtual ~MHO_PyScanStoreInterface()
        {
            Clear();
            delete fEmptyContainer;
        };

        void SetDirectory(std::string dir)
        {
            fScanStore.SetDirectory(dir);
            fInitialized = false;
        }

        bool Initialize()
        {
            fInitialized = fScanStore.Initialize();
            return fInitialized;
        }; 

        bool IsValid(){return fInitialized && fScanStore.IsValid();};
 
        bool IsBaselinePresent(std::string bl){return fScanStore.IsBaselinePresent(bl);}
        bool IsStationPresent(std::string st){return fScanStore.IsStationPresent(st);}
        bool IsFringePresent(std::string fr){return fScanStore.IsFringePresent(fr);}

        std::size_t GetNBaselines(){return fScanStore.GetNBaselines();}
        std::size_t GetNStations(){return fScanStore.GetNStations();}
        std::size_t GetNFringes(){return fScanStore.GetNFringes();}

        std::vector< std::string > GetBaselinesPresent() const {return fScanStore.GetBaselinesPresent();} 
        std::vector< std::string > GetStationsPresent() const { return fScanStore.GetStationsPresent();}
        std::vector< std::string > GetFringesPresent() const {return fScanStore.GetFringesPresent();}

        //retieve file data (root, baseline, station)
        py::dict GetRootFileData(){return fScanStore.GetRootFileData();};
        std::string GetRootFileBasename(){return fScanStore.GetRootFileBasename();}
        std::string GetBaselineFilename(std::string baseline){return fScanStore.GetBaselineFilename(baseline);}
        std::string GetStationFilename(std::string station){return fScanStore.GetStationFilename(station);}
        std::string GetFringeFilename(std::string fringe){return fScanStore.GetFringeFilename(fringe);}

        int LoadBaseline(std::string baseline) //perhaps we should have an optional force-reload parameter?
        {
            if( !IsValid() ){return -1;} //not initialized, bail out

            if( !IsBaselinePresent(baseline) ){return -2;} //not present in this scan

            auto it = fBaselineContainers.find(baseline);
            if(it != fBaselineContainers.end() ){return 1;} //ok, but already loaded 

            MHO_ContainerStore* con = new MHO_ContainerStore();
            fScanStore.LoadBaseline(baseline, con);
            MHO_PyContainerStoreInterface* inter = new MHO_PyContainerStoreInterface(con);
            fBaselineContainers[baseline] = std::make_pair(con, inter);
            return 0; //ok
        }

        bool IsBaselineLoaded(std::string baseline)
        {
            if(!fInitialized){return false;}
            auto it = fBaselineContainers.find(baseline);
            if(it != fBaselineContainers.end() ){return true;}
            return false;
        }

        int LoadStation(std::string station)
        {
            if( !IsValid() ){return -1;} //not initialized, bail out

            if( !IsStationPresent(station) ){return -2;} //not present in this scan

            auto it = fStationContainers.find(station);
            if(it != fStationContainers.end() ){return 1;} //ok, but already loaded 

            MHO_ContainerStore* con = new MHO_ContainerStore();
            fScanStore.LoadStation(station, con);
            MHO_PyContainerStoreInterface* inter = new MHO_PyContainerStoreInterface(con);
            fStationContainers[station] = std::make_pair(con, inter);
            return 0; //ok
        }

        bool IsStationLoaded(std::string station)
        {
            if(!fInitialized){return false;}
            auto it = fStationContainers.find(station);
            if(it != fStationContainers.end() ){return true;}
            return false;
        }


        int LoadFringe(std::string fringe)
        {
            if( !IsValid() ){return -1;} //not initialized, bail out

            if( !IsFringePresent(fringe) ){return -2;} //not present in this scan

            auto it = fFringeContainers.find(fringe);
            if(it != fFringeContainers.end() ){return 1;} //ok, but already loaded 

            MHO_ContainerStore* con = new MHO_ContainerStore();
            fScanStore.LoadFringe(fringe, con);
            MHO_PyContainerStoreInterface* inter = new MHO_PyContainerStoreInterface(con);
            fFringeContainers[fringe] = std::make_pair(con, inter);
            return 0; //ok
        }

        bool IsFringeLoaded(std::string fringe)
        {
            if(!fInitialized){return false;}
            auto it = fFringeContainers.find(fringe);
            if(it != fFringeContainers.end() ){return true;}
            return false;
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

        std::vector< std::string > GetFringesLoaded() const 
        {
            std::vector< std::string > fringes;
            for(auto it = fFringeContainers.begin(); it != fFringeContainers.end(); it++)
            {
                fringes.push_back(it->first);
            }
            return fringes;
        }


        MHO_PyContainerStoreInterface& GetBaselineData(std::string baseline)
        {
            auto it = fBaselineContainers.find(baseline);
            if(it != fBaselineContainers.end() )
            {
                return *(it->second.second);
            }
            //should not be able to reach here from python interface (via lambda)
            //but since we must have a return value, return an empty container
            py::print("could not find baseline: ", baseline, ", returning empty container ");
            return *fEmptyContainer;
        }

        MHO_PyContainerStoreInterface& GetStationData(std::string station)
        {
            auto it = fStationContainers.find(station);
            if(it != fStationContainers.end() )
            {
                return *(it->second.second);
            }
            //should not be able to reach here from python interface (via lambda)
            //but since we must have a return value, return an empty container
            py::print("could not find station: ",station,", returning empty container ");
            return *fEmptyContainer;
        }

        MHO_PyContainerStoreInterface& GetFringeData(std::string fringe)
        {
            auto it = fFringeContainers.find(fringe);
            if(it != fFringeContainers.end() )
            {
                return *(it->second.second);
            }
            //should not be able to reach here from python interface (via lambda)
            //but since we must have a return value, return an empty container
            py::print("could not find fringe: ",fringe,", returning empty container ");
            return *fEmptyContainer;
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
            for(auto it = fFringeContainers.begin(); it != fFringeContainers.end(); it++)
            {
                delete it->second.second;
                delete it->second.first;
            }
            fBaselineContainers.clear();
            fStationContainers.clear();
            fFringeContainers.clear();
            fInitialized = false;
        }

    private:

        //provides directory/data-loading interface
        MHO_ScanDataStore fScanStore;
        bool fInitialized;
    
        //an empty container so we can a reference under certain error conditions
        MHO_PyContainerStoreInterface* fEmptyContainer;

        std::map< std::string, std::pair< MHO_ContainerStore*, MHO_PyContainerStoreInterface* > > fBaselineContainers;
        std::map< std::string, std::pair< MHO_ContainerStore*, MHO_PyContainerStoreInterface* > > fStationContainers;
        std::map< std::string, std::pair< MHO_ContainerStore*, MHO_PyContainerStoreInterface* > > fFringeContainers;

};

void
DeclarePyScanStoreInterface(py::module &m, std::string pyclass_name)
{
    py::class_< MHO_PyScanStoreInterface >(m, pyclass_name.c_str() )
        .def(py::init<>())
        .def("set_directory", &hops::MHO_PyScanStoreInterface::SetDirectory)
        .def("initialize", &hops::MHO_PyScanStoreInterface::Initialize)
        .def("is_valid", &hops::MHO_PyScanStoreInterface::IsValid)
        .def("has_baseline", &hops::MHO_PyScanStoreInterface::IsBaselinePresent)
        .def("has_station", &hops::MHO_PyScanStoreInterface::IsStationPresent)
        .def("has_fringe", &hops::MHO_PyScanStoreInterface::IsFringePresent)
        .def("is_baseline_loaded", &hops::MHO_PyScanStoreInterface::IsBaselineLoaded)
        .def("is_station_loaded", &hops::MHO_PyScanStoreInterface::IsStationLoaded)
        .def("is_fringe_loaded", &hops::MHO_PyScanStoreInterface::IsFringeLoaded)
        .def("get_nbaselines", &hops::MHO_PyScanStoreInterface::GetNBaselines)
        .def("get_nstations", &hops::MHO_PyScanStoreInterface::GetNStations)
        .def("get_nfringes", &hops::MHO_PyScanStoreInterface::GetNFringes)
        .def("get_baseline_list", &hops::MHO_PyScanStoreInterface::GetBaselinesPresent)
        .def("get_station_list", &hops::MHO_PyScanStoreInterface::GetStationsPresent)
        .def("get_fringe_list", &hops::MHO_PyScanStoreInterface::GetFringesPresent)
        .def("get_rootfile_data", &hops::MHO_PyScanStoreInterface::GetRootFileData)
        .def("get_rootfile_basename", &hops::MHO_PyScanStoreInterface::GetRootFileBasename)
        .def("get_baseline_filename", &hops::MHO_PyScanStoreInterface::GetBaselineFilename)
        .def("get_station_filename", &hops::MHO_PyScanStoreInterface::GetStationFilename)
        .def("get_fringe_filename", &hops::MHO_PyScanStoreInterface::GetFringeFilename)
        .def("load_baseline", &hops::MHO_PyScanStoreInterface::LoadBaseline)
        .def("load_station", &hops::MHO_PyScanStoreInterface::LoadStation)
        .def("load_fringe", &hops::MHO_PyScanStoreInterface::LoadFringe)
        .def("get_baselines_loaded", &hops::MHO_PyScanStoreInterface::GetBaselinesLoaded)
        .def("get_stations_loaded", &hops::MHO_PyScanStoreInterface::GetStationsLoaded)
        .def("get_fringes_loaded", &hops::MHO_PyScanStoreInterface::GetFringesLoaded)
        .def("get_baseline_data", //lambda for returing either baseline data or None-type
            [=](MHO_PyScanStoreInterface& m, std::string baseline) -> py::object 
            {
                if( m.IsBaselineLoaded(baseline) ) 
                {
                    return py::cast( m.GetBaselineData(baseline) );
                }
                py::print( "data for baseline ",baseline," either it has not been loaded or it does not exist in this scan.");
                return py::object(py::cast(nullptr));
            })
        .def("get_station_data", //lambda for returing either station data or None-type
            [=](MHO_PyScanStoreInterface& m, std::string station) -> py::object 
            {
                if( m.IsStationLoaded(station) ) 
                {
                    return py::cast( m.GetStationData(station) );
                }
                py::print( "data for station ", station, " either it has not been loaded or it does not exist in this scan.");
                return py::object(py::cast(nullptr));
            })
        .def("get_fringe_data", //lambda for returing either fringe data or None-type
            [=](MHO_PyScanStoreInterface& m, std::string fringe) -> py::object 
            {
                if( m.IsFringeLoaded(fringe) ) 
                {
                    return py::cast( m.GetFringeData(fringe) );
                }
                py::print( "data for fringe ", fringe, " either it has not been loaded or it does not exist in this scan.");
                return py::object(py::cast(nullptr));
            })
        .def("Clear", &hops::MHO_PyScanStoreInterface::Clear);

}

}//end of namespace

#endif /* end of include guard: MHO_PyScanStoreInterface */
