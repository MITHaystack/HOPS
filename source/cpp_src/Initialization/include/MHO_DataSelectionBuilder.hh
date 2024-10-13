#ifndef MHO_DataSelectionBuilderBuilder_HH__
#define MHO_DataSelectionBuilderBuilder_HH__

#include "MHO_OperatorBuilder.hh"

#include "MHO_Tokenizer.hh"

namespace hops
{

/*!
 *@file MHO_DataSelectionBuilder.hh
 *@class MHO_DataSelectionBuilder
 *@author J. Barrett - barrettj@mit.edu
 *@date Fri Jun 9 15:01:13 2023 -0400
 *@brief
 */

class MHO_DataSelectionBuilder: public MHO_OperatorBuilder
{
    public:
        MHO_DataSelectionBuilder(MHO_OperatorToolbox* toolbox, MHO_ContainerStore* cstore = nullptr,
                                 MHO_ParameterStore* pstore = nullptr)
            : MHO_OperatorBuilder(toolbox, cstore, pstore){};

        virtual ~MHO_DataSelectionBuilder(){};

        virtual bool Build() override;

    private:
        MHO_Tokenizer fTokenizer;
};

} // namespace hops

#endif /*! end of include guard: MHO_DataSelectionBuilderBuilder_HH__ */
