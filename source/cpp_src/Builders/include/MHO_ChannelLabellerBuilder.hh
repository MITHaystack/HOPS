#ifndef MHO_ChannelLabellerBuilderBuilder_HH__
#define MHO_ChannelLabellerBuilderBuilder_HH__

/*
*File: MHO_ChannelLabellerBuilder.hh
*Class: MHO_ChannelLabellerBuilder
*Author:
*Email: 
*Date:
*Description:
*/

#include <vector>
#include <map>

#include "MHO_Tokenizer.hh"
#include "MHO_OperatorBuilder.hh"

namespace hops
{

class MHO_ChannelLabellerBuilder: public MHO_OperatorBuilder
{
    public:
        MHO_ChannelLabellerBuilder(): MHO_OperatorBuilder(){};
        virtual ~MHO_ChannelLabellerBuilder(){};

        virtual std::pair<std::string, MHO_Operator*> Build() override;

    private:

};

}//end namespace


#endif /* end of include guard: MHO_ChannelLabellerBuilderBuilder_HH__ */
