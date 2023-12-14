#ifndef MHO_MultitonePhaseCorrectionBuilderBuilder_HH__
#define MHO_MultitonePhaseCorrectionBuilderBuilder_HH__

/*
*File: MHO_MultitonePhaseCorrectionBuilder.hh
*Class: MHO_MultitonePhaseCorrectionBuilder
*Author:
*Email:
*Date:
*Description:
*/

#include "MHO_OperatorBuilder.hh"
#include "MHO_ContainerDefinitions.hh"


namespace hops
{

class MHO_MultitonePhaseCorrectionBuilder:
    public MHO_OperatorBuilder
{
    public:

        MHO_MultitonePhaseCorrectionBuilder(MHO_OperatorToolbox* toolbox,
                                            MHO_ContainerStore* cstore = nullptr,
                                            MHO_ParameterStore* pstore = nullptr):
            MHO_OperatorBuilder(toolbox, cstore, pstore)
        {
            fRefOpName = "ref_multitone_pcal";
            fRemOpName = "rem_multitone_pcal";
        };

        virtual ~MHO_MultitonePhaseCorrectionBuilder(){};

        virtual bool Build() override;

    private:

        std::string ExtractStationMk4ID(std::string op_name); //op_name indicates reference or remote station
        int ExtractPCPeriod(std::string mk4id); //pulls the appropriate pc_period out of parameter store
        void ExtractSamplerDelays(multitone_pcal_type* pcal_data, std::string mk4id); //attaches sampler delays to pcal data
        std::string GetSamplerDelayKey(std::string pol);
        
        std::string fRefOpName;
        std::string fRemOpName;
};

}//end namespace


#endif /* end of include guard: MHO_MultitonePhaseCorrectionBuilderBuilder_HH__ */
