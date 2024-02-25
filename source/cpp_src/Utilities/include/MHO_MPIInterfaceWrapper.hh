#ifndef MHO_MPIInterfaceWrapper_HH__
#define MHO_MPIInterfaceWrapper_HH__

/*!
*@file MHO_MPIInterfaceWrapper.hh
*@class
*@date Sat Mar 11 18:35:02 2023 -0500
*@brief hides the usage of MPI when it was not compiled-in
*@author J. Barrett - barrettj@mit.edu
*/


#ifdef HOPS_USE_MPI
#include "MHO_MPIInterface.hh"
#endif

#ifdef HOPS_USE_MPI
    #define MPI_SINGLE_PROCESS if (hops::MHO_MPIInterface::GetInstance()->GetGlobalProcessID() == 0)
#else
    #define MPI_SINGLE_PROCESS
#endif

#endif /*! end of include guard: MHO_MPIInterfaceWrapper_HH__ */
