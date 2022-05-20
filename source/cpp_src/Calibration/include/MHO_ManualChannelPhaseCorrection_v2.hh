#ifndef MHO_ManualChannelPhaseCorrection_v2_HH__
#define MHO_ManualChannelPhaseCorrection_v2_HH__

/*
*File: MHO_ManualChannelPhaseCorrection_v2.hh
*Class: MHO_ManualChannelPhaseCorrection_v2
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


class MHO_ManualChannelPhaseCorrection_v2: public MHO_BinaryOperator<
    ch_visibility_type,
    ch_pcal_phase_type,
    ch_visibility_type >
{
    public:

        MHO_ManualChannelPhaseCorrection_v2();
        virtual ~MHO_ManualChannelPhaseCorrection_v2();

        using XArgType1 = ch_visibility_type;
        using XArgType2 = ch_pcal_phase_type;
        using XArgType3 = ch_visibility_type;

        virtual bool InitializeImpl(const XArgType1* in1, const XArgType2* in2, XArgType3* out) override;
        virtual bool ExecuteImpl(const XArgType1* in1, const XArgType2* in2, XArgType3* out) override;

    private:

        //may want to cache the dimensions of the input arrays and initialization state 


};


}


#endif /* end of include guard: MHO_ManualChannelPhaseCorrection_v2 */
