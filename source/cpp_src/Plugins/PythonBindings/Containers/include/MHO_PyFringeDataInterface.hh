#ifndef MHO_PyFringeDataInterface_HH__
#define MHO_PyFringeDataInterface_HH__

#include "MHO_ContainerDictionary.hh"
#include "MHO_ContainerDefinitions.hh"
#include "MHO_ContainerStore.hh"
#include "MHO_FringeData.hh"

#include "MHO_PyTableContainer.hh"
#include "MHO_PyScanStoreInterface.hh"
#include "MHO_PyContainerStoreInterface.hh"
#include "MHO_PyParameterStoreInterface.hh"

//need these extras to be able to translate between nl:json and py:dict or py::object
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include "pybind11_json/pybind11_json.hpp"
namespace py = pybind11;
namespace nl = nlohmann;
using namespace pybind11::literals;


/*
*@file: MHO_PyFringeDataInterface.hh
*@class: MHO_PyFringeDataInterface
*@author: J. Barrett
*@email: barrettj@mit.edu
*@date: Fri Sep 15 10:03:38 PM EDT 2023
*@brief:
*/

namespace hops
{


class MHO_PyFringeDataInterface
{
    public:

        MHO_PyFringeDataInterface(MHO_FringeData* fdata):
            fFringeData(fdata),
            fScanInterface(fdata->GetScanDataStore()),
            fContainerInterface(fdata->GetContainerStore()),
            fParameterInterface(fdata->GetParameterStore())
        {

        };

        virtual ~MHO_PyFringeDataInterface(){};

        MHO_PyParameterStoreInterface& GetParameterStore(){return fParameterInterface;}
        MHO_PyContainerStoreInterface& GetContainerStore(){return fContainerInterface;}
        MHO_PyScanStoreInterface& GetScanStore(){return fScanInterface;}

        //return copy of vex data converted to py::dict
        py::dict GetVex() const {return fFringeData->GetVex();}
        py::dict GetPlotData() const {return fFringeData->GetPlotData();}

        //for now we do not expose these to python
        //access to the control format and parsed control statements as py::dict
        // py::dict GetControlFormat() const {return fFringeData->GetControlFormat();}
        // py::dict GetControlStatements() const {return fFringeData->GetControlStatements();}

    private:

        MHO_FringeData* fFringeData;

        //pointer to the parameter store
        MHO_PyScanStoreInterface fScanInterface;
        MHO_PyContainerStoreInterface fContainerInterface;
        MHO_PyParameterStoreInterface fParameterInterface;
};

void
DeclarePyFringeDataInterface(py::module &m, std::string pyclass_name)
{
    py::class_< MHO_PyFringeDataInterface >(m, pyclass_name.c_str() )
        //no __init__ def here, as this class is not meant to be constructable on the python side
        .def("get_parameter_store", &hops::MHO_PyFringeDataInterface::GetParameterStore,
            py::return_value_policy::reference,
            "get the current parameter set object"
        )
        .def("get_container_store", &hops::MHO_PyFringeDataInterface::GetContainerStore,
            py::return_value_policy::reference,
            "get the current container store object"
        )
        .def("get_scan_store", &hops::MHO_PyFringeDataInterface::GetScanStore,
            py::return_value_policy::reference,
            "get the current scan data store object"
        )
        .def("get_vex", &hops::MHO_PyFringeDataInterface::GetVex,
            py::return_value_policy::copy,
            "get the current scan ovex/root data as a python dictionary"
        )
        .def("get_plot_data", &hops::MHO_PyFringeDataInterface::GetPlotData,
            py::return_value_policy::copy,
            "get the current plot data as a python dictionary"
        );
}





}//end of namespace

#endif /* end of include guard: MHO_PyFringeDataInterface */
