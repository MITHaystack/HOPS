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
*@brief
*/


class MHO_CircularFieldRotationBuilder:
    public MHO_OperatorBuilder
{
    public:

        MHO_CircularFieldRotationBuilder(MHO_OperatorToolbox* toolbox,
                                                MHO_ContainerStore* cstore = nullptr,
                                                MHO_ParameterStore* pstore = nullptr):
            MHO_OperatorBuilder(toolbox, cstore, pstore)
        {};

        virtual ~MHO_CircularFieldRotationBuilder(){};

        virtual bool Build() override;

    private:

};

}//end namespace


#endif /*! end of include guard: MHO_CircularFieldRotationBuilderBuilder_HH__ */
