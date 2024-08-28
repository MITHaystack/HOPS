#ifndef MHO_DCBlockBuilderBuilder_HH__
#define MHO_DCBlockBuilderBuilder_HH__


#include "MHO_OperatorBuilder.hh"

namespace hops
{

/*!
*@file MHO_DCBlockBuilder.hh
*@class MHO_DCBlockBuilder
*@author J. Barrett - barrettj@mit.edu
*@date Tue Jun 20 12:35:56 2023 -0400
*@brief
*/

class MHO_DCBlockBuilder:
    public MHO_OperatorBuilder
{
    public:

        MHO_DCBlockBuilder(MHO_OperatorToolbox* toolbox,
                            MHO_ContainerStore* cstore = nullptr,
                            MHO_ParameterStore* pstore = nullptr):
            MHO_OperatorBuilder(toolbox, cstore, pstore)
        {};

        virtual ~MHO_DCBlockBuilder(){};

        virtual bool Build() override;

    private:

};

}//end namespace


#endif /*! end of include guard: MHO_DCBlockBuilderBuilder_HH__ */
