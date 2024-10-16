#ifndef MHO_MultitonePhaseCorrectionBuilderBuilder_HH__
#define MHO_MultitonePhaseCorrectionBuilderBuilder_HH__

#include "MHO_ContainerDefinitions.hh"
#include "MHO_OperatorBuilder.hh"

namespace hops
{

/*!
 *@file MHO_MultitonePhaseCorrectionBuilder.hh
 *@class MHO_MultitonePhaseCorrectionBuilder
 *@author J. Barrett - barrettj@mit.edu
 *@date Thu Dec 7 13:29:58 2023 -0500
 *@brief
 */

class MHO_MultitonePhaseCorrectionBuilder: public MHO_OperatorBuilder
{
    public:

        MHO_MultitonePhaseCorrectionBuilder(MHO_OperatorToolbox* toolbox, MHO_FringeData* fdata)
            : MHO_OperatorBuilder(toolbox, fdata)
        {
            fRefOpName = "ref_multitone_pcal";
            fRemOpName = "rem_multitone_pcal";
        };

        MHO_MultitonePhaseCorrectionBuilder(MHO_OperatorToolbox* toolbox, MHO_ContainerStore* cstore = nullptr,
                                            MHO_ParameterStore* pstore = nullptr)
            : MHO_OperatorBuilder(toolbox, cstore, pstore)
        {
            fRefOpName = "ref_multitone_pcal";
            fRemOpName = "rem_multitone_pcal";
        };

        virtual ~MHO_MultitonePhaseCorrectionBuilder(){};

        virtual bool Build() override;

    private:
        std::string ExtractStationMk4ID(std::string op_name); //op_name indicates reference or remote station
        int ExtractPCPeriod(std::string mk4id);               //pulls the appropriate pc_period out of parameter store
        void AttachSamplerDelays(multitone_pcal_type* pcal_data, std::string mk4id); //attaches sampler delays to pcal data
        void AttachPCToneMask(multitone_pcal_type* pcal_data,
                              std::string mk4id); //attaches pc_tonemask infor to pcal data (if present)
        std::string GetSamplerDelayKey(std::string pol);

        std::string fRefOpName;
        std::string fRemOpName;
};

} // namespace hops

#endif /*! end of include guard: MHO_MultitonePhaseCorrectionBuilderBuilder_HH__ */
