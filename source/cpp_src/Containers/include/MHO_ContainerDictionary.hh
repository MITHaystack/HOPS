#ifndef MHO_ContainerDictionary_HH__
#define MHO_ContainerDictionary_HH__


#include "MHO_Axis.hh"
#include "MHO_AxisPack.hh"
#include "MHO_ScalarContainer.hh"
#include "MHO_VectorContainer.hh"
#include "MHO_TableContainer.hh"
#include "MHO_ObjectTags.hh"

#include "MHO_ClassIdentityMap.hh"

namespace hops
{

/*!
*@file MHO_ContainerDictionary.hh
*@class MHO_ContainerDictionary
*@author J. Barrett - barrettj@mit.edu
*@date Thu Apr 29 12:30:28 2021 -0400
*@brief
*/

class MHO_ContainerDictionary: public MHO_ClassIdentityMap
{
    public:

        MHO_ContainerDictionary();
        virtual ~MHO_ContainerDictionary(){};

    private:
};

}//end of namespace

#endif /*! end of include guard: MHO_ContainerDictionary */
