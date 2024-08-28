#ifndef MHO_MPIInterfaceWrapper_HH__
#define MHO_MPIInterfaceWrapper_HH__

//hides the usage of MPI when it was not compiled-in

#ifdef HOPS_USE_MPI
#include "MHO_MPIInterface.hh"
#endif

#ifdef HOPS_USE_MPI
    #define MPI_SINGLE_PROCESS if (hops::MHO_MPIInterface::GetInstance()->GetGlobalProcessID() == 0)
#else
    #define MPI_SINGLE_PROCESS
#endif

#endif /* end of include guard: MHO_MPIInterfaceWrapper_HH__ */
