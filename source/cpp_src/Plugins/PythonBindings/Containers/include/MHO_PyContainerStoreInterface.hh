#ifndef MHO_PyContainerStoreInterface_HH__
#define MHO_PyContainerStoreInterface_HH__


#include "MHO_ContainerDefinitions.hh"
#include "MHO_ContainerStore.hh"
#include "MHO_PyTableContainer.hh"

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h> //this is important to have for std::complex<T> support!

namespace py = pybind11;

/*
*@file: MHO_PyContainerStoreInterface.hh
*@class: MHO_PyContainerStoreInterface
*@author: J. Barrett
*@email: barrettj@mit.edu
*@date:
*@brief:
*/

namespace hops
{


class MHO_PyContainerStoreInterface
{
    public:

        MHO_PyContainerStoreInterface():fContainerStore(nullptr){};
        virtual ~MHO_PyContainerStoreInterface(){};

        //single access point to visiblity object
        void SetVisibilities(visibility_type* vis){fVisibilities = vis;};
        visibility_type& GetVisibilities(){return *fVisibilities;}

        MHO_PyTableContainer< visibility_type >& GetVisibilityTable()
        {
            if( fVisibilities->HasExtension< MHO_PyTableContainer< visibility_type > >() )
            {
                return *( fVisibilities->AsExtension< MHO_PyTableContainer< visibility_type > >() );
            }
            else
            {
                return *(fVisibilities->MakeExtension< MHO_PyTableContainer< visibility_type > >() );
            }
        }


    private:

        //pointers to objects which have been registered
        visibility_type* fVisibilities;

};

}//end of namespace

#endif /* end of include guard: MHO_PyContainerStoreInterface */
