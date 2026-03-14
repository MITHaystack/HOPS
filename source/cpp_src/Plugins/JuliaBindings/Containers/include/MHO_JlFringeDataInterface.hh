#ifndef MHO_JlFringeDataInterface_HH__
#define MHO_JlFringeDataInterface_HH__

#include <string>

#include "MHO_ContainerDefinitions.hh"
#include "MHO_ContainerDictionary.hh"
#include "MHO_ContainerStore.hh"
#include "MHO_FringeData.hh"

#include "MHO_JlContainerStoreInterface.hh"
#include "MHO_JlParameterStoreInterface.hh"
#include "MHO_JlScanStoreInterface.hh"
#include "MHO_JlTableContainer.hh"

#include "jlcxx/jlcxx.hpp"

namespace hops
{

/*!
 *@file  MHO_JlFringeDataInterface.hh
 *@class  MHO_JlFringeDataInterface
 *@author  J. Barrett - barrettj@mit.edu
 *@brief Julia (CxxWrap) bindings for MHO_FringeData.
 *
 * Aggregates the parameter, container, and scan store interfaces.
 * get_vex() and get_plot_data() return JSON strings.
 */

class MHO_JlFringeDataInterface
{
    public:
        MHO_JlFringeDataInterface(MHO_FringeData* fdata)
            : fFringeData(fdata),
              fScanInterface(fdata->GetScanDataStore()),
              fContainerInterface(fdata->GetContainerStore()),
              fParameterInterface(fdata->GetParameterStore()){};

        virtual ~MHO_JlFringeDataInterface(){};

        MHO_JlParameterStoreInterface& GetParameterStore() { return fParameterInterface; }
        MHO_JlContainerStoreInterface& GetContainerStore() { return fContainerInterface; }
        MHO_JlScanStoreInterface&      GetScanStore()      { return fScanInterface; }

        //! Return the vex/root file data as a JSON string.
        std::string GetVexJSON() const
        {
            mho_json data = fFringeData->GetVex();
            return data.dump();
        }

        //! Return the current fringe plot data as a JSON string.
        std::string GetPlotDataJSON() const
        {
            mho_json data = fFringeData->GetPlotData();
            return data.dump();
        }

    private:
        MHO_FringeData* fFringeData;
        MHO_JlScanStoreInterface      fScanInterface;
        MHO_JlContainerStoreInterface fContainerInterface;
        MHO_JlParameterStoreInterface fParameterInterface;
};


inline void DeclareJlFringeDataInterface(jlcxx::Module& mod, const std::string& jl_type_name)
{
    mod.add_type< MHO_JlFringeDataInterface >(jl_type_name)
        // Not constructable from Julia (wraps a C++-owned object).
        .method("get_parameter_store", &hops::MHO_JlFringeDataInterface::GetParameterStore)
        .method("get_container_store", &hops::MHO_JlFringeDataInterface::GetContainerStore)
        .method("get_scan_store",      &hops::MHO_JlFringeDataInterface::GetScanStore)
        .method("get_vex",             &hops::MHO_JlFringeDataInterface::GetVexJSON)
        .method("get_plot_data",       &hops::MHO_JlFringeDataInterface::GetPlotDataJSON);
}

} // namespace hops

#endif /*! end of include guard: MHO_JlFringeDataInterface */
