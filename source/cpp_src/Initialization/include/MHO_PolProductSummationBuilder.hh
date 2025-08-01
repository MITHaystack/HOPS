#ifndef MHO_PolProductSummationBuilderBuilder_HH__
#define MHO_PolProductSummationBuilderBuilder_HH__

#include "MHO_OperatorBuilder.hh"

namespace hops
{

/*!
 *@file MHO_PolProductSummationBuilder.hh
 *@class MHO_PolProductSummationBuilder
 *@author J. Barrett - barrettj@mit.edu
 *@date Mon Jan 15 11:23:36 2024 -0500
 *@brief Builds an operator to perform a summation over specified pol-products (or pseudo-Stokes-I)
 */

/**
 * @brief Class MHO_PolProductSummationBuilder
 */
class MHO_PolProductSummationBuilder: public MHO_OperatorBuilder
{
    public:
        MHO_PolProductSummationBuilder(MHO_OperatorToolbox* toolbox, MHO_FringeData* fdata)
            : MHO_OperatorBuilder(toolbox, fdata){};

        MHO_PolProductSummationBuilder(MHO_OperatorToolbox* toolbox, MHO_ContainerStore* cstore = nullptr,
                                       MHO_ParameterStore* pstore = nullptr)
            : MHO_OperatorBuilder(toolbox, cstore, pstore){};

        virtual ~MHO_PolProductSummationBuilder(){};

        /**
         * @brief Constructs and initializes the PolProductSummation operator, and adds to toolbox
         *
         * @return True if successful, false otherwise.
         */
        virtual bool Build() override;

    private:
        // std::string ParsePolFromName(const std::string& name);
        // std::string ExtractStationMk4ID();
};

} // namespace hops

#endif /*! end of include guard: MHO_PolProductSummationBuilderBuilder_HH__ */
