#ifndef MHO_MixedPolYShiftBuilderBuilder_HH__
#define MHO_MixedPolYShiftBuilderBuilder_HH__

#include "MHO_OperatorBuilder.hh"

namespace hops
{

/*!
 *@file MHO_MixedPolYShiftBuilder.hh
 *@class MHO_MixedPolYShiftBuilder
 *@author J. Barrett - barrettj@mit.edu
 *@date Tue Jun 20 12:35:56 2023 -0400
 *@brief
 */

class MHO_MixedPolYShiftBuilder: public MHO_OperatorBuilder
{
    public:
        MHO_MixedPolYShiftBuilder(MHO_OperatorToolbox* toolbox,
                                  MHO_ContainerStore* cstore = nullptr,
                                  MHO_ParameterStore* pstore = nullptr)
            : MHO_OperatorBuilder(toolbox, cstore, pstore){};

        virtual ~MHO_MixedPolYShiftBuilder(){};

        virtual bool Build() override;

    private:

};

} // namespace hops

#endif /*! end of include guard: MHO_MixedPolYShiftBuilderBuilder_HH__ */
