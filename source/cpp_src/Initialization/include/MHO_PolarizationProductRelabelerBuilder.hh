#ifndef MHO_PolarizationProductRelabelerBuilder_HH
#define MHO_PolarizationProductRelabelerBuilder_HH

#include "MHO_OperatorBuilder.hh"

namespace hops
{

class MHO_PolarizationProductRelabelerBuilder : public MHO_OperatorBuilder
{
public:
    MHO_PolarizationProductRelabelerBuilder(MHO_OperatorToolbox* toolbox, MHO_FringeData* fdata): MHO_OperatorBuilder(toolbox, fdata){};

    MHO_PolarizationProductRelabelerBuilder(MHO_OperatorToolbox* toolbox, MHO_ContainerStore* cstore = nullptr,
                       MHO_ParameterStore* pstore = nullptr)
        : MHO_OperatorBuilder(toolbox, cstore, pstore){};

    virtual ~MHO_PolarizationProductRelabelerBuilder(){};

    virtual bool Build() override;
};

}

#endif