#ifndef MHO_PassbandBuilderBuilder_HH__
#define MHO_PassbandBuilderBuilder_HH__

#include "MHO_OperatorBuilder.hh"

namespace hops
{

/*!
 *@file MHO_PassbandBuilder.hh
 *@class MHO_PassbandBuilder
 *@author J. Barrett - barrettj@mit.edu
 *@date Tue Jun 20 12:35:56 2023 -0400
 *@brief Builds a passband (frequency chunk excision) operator
 */

/**
 * @brief Class MHO_PassbandBuilder
 */
class MHO_PassbandBuilder: public MHO_OperatorBuilder
{
    public:

        MHO_PassbandBuilder(MHO_OperatorToolbox* toolbox, MHO_FringeData* fdata)
            : MHO_OperatorBuilder(toolbox, fdata){};

        MHO_PassbandBuilder(MHO_OperatorToolbox* toolbox, MHO_ContainerStore* cstore = nullptr,
                            MHO_ParameterStore* pstore = nullptr)
            : MHO_OperatorBuilder(toolbox, cstore, pstore){};

        virtual ~MHO_PassbandBuilder(){};

        /**
         * @brief Constructs and initializes the passband operator and adds to the toolbox
         * 
         * @return bool indicating success/failure
         */
        virtual bool Build() override;

    private:
};

} // namespace hops

#endif /*! end of include guard: MHO_PassbandBuilderBuilder_HH__ */
