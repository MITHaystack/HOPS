#ifndef MHO_ChannelLabelerBuilderBuilder_HH__
#define MHO_ChannelLabelerBuilderBuilder_HH__

#include "MHO_ChannelQuantity.hh"
#include "MHO_OperatorBuilder.hh"

namespace hops
{

/*!
 *@file MHO_ChannelLabelerBuilder.hh
 *@class MHO_ChannelLabelerBuilder
 *@author
 *@date Fri Jun 2 16:09:43 2023 -0400
 *@brief builds a channel labeler operator 
 */

/**
 * @brief Class MHO_ChannelLabelerBuilder
 */
class MHO_ChannelLabelerBuilder: public MHO_OperatorBuilder, public MHO_ChannelQuantity
{
    public:

        MHO_ChannelLabelerBuilder(MHO_OperatorToolbox* toolbox, MHO_FringeData* fdata)
            : MHO_OperatorBuilder(toolbox, fdata), MHO_ChannelQuantity(){};

        MHO_ChannelLabelerBuilder(MHO_OperatorToolbox* toolbox, 
                                  MHO_ContainerStore* cstore = nullptr,
                                  MHO_ParameterStore* pstore = nullptr)
            : MHO_OperatorBuilder(toolbox, cstore, pstore), MHO_ChannelQuantity(){};

        virtual ~MHO_ChannelLabelerBuilder(){};

        /**
         * @brief Initializes and builds the channel labeler operator.
         * 
         * @return bool indicating success of initialization.
         */
        virtual bool Build() override;

    private:
};

} // namespace hops

#endif /*! end of include guard: MHO_ChannelLabelerBuilderBuilder_HH__ */
