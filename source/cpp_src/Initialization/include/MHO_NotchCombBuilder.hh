#ifndef MHO_NotchCombBuilder_HH__
#define MHO_NotchCombBuilder_HH__

#include "MHO_OperatorBuilder.hh"

namespace hops
{

/*!
 *@file MHO_NotchCombBuilder.hh
 *@class MHO_NotchCombBuilder
 *@author J. Barrett - barrettj@mit.edu
 *@date Tue Jun 20 12:35:56 2023 -0400
 *@brief builds a notches (frequency cut) operator
 */

/**
 * @brief Class MHO_NotchCombBuilder
 */
class MHO_NotchCombBuilder: public MHO_OperatorBuilder
{
    public:
        MHO_NotchCombBuilder(MHO_OperatorToolbox* toolbox, MHO_FringeData* fdata): MHO_OperatorBuilder(toolbox, fdata){};

        MHO_NotchCombBuilder(MHO_OperatorToolbox* toolbox, MHO_ContainerStore* cstore = nullptr,
                           MHO_ParameterStore* pstore = nullptr)
            : MHO_OperatorBuilder(toolbox, cstore, pstore){};

        virtual ~MHO_NotchCombBuilder(){};

        /**
         * @brief Constructs and adds a new MHO_NotchComb operator to the toolbox
         *
         * @return True if successful, false otherwise.
         */
        virtual bool Build() override;

    private:
};

} // namespace hops

#endif /*! end of include guard: MHO_NotchCombBuilder_HH__ */
