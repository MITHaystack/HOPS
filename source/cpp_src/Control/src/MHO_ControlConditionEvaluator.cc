#include "MHO_ControlConditionEvaluator.hh"

#include <iostream>

#define ERROR_STATE -1
#define FALSE_STATE 0
#define TRUE_STATE 1
#define OPEN_PAR 2
#define CLOSED_PAR 3
#define NOT_OP 4
#define AND_OP 5
#define OR_OP 6

namespace hops
{

MHO_ControlConditionEvaluator::MHO_ControlConditionEvaluator()
{
    fDelim = "-";
    fWildcard = "?";
    fBaselineMk4 = "??";
    fRefStationMk4ID = "?";
    fRemStationMk4ID = "?";
    fCanonicalRefStation = "";
    fCanonicalRemStation = "";
    fSource = "?";
    fFGroup = "?";
    fScanTime = "?";
}

MHO_ControlConditionEvaluator::~MHO_ControlConditionEvaluator()
{}

void MHO_ControlConditionEvaluator::SetPassInformation(std::string baseline, std::string source, std::string fgroup,
                                                       std::string scan_time)
{
    if(baseline.size() == 2) //mk4 style baseline 'GE'
    {
        fBaselineMk4 = baseline;
        fRefStationMk4ID = std::string(1, fBaselineMk4[0]);
        fRemStationMk4ID = std::string(1, fBaselineMk4[1]);
        fCanonicalRefStation = MHO_StationIdentifier::GetInstance()->CanonicalStationName(fRefStationMk4ID);
        fCanonicalRemStation = MHO_StationIdentifier::GetInstance()->CanonicalStationName(fRemStationMk4ID);
    }
    else if( baseline.find(fDelim) != std::string::npos ) //baseline is of the form: 'Gs-Wf'
    {
        std::string ref_station = baseline.substr(0,baseline.find(fDelim));
        std::string rem_station = baseline.substr(baseline.find(fDelim)+1);
        fCanonicalRefStation = MHO_StationIdentifier::GetInstance()->CanonicalStationName(ref_station);
        fCanonicalRemStation = MHO_StationIdentifier::GetInstance()->CanonicalStationName(rem_station);
        fRefStationMk4ID = MHO_StationIdentifier::GetInstance()->StationMk4IDFromName(fCanonicalRefStation);
        fRemStationMk4ID = MHO_StationIdentifier::GetInstance()->StationMk4IDFromName(fCanonicalRemStation);
        fBaselineMk4 = fRefStationMk4ID + fRemStationMk4ID;
    }

    msg_debug("control", "condition evaluator, canonical station names: "<< fCanonicalRefStation <<", "<< fCanonicalRemStation << eom);
    
    fSource = source;
    fFGroup = fgroup;
    fScanTime = scan_time;
}

mho_json MHO_ControlConditionEvaluator::GetApplicableStatements(mho_json& control_contents)
{
    mho_json control_statements;
    for(auto it = control_contents["conditions"].begin(); it != control_contents["conditions"].end(); it++)
    {
        if(it->find("statement_type") != it->end())
        {
            if(!((*it)["statement_type"].is_null()) && (*it)["statement_type"].get< std::string >() == "conditional")
            {
                if(Evaluate(*it))
                {
                    control_statements.push_back(*it);
                    // for(auto st = (*it)["statements"].begin(); st != (*it)["statements"].end(); st++)
                    // {
                    //     control_statements.push_back(*st);
                    // }
                    msg_debug("control", "statement is true: " << (*it)["value"] << eom);
                }
                else
                {
                    msg_debug("control", "statement is false: " << (*it)["value"] << eom);
                }
            }
        }
    }
    return control_statements;
}

bool MHO_ControlConditionEvaluator::Evaluate(mho_json& control_condition)
{
    bool condition_value = false;

    //open parentheses is +1, closed is -1, if we ever go negative or finish with a non-zero value
    //then we have and unmatched parentheses error somewhere.
    int paren_count = 0;

    if(control_condition.find("statement_type") != control_condition.end())
    {
        if(!(control_condition["statement_type"].is_null()))
        {
            if(control_condition["statement_type"].get< std::string >() == "conditional")
            {
                fStartLineNumber = control_condition["line_number"];
                std::vector< std::string > tokens = control_condition["value"];

                //std::cout<<"evaluating: " << control_condition["value"] << std::endl;

                std::stack< int > eval_stack;
                auto it = tokens.begin();
                auto it_end = tokens.end();
                while(it != tokens.end())
                {
                    //std::cout<<"token: "<<*it<<std::endl;
                    eval_stack.push(ProcessToken(it, it_end));
                    if(eval_stack.top() == ERROR_STATE)
                    {
                        msg_fatal("control", "error parsing token: '" << *it << "' on line: " << fStartLineNumber << eom);
                        std::exit(1);
                    }

                    //std::cout<<"top value = "<<eval_stack.top()<<std::endl;
                    if(eval_stack.top() == OPEN_PAR)
                    {
                        paren_count++;
                    }
                    if(eval_stack.top() == CLOSED_PAR)
                    {
                        paren_count--;
                    }

                    if(paren_count < 0)
                    {
                        msg_fatal("control", "unmatched parentheses while parsing if statement starting on line "
                                                 << fStartLineNumber << "." << eom);
                        std::exit(1);
                    }

                    if(eval_stack.top() == CLOSED_PAR)
                    {
                        //pop off everything until we get back to the open parentheses
                        //then evaluate it, and push the result back onto the stack
                        eval_stack.pop();
                        paren_count++;
                        std::list< int > tmp;
                        while(eval_stack.top() != OPEN_PAR && eval_stack.size() != 0)
                        {
                            tmp.push_back(eval_stack.top());
                            eval_stack.pop();
                        }
                        if(eval_stack.top() == OPEN_PAR)
                        {
                            eval_stack.pop();
                            paren_count--;
                        }
                        int result = EvaluateBooleanOps(tmp);
                        eval_stack.push(result);
                    }
                    it++;
                }

                if(paren_count != 0)
                {
                    msg_fatal("control", "unmatched parentheses while parsing if statement starting on line "
                                             << fStartLineNumber << "." << eom);
                    std::exit(1);
                }

                //evalute the stack
                //pop off everything until we get back to the open parentheses
                //then evaluate it, and push the result back onto the stack

                std::list< int > tmp;
                while(eval_stack.size() != 0)
                {
                    tmp.push_front(eval_stack.top());
                    eval_stack.pop();
                }
                condition_value = EvaluateBooleanOps(tmp);
            }
        }
    }
    return condition_value;
}

int MHO_ControlConditionEvaluator::ProcessToken(token_iter& it, token_iter& it_end)
{
    if(*it == "true")
    {
        return TRUE_STATE;
    }
    if(*it == "false")
    {
        return FALSE_STATE;
    }
    if(*it == "(")
    {
        return OPEN_PAR;
    }
    if(*it == ")")
    {
        return CLOSED_PAR;
    }
    if(*it == "and")
    {
        return AND_OP;
    }
    if(*it == "not")
    {
        return NOT_OP;
    }
    if(*it == "or")
    {
        return OR_OP;
    }

    if(std::next(it) != it_end)
    {
        if(*it == "station")
        {
            return EvaluateStation(++it);
        } //++it because we must consume the next token
        if(*it == "baseline")
        {
            return EvaluateBaseline(++it);
        }
        if(*it == "source")
        {
            return EvaluateSource(++it);
        }
        if(*it == "f_group")
        {
            return EvaluateFrequencyGroup(++it);
        }
        if(*it == "scan")
        {
            return EvaluateScan(++it, it_end);
        }
    }
    return ERROR_STATE;
}

int MHO_ControlConditionEvaluator::EvaluateBooleanOps(std::list< int > states)
{
    //states vector consists of NOT, AND, OR and TRUE/FALSE values
    //order of precedence is NOT, then AND, then OR

    int not_count = 0;
    int and_count = 0;
    int or_count = 0;

    do
    {
        not_count = 0;
        and_count = 0;
        or_count = 0;

        //std::cout<<"printing states:"<<std::endl;
        //for(auto s = states.begin(); s != states.end(); s++){std::cout<< *s <<std::endl;}
        //std::cout<<"------"<<std::endl;

        //first loop over list evaluating NOTs
        for(auto it = states.begin(); it != states.end(); it++)
        {
            if(*it == NOT_OP)
            {
                not_count++;
                it = states.erase(it);
                if(it != states.end())
                {
                    if(*it == TRUE_STATE)
                    {
                        *it = FALSE_STATE;
                    }
                    if(*it == FALSE_STATE)
                    {
                        *it = TRUE_STATE;
                    }
                }
            }
        }

        //now loop over the list evaluating the ANDs
        for(auto it = states.begin(); it != states.end(); it++)
        {
            if(*it == AND_OP)
            {
                and_count++;

                if(it == states.begin())
                {
                    msg_fatal("control",
                              "cannot parse 'and' condition with missing first argument for if statement starting on line "
                                  << fStartLineNumber << "." << eom);
                    std::exit(1);
                }

                auto first_arg_it = std::prev(it);
                auto second_arg_it = std::next(it);

                if(second_arg_it == states.end()) //can't have an and statment start the line
                {
                    msg_fatal("control",
                              "cannot parse 'and' condition with missing second argument for if statement starting on line "
                                  << fStartLineNumber << "." << eom);
                    std::exit(1);
                }

                int val1 = *first_arg_it;
                int val2 = *second_arg_it;
                int result = FALSE_STATE;
                if(val1 == TRUE_STATE && val2 == TRUE_STATE)
                {
                    result = TRUE_STATE;
                }
                *first_arg_it = result; //set the first argument equal to result
                it = states.erase(it);  //erase AND_OP
                it = states.erase(it);  //erase second arg
            }
        }

        //now loop over the list evaluating any ORs
        for(auto it = states.begin(); it != states.end(); it++)
        {
            if(*it == OR_OP)
            {
                or_count++;

                if(it == states.begin())
                {
                    msg_fatal("control",
                              "cannot parse 'or' condition with missing first argument for if statement starting on line "
                                  << fStartLineNumber << "." << eom);
                    std::exit(1);
                }

                auto first_arg_it = std::prev(it);
                auto second_arg_it = std::next(it);

                if(second_arg_it == states.end()) //can't have an and statment start the line
                {
                    msg_fatal("control",
                              "cannot parse 'or' condition with missing second argument for if statement starting on line "
                                  << fStartLineNumber << "." << eom);
                    std::exit(1);
                }

                int val1 = *first_arg_it;
                int val2 = *second_arg_it;
                int result = TRUE_STATE;
                if(val1 == FALSE_STATE && val2 == FALSE_STATE)
                {
                    result = FALSE_STATE;
                }
                *first_arg_it = result; //set the first argument equal to result
                it = states.erase(it);  //erase OR_OP
                it = states.erase(it);  //erase second arg
            }
        }
    }
    while(not_count || and_count || or_count); //keep looping until we have evaluated all of (not,and,or)

    //if we have more than one boolean state at the end, we by default and them all together
    //this could happen if someone wrote: "if (station A) (station B)", and failed to put an explicit 'and' in the middle
    if(states.size() > 1)
    {
        msg_warn("control",
                 "ambiguous boolean statement on line " << fStartLineNumber << ", assuming implied 'AND' condition" << eom);
        for(auto it = states.begin(); it != states.end(); it++)
        {
            if(*it == FALSE_STATE)
            {
                return FALSE_STATE;
            }
        }
        return TRUE_STATE;
    }
    else if(states.size() == 1)
    {
        return states.front();
    }
    else
    {
        //we have an error
        msg_error("control", "error parsing if statement on line " << fStartLineNumber << ", assuming false." << eom);
        return FALSE_STATE;
    }
}

int MHO_ControlConditionEvaluator::EvaluateStation(token_iter& it)
{
    //if( it == tokens.end() ){msg_error("control", "missing argument to station statement." << eom); return FALSE_STATE;}
    auto station = *it;
    if(station.size() == 1 ) //1-char mk4 id station
    {
        if(station == fRefStationMk4ID || station == fRemStationMk4ID)
        {
            return TRUE_STATE;
        }
        if(station == fWildcard)
        {
            return TRUE_STATE;
        }
    }
    else //multi-character station name/code
    {
        std::string station_name = MHO_StationIdentifier::GetInstance()->CanonicalStationName(station);
        if(station_name == fCanonicalRefStation || station_name == fCanonicalRemStation)
        {
            return TRUE_STATE;
        }
        if(station_name == fWildcard)
        {
            return TRUE_STATE;
        }
    }
    
    return FALSE_STATE;
}

int MHO_ControlConditionEvaluator::EvaluateBaseline(token_iter& it)
{
    auto baseline = *it;
    if(baseline.size() < 2){return FALSE_STATE;}
    else if(baseline.size() == 2)
    {
        // mk4 style baseline, e.g. two conjoined mk4ids: GE
        return EvaluateTwoCharacterBaseline(it);
    }
    else
    {
        //e.g. Gs-Wf
        return EvaluateMultiCharacterBaseline(it);
    }
}


int MHO_ControlConditionEvaluator::EvaluateTwoCharacterBaseline(token_iter& it)
{
    //if( it == tokens.end() ){msg_error("control", "missing argument to baseline statement." << eom); return FALSE_STATE;}
    auto baseline = *it;
    std::string ref_station(1, baseline[0]);
    std::string rem_station(1, baseline[1]);
    if(baseline == fBaselineMk4)
    {
        return TRUE_STATE;
    }
    if(ref_station == fRefStationMk4ID && rem_station == fRemStationMk4ID)
    {
        return TRUE_STATE;
    }
    if(ref_station == fWildcard && rem_station == fRemStationMk4ID)
    {
        return TRUE_STATE;
    }
    if(rem_station == fWildcard && ref_station == fRefStationMk4ID)
    {
        return TRUE_STATE;
    }
    if(rem_station == fWildcard && ref_station == fWildcard)
    {
        return TRUE_STATE;
    }
    return FALSE_STATE;
}


int MHO_ControlConditionEvaluator::EvaluateMultiCharacterBaseline(token_iter& it)
{
    auto baseline = *it;
    if(baseline.find(fDelim) != std::string::npos) //"-" must be present to split stations
    {
        std::string ref_station = baseline.substr(0,baseline.find(fDelim));
        std::string rem_station = baseline.substr(baseline.find(fDelim)+1);
        std::string ref_station_name = MHO_StationIdentifier::GetInstance()->CanonicalStationName(ref_station);
        std::string rem_station_name = MHO_StationIdentifier::GetInstance()->CanonicalStationName(rem_station);
        if(ref_station_name == fCanonicalRefStation && rem_station_name == fCanonicalRemStation)
        {
            return TRUE_STATE;
        }
        if(ref_station == fWildcard && rem_station_name == fCanonicalRemStation)
        {
            return TRUE_STATE;
        }
        if(rem_station == fWildcard && ref_station_name == fCanonicalRefStation)
        {
            return TRUE_STATE;
        }
        if(rem_station == fWildcard && ref_station == fWildcard)
        {
            return TRUE_STATE;
        }
        return FALSE_STATE;
    }
    return FALSE_STATE;
}


int MHO_ControlConditionEvaluator::EvaluateSource(token_iter& it)
{
    //if( it == tokens.end() ){msg_error("control", "missing argument to source statement." << eom); return FALSE_STATE;}
    auto src = *it;
    if(src == fWildcard)
    {
        return TRUE_STATE;
    }
    if(src == fSource)
    {
        return TRUE_STATE;
    }
    return FALSE_STATE;
}

int MHO_ControlConditionEvaluator::EvaluateFrequencyGroup(token_iter& it)
{
    //if( it == tokens.end() ){msg_error("control", "missing argument to f_group statement." << eom); return FALSE_STATE;}
    auto fgroup = *it;
    if(fgroup == fFGroup)
    {
        return TRUE_STATE;
    }
    if(fgroup == fWildcard)
    {
        return TRUE_STATE;
    }
    return FALSE_STATE;
}

int MHO_ControlConditionEvaluator::EvaluateScan(token_iter& it, token_iter& it_end)
{
    //if( it == tokens.end() ){msg_error("control", "missing argument to scan statement." << eom); return FALSE_STATE;}

    if(*it == "<")
    {
        ++it;
        std::string scan_value = *it;
        return ScanLessThan(scan_value);
    }
    else if(*it == ">")
    {
        ++it;
        std::string scan_value = *it;
        return ScanGreaterThan(scan_value);
    }
    else if(std::next(it) != it_end && *(std::next(it)) == "to")
    {
        //must be a range statement
        std::string scan_low = *it;
        ++it;
        std::string to_token = *it;
        ++it;
        std::string scan_high = *it;
        if(to_token == "to")
        {
            return ScanInBetween(scan_low, scan_high);
        }
        else
        {
            return FALSE_STATE;
        }
    }
    else
    {
        //single scan only
        if(*it == fScanTime)
        {
            return TRUE_STATE;
        }
    }

    return FALSE_STATE;
}

int MHO_ControlConditionEvaluator::ScanLessThan(std::string scan)
{
    if(fScanTime < scan)
    {
        return TRUE_STATE;
    }
    return FALSE_STATE;
}

int MHO_ControlConditionEvaluator::ScanGreaterThan(std::string scan)
{
    if(fScanTime > scan)
    {
        return TRUE_STATE;
    }
    return FALSE_STATE;
}

int MHO_ControlConditionEvaluator::ScanInBetween(std::string scan_low, std::string scan_high)
{
    if((scan_low < fScanTime) && (fScanTime < scan_high))
    {
        return TRUE_STATE;
    }
    return FALSE_STATE;
}

} // namespace hops
