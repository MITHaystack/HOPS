#ifndef MHO_JlScanStoreInterface_HH__
#define MHO_JlScanStoreInterface_HH__

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "MHO_ContainerDefinitions.hh"
#include "MHO_ContainerDictionary.hh"
#include "MHO_ContainerStore.hh"
#include "MHO_JlContainerStoreInterface.hh"
#include "MHO_JlTableContainer.hh"
#include "MHO_ScanDataStore.hh"

#include "jlcxx/jlcxx.hpp"

namespace hops
{

/*!
 *@file  MHO_JlScanStoreInterface.hh
 *@class  MHO_JlScanStoreInterface
 *@author  J. Barrett - barrettj@mit.edu
 *@brief Julia (CxxWrap) bindings for MHO_ScanDataStore.
 */

class MHO_JlScanStoreInterface
{
    public:
        //! Called from the C++ side to wrap an existing ScanDataStore.
        MHO_JlScanStoreInterface(MHO_ScanDataStore* scan_store)
            : fScanStore(scan_store), fIsOwned(false), fInitialized(false){};

        //! Called from the Julia side to create a stand-alone store.
        MHO_JlScanStoreInterface()
            : fScanStore(new MHO_ScanDataStore()), fIsOwned(true), fInitialized(false){};

        virtual ~MHO_JlScanStoreInterface()
        {
            if(fIsOwned)
            {
                Clear();
                delete fScanStore;
            }
        };

        void SetDirectory(std::string dir)
        {
            fScanStore->SetDirectory(dir);
            fInitialized = false;
        }

        bool Initialize()
        {
            fInitialized = fScanStore->Initialize();
            return fInitialized;
        }

        bool IsValid() const { return fScanStore->IsValid(); }

        bool IsBaselinePresent(std::string bl) const { return fScanStore->IsBaselinePresent(bl); }
        bool IsStationPresent(std::string st)  const { return fScanStore->IsStationPresent(st); }
        bool IsFringePresent(std::string fr)   const { return fScanStore->IsFringePresent(fr); }

        std::size_t GetNBaselines() const { return fScanStore->GetNBaselines(); }
        std::size_t GetNStations()  const { return fScanStore->GetNStations(); }
        std::size_t GetNFringes()   const { return fScanStore->GetNFringes(); }

        std::vector< std::string > GetBaselinesPresent() const { return fScanStore->GetBaselinesPresent(); }
        std::vector< std::string > GetStationsPresent()  const { return fScanStore->GetStationsPresent(); }
        std::vector< std::string > GetFringesPresent()   const { return fScanStore->GetFringesPresent(); }

        //! Return the root/ovex file data as a JSON string.
        std::string GetRootFileDataJSON() const
        {
            mho_json data = fScanStore->GetRootFileData();
            return data.dump();
        }

        std::string GetRootFileBasename()                   const { return fScanStore->GetRootFileBasename(); }
        std::string GetBaselineFilename(std::string bl)     const { return fScanStore->GetBaselineFilename(bl); }
        std::string GetStationFilename(std::string st)      const { return fScanStore->GetStationFilename(st); }
        std::string GetFringeFilename(std::string fr)       const { return fScanStore->GetFringeFilename(fr); }

        //! Load a baseline into memory.
        //! Returns: 0=ok, 1=already loaded, -1=not initialized, -2=not present.
        int LoadBaseline(std::string baseline)
        {
            if(!IsValid()) { return -1; }
            if(!IsBaselinePresent(baseline)) { return -2; }
            if(fBaselineContainers.count(baseline)) { return 1; }
            MHO_ContainerStore* con = new MHO_ContainerStore();
            fScanStore->LoadBaseline(baseline, con);
            fBaselineContainers[baseline] = std::make_pair(con, new MHO_JlContainerStoreInterface(con));
            return 0;
        }

        int LoadStation(std::string station)
        {
            if(!IsValid()) { return -1; }
            if(!IsStationPresent(station)) { return -2; }
            if(fStationContainers.count(station)) { return 1; }
            MHO_ContainerStore* con = new MHO_ContainerStore();
            fScanStore->LoadStation(station, con);
            fStationContainers[station] = std::make_pair(con, new MHO_JlContainerStoreInterface(con));
            return 0;
        }

        int LoadFringe(std::string fringe)
        {
            if(!IsValid()) { return -1; }
            if(!IsFringePresent(fringe)) { return -2; }
            if(fFringeContainers.count(fringe)) { return 1; }
            MHO_ContainerStore* con = new MHO_ContainerStore();
            fScanStore->LoadFringe(fringe, con);
            fFringeContainers[fringe] = std::make_pair(con, new MHO_JlContainerStoreInterface(con));
            return 0;
        }

        bool IsBaselineLoaded(std::string bl) const { return fInitialized && fBaselineContainers.count(bl) > 0; }
        bool IsStationLoaded(std::string st)  const { return fInitialized && fStationContainers.count(st) > 0; }
        bool IsFringeLoaded(std::string fr)   const { return fInitialized && fFringeContainers.count(fr) > 0; }

        std::vector< std::string > GetBaselinesLoaded() const { return Keys(fBaselineContainers); }
        std::vector< std::string > GetStationsLoaded()  const { return Keys(fStationContainers); }
        std::vector< std::string > GetFringesLoaded()   const { return Keys(fFringeContainers); }

        MHO_JlContainerStoreInterface& GetBaselineData(std::string baseline)
        {
            auto it = fBaselineContainers.find(baseline);
            if(it != fBaselineContainers.end()) { return *(it->second.second); }
            msg_error("julia_bindings", "baseline " << baseline << " not loaded" << eom);
            return fEmptyContainer;
        }

        MHO_JlContainerStoreInterface& GetStationData(std::string station)
        {
            auto it = fStationContainers.find(station);
            if(it != fStationContainers.end()) { return *(it->second.second); }
            msg_error("julia_bindings", "station " << station << " not loaded" << eom);
            return fEmptyContainer;
        }

        MHO_JlContainerStoreInterface& GetFringeData(std::string fringe)
        {
            auto it = fFringeContainers.find(fringe);
            if(it != fFringeContainers.end()) { return *(it->second.second); }
            msg_error("julia_bindings", "fringe " << fringe << " not loaded" << eom);
            return fEmptyContainer;
        }

        void Clear()
        {
            for(auto& kv : fBaselineContainers) { delete kv.second.second; delete kv.second.first; }
            for(auto& kv : fStationContainers)  { delete kv.second.second; delete kv.second.first; }
            for(auto& kv : fFringeContainers)   { delete kv.second.second; delete kv.second.first; }
            fBaselineContainers.clear();
            fStationContainers.clear();
            fFringeContainers.clear();
            fInitialized = false;
        }

    private:
        template< typename MapT >
        static std::vector< std::string > Keys(const MapT& m)
        {
            std::vector< std::string > keys;
            for(auto& kv : m) { keys.push_back(kv.first); }
            return keys;
        }

        MHO_ScanDataStore* fScanStore;
        bool fIsOwned;
        bool fInitialized;
        MHO_JlContainerStoreInterface fEmptyContainer;
        std::map< std::string, std::pair< MHO_ContainerStore*, MHO_JlContainerStoreInterface* > > fBaselineContainers;
        std::map< std::string, std::pair< MHO_ContainerStore*, MHO_JlContainerStoreInterface* > > fStationContainers;
        std::map< std::string, std::pair< MHO_ContainerStore*, MHO_JlContainerStoreInterface* > > fFringeContainers;
};


inline void DeclareJlScanStoreInterface(jlcxx::Module& mod, const std::string& jl_type_name)
{
    mod.add_type< MHO_JlScanStoreInterface >(jl_type_name)
        .constructor<>()  // constructable from Julia for stand-alone use
        .method("set_directory",        &hops::MHO_JlScanStoreInterface::SetDirectory)
        .method("initialize",           &hops::MHO_JlScanStoreInterface::Initialize)
        .method("is_valid",             &hops::MHO_JlScanStoreInterface::IsValid)
        .method("has_baseline",         &hops::MHO_JlScanStoreInterface::IsBaselinePresent)
        .method("has_station",          &hops::MHO_JlScanStoreInterface::IsStationPresent)
        .method("has_fringe",           &hops::MHO_JlScanStoreInterface::IsFringePresent)
        .method("get_nbaselines",       &hops::MHO_JlScanStoreInterface::GetNBaselines)
        .method("get_nstations",        &hops::MHO_JlScanStoreInterface::GetNStations)
        .method("get_nfringes",         &hops::MHO_JlScanStoreInterface::GetNFringes)
        .method("get_baseline_list",    &hops::MHO_JlScanStoreInterface::GetBaselinesPresent)
        .method("get_station_list",     &hops::MHO_JlScanStoreInterface::GetStationsPresent)
        .method("get_fringe_list",      &hops::MHO_JlScanStoreInterface::GetFringesPresent)
        .method("get_rootfile_data",    &hops::MHO_JlScanStoreInterface::GetRootFileDataJSON)
        .method("get_rootfile_basename",&hops::MHO_JlScanStoreInterface::GetRootFileBasename)
        .method("get_baseline_filename",&hops::MHO_JlScanStoreInterface::GetBaselineFilename)
        .method("get_station_filename", &hops::MHO_JlScanStoreInterface::GetStationFilename)
        .method("get_fringe_filename",  &hops::MHO_JlScanStoreInterface::GetFringeFilename)
        .method("load_baseline",        &hops::MHO_JlScanStoreInterface::LoadBaseline)
        .method("load_station",         &hops::MHO_JlScanStoreInterface::LoadStation)
        .method("load_fringe",          &hops::MHO_JlScanStoreInterface::LoadFringe)
        .method("is_baseline_loaded",   &hops::MHO_JlScanStoreInterface::IsBaselineLoaded)
        .method("is_station_loaded",    &hops::MHO_JlScanStoreInterface::IsStationLoaded)
        .method("is_fringe_loaded",     &hops::MHO_JlScanStoreInterface::IsFringeLoaded)
        .method("get_baselines_loaded", &hops::MHO_JlScanStoreInterface::GetBaselinesLoaded)
        .method("get_stations_loaded",  &hops::MHO_JlScanStoreInterface::GetStationsLoaded)
        .method("get_fringes_loaded",   &hops::MHO_JlScanStoreInterface::GetFringesLoaded)
        .method("get_baseline_data",    &hops::MHO_JlScanStoreInterface::GetBaselineData)
        .method("get_station_data",     &hops::MHO_JlScanStoreInterface::GetStationData)
        .method("get_fringe_data",      &hops::MHO_JlScanStoreInterface::GetFringeData)
        .method("clear",                &hops::MHO_JlScanStoreInterface::Clear);
}

} // namespace hops

#endif /*! end of include guard: MHO_JlScanStoreInterface */
