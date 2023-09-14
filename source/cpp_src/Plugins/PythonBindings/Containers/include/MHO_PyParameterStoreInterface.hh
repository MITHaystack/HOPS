#ifndef MHO_PyParameterStoreInterface_HH__
#define MHO_PyParameterStoreInterface_HH__


// #include "MHO_ContainerDefinitions.hh"
// #include "MHO_ContainerStore.hh"
// #include "MHO_PyTableContainer.hh"

#include "MHO_ParameterStore.hh"

#include <pybind11/pybind11.h>

namespace py = pybind11;

/*
*@file: MHO_PyParameterStoreInterface.hh
*@class: MHO_PyParameterStoreInterface
*@author: J. Barrett
*@email: barrettj@mit.edu
*@date:
*@brief:
*/

namespace hops
{


class MHO_PyParameterStoreInterface
{
    public:

        MHO_PyParameterStoreInterface(MHO_ParameterStore* paramStore):fParameterStore(paramStore){};
        virtual ~MHO_PyParameterStoreInterface(){};

        // //single access point to visiblity object
        // void SetVisibilities(visibility_type* vis){fVisibilities = vis;};
        // visibility_type& GetVisibilities(){return *fVisibilities;}
        //
        // MHO_PyTableContainer< visibility_type >& GetVisibilityTable()
        // {
        //     if( fVisibilities->HasExtension< MHO_PyTableContainer< visibility_type > >() )
        //     {
        //         return *( fVisibilities->AsExtension< MHO_PyTableContainer< visibility_type > >() );
        //     }
        //     else
        //     {
        //         return *(fVisibilities->MakeExtension< MHO_PyTableContainer< visibility_type > >() );
        //     }
        // }

        py::bool_ IsPresent(const std::string& value_path) const
        {
            return fParameterStore->IsPresent(value_path);
        }


    private:

        //pointer to the parameter store
        MHO_ParameterStore* fParameterStore;

};



//XTableType must inherit from MHO_NDArrayWrapper<XValueType, RANK>
void
DeclarePyParameterStoreInterface(py::module &m, std::string pyclass_name)
{
    py::class_< MHO_PyParameterStoreInterface >(m, pyclass_name.c_str() )
        //no __init__ def here, as this class is not meant to be constructable on the python side
        .def("IsPresent", &hops::MHO_PyParameterStoreInterface::IsPresent);
}


}//end of namespace

#endif /* end of include guard: MHO_PyParameterStoreInterface */
