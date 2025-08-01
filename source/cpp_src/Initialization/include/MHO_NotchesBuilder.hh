#ifndef MHO_NotchesBuilderBuilder_HH__
#define MHO_NotchesBuilderBuilder_HH__

#include "MHO_OperatorBuilder.hh"

namespace hops
{

/*!
 *@file MHO_NotchesBuilder.hh
 *@class MHO_NotchesBuilder
 *@author J. Barrett - barrettj@mit.edu
 *@date Tue Jun 20 12:35:56 2023 -0400
 *@brief builds a notches (frequency cut) operator
 */

/**
 * @brief Class MHO_NotchesBuilder
 */
class MHO_NotchesBuilder: public MHO_OperatorBuilder
{
    public:
        MHO_NotchesBuilder(MHO_OperatorToolbox* toolbox, MHO_FringeData* fdata): MHO_OperatorBuilder(toolbox, fdata){};

        MHO_NotchesBuilder(MHO_OperatorToolbox* toolbox, MHO_ContainerStore* cstore = nullptr,
                           MHO_ParameterStore* pstore = nullptr)
            : MHO_OperatorBuilder(toolbox, cstore, pstore){};

        virtual ~MHO_NotchesBuilder(){};

        /**
         * @brief Constructs and adds a new MHO_Notches operator to the toolbox
         *
         * @return True if successful, false otherwise.
         */
        virtual bool Build() override;

    private:
};

} // namespace hops

#endif /*! end of include guard: MHO_NotchesBuilderBuilder_HH__ */
