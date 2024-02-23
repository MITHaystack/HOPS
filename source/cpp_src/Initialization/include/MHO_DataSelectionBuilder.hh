#ifndef MHO_DataSelectionBuilderBuilder_HH__
#define MHO_DataSelectionBuilderBuilder_HH__

/*!
*@file MHO_DataSelectionBuilder.hh
*@class MHO_DataSelectionBuilder
*@author
*Email:
*@date
*@brief
*/

#include "MHO_OperatorBuilder.hh"

#include "MHO_Tokenizer.hh"

namespace hops
{

class MHO_DataSelectionBuilder:
    public MHO_OperatorBuilder
{
    public:

        MHO_DataSelectionBuilder(MHO_OperatorToolbox* toolbox,
                                 MHO_ContainerStore* cstore = nullptr,
                                 MHO_ParameterStore* pstore = nullptr):
            MHO_OperatorBuilder(toolbox, cstore, pstore)
        {};

        virtual ~MHO_DataSelectionBuilder(){};

        virtual bool Build() override;

    private:

        MHO_Tokenizer fTokenizer;

};

}//end namespace


#endif /*! end of include guard: MHO_DataSelectionBuilderBuilder_HH__ */
