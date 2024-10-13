#ifndef MHO_LSBOffsetBuilderBuilder_HH__
#define MHO_LSBOffsetBuilderBuilder_HH__

#include "MHO_OperatorBuilder.hh"

namespace hops
{

/*!
 *@file MHO_LSBOffsetBuilder.hh
 *@class MHO_LSBOffsetBuilder
 *@author J. Barrett - barrettj@mit.edu
 *@date Tue Jun 20 12:35:56 2023 -0400
 *@brief
 */

class MHO_LSBOffsetBuilder: public MHO_OperatorBuilder
{
    public:
        MHO_LSBOffsetBuilder(MHO_OperatorToolbox* toolbox, MHO_ContainerStore* cstore = nullptr,
                             MHO_ParameterStore* pstore = nullptr)
            : MHO_OperatorBuilder(toolbox, cstore, pstore){};

        virtual ~MHO_LSBOffsetBuilder(){};

        virtual bool Build() override;

    private:
        std::string ExtractStationIdentifier();
};

} // namespace hops

#endif /*! end of include guard: MHO_LSBOffsetBuilderBuilder_HH__ */
