#ifndef MHO_SamplerLabelerBuilderBuilder_HH__
#define MHO_SamplerLabelerBuilderBuilder_HH__

/*!
*@file MHO_SamplerLabelerBuilder.hh
*@class MHO_SamplerLabelerBuilder
*@author
*Email:
*@date
*@brief
*/

#include "MHO_OperatorBuilder.hh"

namespace hops
{

class MHO_SamplerLabelerBuilder: public MHO_OperatorBuilder
{
    public:

        MHO_SamplerLabelerBuilder(MHO_OperatorToolbox* toolbox,
                                   MHO_ContainerStore* cstore = nullptr,
                                   MHO_ParameterStore* pstore = nullptr):
            MHO_OperatorBuilder(toolbox, cstore, pstore)
        {};

        virtual ~MHO_SamplerLabelerBuilder(){};

        virtual bool Build() override;

    private:

};

}//end namespace


#endif /*! end of include guard: MHO_SamplerLabelerBuilderBuilder_HH__ */
