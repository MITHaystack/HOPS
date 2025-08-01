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
 *@brief builds a LSB (lower side band) offset operator
 */

/**
 * @brief Class MHO_LSBOffsetBuilder
 */
class MHO_LSBOffsetBuilder: public MHO_OperatorBuilder
{
    public:
        MHO_LSBOffsetBuilder(MHO_OperatorToolbox* toolbox, MHO_FringeData* fdata): MHO_OperatorBuilder(toolbox, fdata){};

        MHO_LSBOffsetBuilder(MHO_OperatorToolbox* toolbox, MHO_ContainerStore* cstore = nullptr,
                             MHO_ParameterStore* pstore = nullptr)
            : MHO_OperatorBuilder(toolbox, cstore, pstore){};

        virtual ~MHO_LSBOffsetBuilder(){};

        /**
         * @brief Constructs and initializes a MHO_LSBOffset operator.
         *
         * @return True if successful, false otherwise.
         */
        virtual bool Build() override;

    private:
        /**
         * @brief Extracts and returns the station identifier from the conditions vector.
         *
         * @return The extracted station identifier as a string.
         */
        std::string ExtractStationIdentifier();
};

} // namespace hops

#endif /*! end of include guard: MHO_LSBOffsetBuilderBuilder_HH__ */
