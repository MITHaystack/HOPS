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
#include "MHO_ChannelQuantity.hh"

namespace hops
{

class MHO_MultitonePhaseCorrectionBuilder: 
    public MHO_OperatorBuilder,
    public MHO_ChannelQuantity
{
    public:

        MHO_MultitonePhaseCorrectionBuilder(MHO_OperatorToolbox* toolbox, 
                                            MHO_ContainerStore* cstore = nullptr,
                                            MHO_ParameterStore* pstore = nullptr):
            MHO_OperatorBuilder(toolbox, cstore, pstore)
        {};

        virtual ~MHO_MultitonePhaseCorrectionBuilder(){};

        virtual bool Build() override;

    private:

        std::string ExtractStationMk4ID();
};

}//end namespace


#endif /* end of include guard: MHO_MultitonePhaseCorrectionBuilderBuilder_HH__ */
