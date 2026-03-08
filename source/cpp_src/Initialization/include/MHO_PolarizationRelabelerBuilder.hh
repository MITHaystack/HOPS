#ifndef MHO_PolarizationRelabelerBuilder_HH
#define MHO_PolarizationRelabelerBuilder_HH

#include "MHO_OperatorBuilder.hh"

namespace hops
{

class MHO_PolarizationRelabelerBuilder : public MHO_OperatorBuilder
{
    public:
        MHO_PolarizationRelabelerBuilder(MHO_OperatorToolbox* toolbox, MHO_FringeData* fdata): MHO_OperatorBuilder(toolbox, fdata){};

        MHO_PolarizationRelabelerBuilder(MHO_OperatorToolbox* toolbox, MHO_ContainerStore* cstore = nullptr, MHO_ParameterStore* pstore = nullptr)
            : MHO_OperatorBuilder(toolbox, cstore, pstore){};

        virtual ~MHO_PolarizationRelabelerBuilder(){};

        //polarization relabeler builder takes care of visibility, weight and multitone pcal data 
        //(it builds multiple operator classes to do this)
        virtual bool Build() override;

    private:

        std::string ExtractStationIdentifier();
};

}

#endif