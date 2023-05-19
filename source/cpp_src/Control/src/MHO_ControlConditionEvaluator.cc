#include "MHO_ControlConditionEvaluator.hh"

#include <iostream>

namespace hops
{

MHO_ControlConditionEvaluator::MHO_ControlConditionEvaluator()
{

}

MHO_ControlConditionEvaluator::~MHO_ControlConditionEvaluator()
{

}

void
MHO_ControlConditionEvaluator::SetPassInformation(std::string baseline, std::string source, std::string fgroup, std::string scan_time)
{
    fBaseline = baseline;
    fSource = source;
    fFGroup = fgroup;
    fScanTime = scan_time;
}

bool
MHO_ControlConditionEvaluator::Evaluate(mho_json& control_condition)
{
    bool value = false;

    if( control_condition.find("statement_type") != control_condition.end() )
    {

        if( !( control_condition["statement_type"].is_null() ) )
        {
            if(control_condition["statement_type"].get<std::string>() == "conditional")
            {
                std::vector< std::string > tokens = control_condition["value"];

                for(std::size_t i=0; i < tokens.size() ; i++)
                {
                    std::cout<< tokens[i] << std::endl;
                 }

            }
        }

    }

        //                                                    /* compare baselines */
        // match[0] =  (base[0] == WILDCARD
        //          || cb_ptr->baseline[0] == WILDCARD
        //          || cb_ptr->baseline[0] == base[0])
        //          && (base[1] == WILDCARD
        //          ||  cb_ptr->baseline[1] == WILDCARD
        //          ||  cb_ptr->baseline[1] == base[1]);
        //
        //                                                      /* compare sources */
        // match[1] = TRUE;
        // for (i=0; i<16; i++)            // any different non-wildcard chars ruin match
        //     if (sour[i] != WILDCARD
        //         && sour[i] != ' '
        //         && sour[i] != '\0'
        //         && cb_ptr->source[i] != WILDCARD
        //         && sour[i] != cb_ptr->source[i])
        //         match[1] = FALSE;
        //
        //                                              /* compare frequency group */
        // match[2] =  group == WILDCARD
        //          || cb_ptr->f_group == WILDCARD
        //          || cb_ptr->f_group == group;
        //
        //
        //                                         /* and compare scan start times */
        // match[3] =  (cb_ptr->scan[0] == NULLINT
        //          ||  cb_ptr->scan[0] <= time)
        //          && (cb_ptr->scan[1] == NULLINT
        //          ||  cb_ptr->scan[1] >= time);
        //
        //
        //
        // all_match = TRUE;                        /* innocent 'til proven guilty */
        //
        //                                           /* mark false if any disagree */
        // for (i=0; i<4; i++)
        //     all_match &= (cb_ptr->knot[i] == FALSE && match[i] == TRUE )  ||
        //                  (cb_ptr->knot[i] == TRUE  && match[i] == FALSE);
        //


    return value;
}




}
