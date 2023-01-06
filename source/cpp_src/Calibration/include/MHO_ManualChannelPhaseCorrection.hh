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
#include "MHO_ContainerDefinitions.hh"
#include "MHO_BinaryOperator.hh"


namespace hops
{


class MHO_ManualChannelPhaseCorrection: public MHO_BinaryOperator<
    ch_visibility_type,
    ch_pcal_phase_type,
    ch_visibility_type >
{
    public:

        MHO_ManualChannelPhaseCorrection();
        virtual ~MHO_ManualChannelPhaseCorrection();

        using XArgType1 = ch_visibility_type;
        using XArgType2 = ch_pcal_phase_type;
        using XArgType3 = ch_visibility_type;

        virtual bool InitializeImpl(const XArgType1* in1, const XArgType2* in2, XArgType3* out) override;
        virtual bool ExecuteImpl(const XArgType1* in1, const XArgType2* in2, XArgType3* out) override;

    private:

        bool fInitialized;

        //may want to cache the dimensions of the input arrays and initialization state 




};


}


#endif /* end of include guard: MHO_ManualChannelPhaseCorrection */
