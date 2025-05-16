#ifndef MHO_HDF5ContainerFileInterface_HH__
#define MHO_HDF5ContainerFileInterface_HH__

#include "MHO_BinaryFileInterface.hh"
#include "MHO_ContainerFileInterface.hh"
#include "MHO_ContainerDictionary.hh"
#include "MHO_Message.hh"

#include "MHO_ContainerDefinitions.hh"
#include "MHO_ContainerStore.hh"

#include "MHO_HDF5ConverterDictionary.hh"

namespace hops
{

/*!
 *@file  MHO_HDF5ContainerFileInterface.hh
 *@class  MHO_HDF5ContainerFileInterface
 *@author  J. Barrett - barrettj@mit.edu
 *@date Fri May 16 10:42:52 AM EDT 2025
 *@brief
 */

class MHO_HDF5ContainerFileInterface: 
    public MHO_ContainerFileInterface, 
    public MHO_HDF5ConverterDictionary
{
    public:
        MHO_HDF5ContainerFileInterface();
        virtual ~MHO_HDF5ContainerFileInterface();

        //HDF5 file conversion interface
        int ConvertStoreToHDF5(MHO_ContainerStore& store, std::string hdf5_filename);
};

} // namespace hops

#endif /*! end of include guard: MHO_HDF5ContainerFileInterface */
