#ifndef MHO_SamplerLabelerBuilderBuilder_HH__
#define MHO_SamplerLabelerBuilderBuilder_HH__

#include "MHO_OperatorBuilder.hh"

namespace hops
{

/*!
 *@file MHO_SamplerLabelerBuilder.hh
 *@class MHO_SamplerLabelerBuilder
 *@author J. Barrett - barrettj@mit.edu
 *@date Thu Dec 14 11:49:02 2023 -0500
 *@brief
 */

class MHO_SamplerLabelerBuilder: public MHO_OperatorBuilder
{
    public:
        MHO_SamplerLabelerBuilder(MHO_OperatorToolbox* toolbox, MHO_FringeData* fdata)
            : MHO_OperatorBuilder(toolbox, fdata){};

        MHO_SamplerLabelerBuilder(MHO_OperatorToolbox* toolbox, MHO_ContainerStore* cstore = nullptr,
                                  MHO_ParameterStore* pstore = nullptr)
            : MHO_OperatorBuilder(toolbox, cstore, pstore){};

        virtual ~MHO_SamplerLabelerBuilder(){};

        virtual bool Build() override;

    private:
};

} // namespace hops

#endif /*! end of include guard: MHO_SamplerLabelerBuilderBuilder_HH__ */
