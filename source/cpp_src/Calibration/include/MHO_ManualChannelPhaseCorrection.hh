#ifndef MHO_ManualChannelPhaseCorrection_HH__
#define MHO_ManualChannelPhaseCorrection_HH__

/*
*File: MHO_ManualChannelPhaseCorrection.hh
*Class: MHO_ManualChannelPhaseCorrection
*Author:
*Email: 
*Date:
*Description:
*/

#include <cmath>
#include <complex>

#include "MHO_TableContainer.hh"
#include "MHO_ChannelizedVisibilities.hh"
#include "MHO_UnaryOperator.hh"


namespace hops
{


class MHO_ManualChannelPhaseCorrection: public MHO_UnaryOperator< ch_baseline_data_type >
{
    public:

        MHO_ManualChannelPhaseCorrection();
        virtual ~MHO_ManualChannelPhaseCorrection();

        //need function to pass in the per-channel phase corrections (e.g. '0': 90.0, '1': 45.0 ) etc.
        //how should we associate each channel phase to channel data (name/number/label, etc?)

    protected:

        using XArrayType = ch_baseline_data_type;

        virtual bool InitializeInPlace(XArrayType* in) override;
        virtual bool ExecuteInPlace(XArrayType* in) override;

        //for now these can remain un-implemented, we just call the in-place operations and then do a copy
        virtual bool InitializeOutOfPlace(const XArrayType* in, XArrayType* out) override;
        virtual bool ExecuteOutOfPlace(const XArrayType* in, XArrayType* out) override;

    private:

        //may want to cache the dimensions of the input array, and initialization state (have the phase corrections been set?)

        //need private space to store the phase corrections for each channel


};


}


#endif /* end of include guard: MHO_ManualChannelPhaseCorrection */
