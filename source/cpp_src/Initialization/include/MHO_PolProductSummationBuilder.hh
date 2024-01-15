#ifndef MHO_PolProductSummationBuilderBuilder_HH__
#define MHO_PolProductSummationBuilderBuilder_HH__

/*
*File: MHO_PolProductSummationBuilder.hh
*Class: MHO_PolProductSummationBuilder
*Author:
*Email:
*Date:
*Description:
*/

#include "MHO_OperatorBuilder.hh"

namespace hops
{

class MHO_PolProductSummationBuilder: 
    public MHO_OperatorBuilder
{
    public:

        MHO_PolProductSummationBuilder(MHO_OperatorToolbox* toolbox, 
                                                MHO_ContainerStore* cstore = nullptr,
                                                MHO_ParameterStore* pstore = nullptr):
            MHO_OperatorBuilder(toolbox, cstore, pstore)
        {};

        virtual ~MHO_PolProductSummationBuilder(){};

        virtual bool Build() override;

    private:

        // std::string ParsePolFromName(const std::string& name);
        // std::string ExtractStationMk4ID();
};

}//end namespace


#endif /* end of include guard: MHO_PolProductSummationBuilderBuilder_HH__ */
