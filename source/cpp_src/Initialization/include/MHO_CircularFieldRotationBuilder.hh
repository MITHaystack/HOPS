#ifndef MHO_CircularFieldRotationBuilderBuilder_HH__
#define MHO_CircularFieldRotationBuilderBuilder_HH__

#include "MHO_OperatorBuilder.hh"

namespace hops
{

/*!
 *@file MHO_CircularFieldRotationBuilder.hh
 *@class MHO_CircularFieldRotationBuilder
 *@author J. Barrett - barrettj@mit.edu
 *@date Mon Jan 15 11:23:36 2024 -0500
 *@brief builds a MHO_CircularFieldRotationCorrection operator (station mount types must be known)
 */

/**
 * @brief Class MHO_CircularFieldRotationBuilder
 */
class MHO_CircularFieldRotationBuilder: public MHO_OperatorBuilder
{
    public:
        MHO_CircularFieldRotationBuilder(MHO_OperatorToolbox* toolbox, MHO_FringeData* fdata)
            : MHO_OperatorBuilder(toolbox, fdata){};

        MHO_CircularFieldRotationBuilder(MHO_OperatorToolbox* toolbox, MHO_ContainerStore* cstore = nullptr,
                                         MHO_ParameterStore* pstore = nullptr)
            : MHO_OperatorBuilder(toolbox, cstore, pstore){};

        virtual ~MHO_CircularFieldRotationBuilder(){};

        /**
         * @brief Constructs and adds a new CircularFieldRotationBuilder to the toolbox's multimap.
         * 
         * @return No return value (void)
         */
        virtual bool Build() override;

    private:
};

} // namespace hops

#endif /*! end of include guard: MHO_CircularFieldRotationBuilderBuilder_HH__ */
