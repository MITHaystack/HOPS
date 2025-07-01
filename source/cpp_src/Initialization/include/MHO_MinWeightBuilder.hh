#ifndef MHO_MinWeightBuilderBuilder_HH__
#define MHO_MinWeightBuilderBuilder_HH__

#include "MHO_OperatorBuilder.hh"

namespace hops
{

/*!
 *@file MHO_MinWeightBuilder.hh
 *@class MHO_MinWeightBuilder
 *@author J. Barrett - barrettj@mit.edu
 *@date Tue Jun 20 12:35:56 2023 -0400
 *@brief
 */

/**
 * @brief Class MHO_MinWeightBuilder
 */
class MHO_MinWeightBuilder: public MHO_OperatorBuilder
{
    public:
        MHO_MinWeightBuilder(MHO_OperatorToolbox* toolbox, MHO_FringeData* fdata)
            : MHO_OperatorBuilder(toolbox, fdata){};

        MHO_MinWeightBuilder(MHO_OperatorToolbox* toolbox, MHO_ContainerStore* cstore = nullptr,
                             MHO_ParameterStore* pstore = nullptr)
            : MHO_OperatorBuilder(toolbox, cstore, pstore){};

        virtual ~MHO_MinWeightBuilder(){};

        /**
         * @brief Constructs and initializes the MinWeightBuilder object.
         * 
         * @return True if successful build, false otherwise.
         */
        virtual bool Build() override;

    private:
};

} // namespace hops

#endif /*! end of include guard: MHO_MinWeightBuilderBuilder_HH__ */
