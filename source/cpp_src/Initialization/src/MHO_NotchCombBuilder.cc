#include "MHO_NotchCombBuilder.hh"
#include "MHO_NotchComb.hh"

#include <memory>

#include "MHO_Meta.hh"
#include "MHO_Tokenizer.hh"

namespace hops
{

bool MHO_NotchCombBuilder::Build()
{
    if(IsConfigurationOk())
    {
        msg_debug("initialization", "building a notches operator." << eom);
        //assume attributes are ok for now - TODO add checks!

        std::string op_name = fAttributes["name"].get< std::string >();
        std::string op_category = fFormat["operator_category"].get< std::string >();
        double priority = fFormat["priority"].get< double >();

        //positive values only
        double offset = std::fabs( fAttributes["value"]["offset"].get< double >() ); 
        double period = std::fabs( fAttributes["value"]["period"].get< double >() );
        double width = std::fabs( fAttributes["value"]["width"].get< double >() );

        //retrieve the arguments to operate on from the container store
        visibility_type* vis_data = fContainerStore->GetObject< visibility_type >(std::string("vis"));
        if(vis_data == nullptr)
        {
            msg_error("initialization", "cannot construct MHO_NotchComb without visibility data." << eom);
            return false;
        }

        //retrieve the arguments to operate on from the container store
        weight_type* wt_data = fContainerStore->GetObject< weight_type >(std::string("weight"));
        if(wt_data == nullptr)
        {
            msg_error("initialization", "cannot construct MHO_NotchComb without weight data." << eom);
            return false;
        }

        if(width > period)
        {
            msg_error("initialization", "cannot construct MHO_NotchComb, width is larger than period." << eom);
            return false;
        }

        std::unique_ptr< MHO_NotchComb > op(new MHO_NotchComb());

        //set the arguments
        op->SetArgs(vis_data);
        op->SetWeights(wt_data);
        op->SetNotchOffset(offset);
        op->SetNotchPeriod(period);
        op->SetNotchWidth(width);
        op->SetName(op_name);
        op->SetPriority(priority);

        msg_debug("initialization", "creating operator: " << op_name <<
                  " with notch (offset, period, width) = ("<< offset <<", "<< period<<", "<<width<<")" << eom);

        bool replace_duplicates = false;
        this->fOperatorToolbox->AddOperator(std::move(op), op_name, op_category, replace_duplicates);
        return true;
    }
    return false;
}

} // namespace hops
