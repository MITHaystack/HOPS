#ifndef MHO_MBDelaySearchOpenMP_HH__
#define MHO_MBDelaySearchOpenMP_HH__

#include <vector>

#ifdef _OPENMP
    #include <omp.h>
#else
    //stubs so code compiles without OpenMP; pragmas are silently ignored
    inline int omp_get_max_threads() { return 1; }
    inline int omp_get_thread_num()  { return 0; }
#endif

#include "MHO_MBDelaySearch.hh"

namespace hops
{

/*!
 *@file MHO_MBDelaySearchOpenMP.hh
 *@class MHO_MBDelaySearchOpenMP
 *@author J. Barrett - barrettj@mit.edu
 *@date Tue Apr 11 16:50:11 2023 -0400
 *@brief OpenMP-parallel override of MHO_MBDelaySearch; parallelizes the outer SBD loop
 */

/**
 * @brief Class MHO_MBDelaySearchOpenMP
 */
class MHO_MBDelaySearchOpenMP: public MHO_MBDelaySearch
{
    public:
        MHO_MBDelaySearchOpenMP();
        virtual ~MHO_MBDelaySearchOpenMP();

    protected:
        using XArgType = visibility_type;

        virtual bool InitializeImpl(const XArgType* in) override;
        virtual bool ExecuteImpl(const XArgType* in) override;

        //per-thread argmax accumulator; alignas(64) + _pad prevents false sharing across cache lines
        struct alignas(64) LocalMax
        {
            double val      = -0.0;
            int    mbd_bin  = -1;
            int    sbd_bin  = -1;
            int    dr_bin   = -1;
            double n_points = 0.0;
            char   _pad[32]; //pad struct to one full 64-byte cache line
        };

        int fNThreads;
        std::vector< LocalMax >           fThreadMaxima;
        std::vector< visibility_type >    fPerThreadSBDWorkspace;
        std::vector< visibility_type >    fPerThreadSBDDrData;
        std::vector< MHO_DelayRate >      fPerThreadDelayRateCalc;
        std::vector< mbd_dr_type >        fPerThreadSearchBuffer;
        std::vector< FFT_2D_ENGINE_TYPE > fPerThreadBatchedFFTEngine;
};

} // namespace hops

#endif /*! end of include guard: MHO_MBDelaySearchOpenMP_HH__ */
