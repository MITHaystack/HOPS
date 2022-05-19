#ifndef MHO_PyContainerInterface_HH__
#define MHO_PyContainerInterface_HH__


#include "MHO_ContainerDefinitions.hh"
#include "MHO_PyTableContainer.hh"

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h> //this is important to have for std::complex<T> support!
namespace py = pybind11;

/*
*@file: MHO_PyContainerInterface.hh
*@class: MHO_PyContainerInterface
*@author: J. Barrett
*@email: barrettj@mit.edu
*@date:
*@brief:
*/

namespace hops
{


class MHO_PyContainerInterface
{
    public:

        MHO_PyContainerInterface():fVisibilities(nullptr){};
        virtual ~MHO_PyContainerInterface(){};

        //single access point to visiblity object
        void SetVisibilities(ch_visibility_type* vis){fVisibilities = vis;};
        ch_visibility_type& GetVisibilities(){return *fVisibilities;}

        MHO_PyTableContainer< ch_visibility_type >& GetVisibilityTable()
        {
            if( fVisibilities->HasExtension< MHO_PyTableContainer< ch_visibility_type > >() )
            {
                return *( fVisibilities->AsExtension< MHO_PyTableContainer< ch_visibility_type > >() );
            }
            else
            {
                return *(fVisibilities->MakeExtension< MHO_PyTableContainer< ch_visibility_type > >() );
            }
        }


    private:

        //pointers to objects which have been registered
        ch_visibility_type* fVisibilities;

};

}//end of namespace

#endif /* end of include guard: MHO_PyContainerInterface */
