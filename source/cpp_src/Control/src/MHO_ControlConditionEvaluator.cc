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

                std::vector< std::string > bl;
                std::vector< std::string > src;
                std::vector< std::string > fg;

                //need to parse the tokens, this could be complicated
                std::cout<<"---------------"<<std::endl;
                std::size_t max_idx = tokens.size() - 1;
                for(std::size_t i=0; i < tokens.size() ; i++)
                {
                    if(tokens[i] == "station" && i+1 <= max_idx )
                    {
                        std::string opt1 = tokens[i+1] + "?";
                        std::string opt2 = "?" + tokens[i+1];
                        bl.push_back(opt1);
                        bl.push_back(opt2);
                    }

                    if(tokens[i] == "baseline" && i+1 <= max_idx )
                    {
                        bl.push_back(tokens[i+1]);
                    }

                    if(tokens[i] == "source" && i+1 <= max_idx )
                    {
                        src.push_back(tokens[i+1]);
                    }

                    if(tokens[i] == "f_group" && i+1 <= max_idx )
                    {
                        fg.push_back(tokens[i+1]);
                    }

                    std::cout<< tokens[i] << std::endl;
                }

                std::cout<<"==============="<<std::endl;
                for(std::size_t i=0; i<bl.size(); i++)
                {
                    std::cout<<bl[i]<<std::endl;
                }
                for(std::size_t i=0; i<src.size(); i++)
                {
                    std::cout<<src[i]<<std::endl;
                }
                for(std::size_t i=0; i<fg.size(); i++)
                {
                    std::cout<<fg[i]<<std::endl;
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
