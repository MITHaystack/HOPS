#ifndef MHO_MBDelaySearchCUDA_HH__
#define MHO_MBDelaySearchCUDA_HH__

#include <cmath>
#include <complex>

#include "MHO_MBDelaySearch.hh"

// CUDA includes
#include <cuComplex.h>
#include <cuda.h>
#include <cuda_runtime_api.h>
#include <cufft.h>
#include <stdint.h>

namespace hops
{

/*!
 *@file MHO_MBDelaySearchCUDA.hh
 *@class MHO_MBDelaySearchCUDA
 *@author J. Barrett - barrettj@mit.edu
 *@dateTue Jul 16 10:40:47 PM EDT 2024
 *@brief This is an ultra basic CUDA implementation of the
 * the coarse MBD/SBD/DR search, its quite primitive and only calls the CUFFT library
 * to speed up the inner-most loop over (DR,MBD) space. It is not optimized and
 * has far too much movement of data between host <-> device, additional work is need
 * to optimize this routine.
 */

using mbd_axis_pack = MHO_AxisPack< time_axis_type >;
using mbd_dr_axis_pack = MHO_AxisPack< delay_rate_axis_type, time_axis_type >;
using mbd_type = MHO_TableContainer< visibility_element_type, mbd_axis_pack >;
using mbd_amp_type = MHO_TableContainer< double, mbd_axis_pack >;
using mbd_dr_type = MHO_TableContainer< visibility_element_type, mbd_dr_axis_pack >;

class MHO_MBDelaySearchCUDA: public MHO_MBDelaySearch //public MHO_InspectingOperator< visibility_type >
{
    public:
        MHO_MBDelaySearchCUDA();
        virtual ~MHO_MBDelaySearchCUDA();

    protected:
        using XArgType = visibility_type;

        virtual bool InitializeImpl(const XArgType* in) override;
        virtual bool ExecuteImpl(const XArgType* in) override;

    private:
        //Host data buffer
        mbd_dr_type fHostBuffer;
        //Device memory buffer
        cufftDoubleComplex* fDeviceBuffer;
        //the cuFFT plan
        cufftHandle fCUFFTPlan;
};

} // namespace hops

#endif /*! end of include guard: MHO_MBDelaySearchCUDA_HH__ */
