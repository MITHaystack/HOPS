#ifndef MHO_ManualChannelPhaseCorrectionBuilderBuilder_HH__
#define MHO_ManualChannelPhaseCorrectionBuilderBuilder_HH__

/*
*File: MHO_ManualChannelPhaseCorrectionBuilder.hh
*Class: MHO_ManualChannelPhaseCorrectionBuilder
*Author:
*Email:
*Date:
*Description:
*/

#include "MHO_OperatorBuilder.hh"

namespace hops
{

class MHO_ManualChannelPhaseCorrectionBuilder: public MHO_OperatorBuilder
{
    public:
        MHO_ManualChannelPhaseCorrectionBuilder(): MHO_OperatorBuilder(){};
        virtual ~MHO_ManualChannelPhaseCorrectionBuilder(){};

        virtual bool Build() override;

};

}//end namespace


#endif /* end of include guard: MHO_ManualChannelPhaseCorrectionBuilderBuilder_HH__ */
