#include "MHO_ControlConditionEvaluator.hh"

#include <iostream>

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
    fWildcard = "?";
    fBaseline = "??";
    fRefStation = "?";
    fRemStation = "?";
    fSource = "?";
    fFGroup = "?";
    fScanTime = "?";
}

MHO_ControlConditionEvaluator::~MHO_ControlConditionEvaluator()
{

}

void
MHO_ControlConditionEvaluator::SetPassInformation(std::string baseline, std::string source, std::string fgroup, std::string scan_time)
{
    fBaseline = baseline;
    fRefStation = std::string(1,fBaseline[0]);
    fRemStation = std::string(1,fBaseline[1]);
    fSource = source;
    fFGroup = fgroup;
    fScanTime = scan_time;
}



bool
MHO_ControlConditionEvaluator::Evaluate(mho_json& control_condition)
{
    bool condition_value = false;

    //open parentheses is +1, closed is -1, if we ever go negative or finish with a non-zero value
    //then we have and unmatched parentheses error somewhere.
    int paren_count = 0;

    if( control_condition.find("statement_type") != control_condition.end() )
    {
        if( !( control_condition["statement_type"].is_null() ) )
        {
            if(control_condition["statement_type"].get<std::string>() == "conditional")
            {
                fStartLineNumber = control_condition["line_number"];
                std::vector< std::string > tokens = control_condition["value"];

                std::cout<<" Evaluating: " << control_condition["value"] << std::endl;

                std::stack< int > eval_stack;
                auto it = tokens.begin();
                auto it_end = tokens.end();
                while( it != tokens.end() )
                {
                    std::cout<<"token: "<<*it<<std::endl;
                    eval_stack.push( ProcessToken(it, it_end) );
                    std::cout<<"top value = "<<eval_stack.top()<<std::endl;
                    if(eval_stack.top() == OPEN_PAR){paren_count++;}
                    if(eval_stack.top() == CLOSED_PAR){paren_count--;}

                    if(paren_count < 0)
                    {
                        msg_fatal("control", "unmatched parentheses while parsing if statement starting on line "<< fStartLineNumber << "." << eom );
                        std::exit(1);
                    }

                    if( eval_stack.top() == CLOSED_PAR)
                    {
                        //pop off everything until we get back to the open parentheses
                        //then evaluate it, and push the result back onto the stack
                        eval_stack.pop(); paren_count++;
                        std::list< int > tmp;
                        while(eval_stack.top() != OPEN_PAR && eval_stack.size() != 0)
                        {
                            tmp.push_back( eval_stack.top() );
                            eval_stack.pop();
                        }
                        if( eval_stack.top() == OPEN_PAR){ eval_stack.pop(); paren_count--;}
                        int result = EvaluateBooleanOps(tmp);
                        eval_stack.push(result);
                    }
                    it++;
                }

                if(paren_count != 0)
                {
                    msg_fatal("control", "unmatched parentheses while parsing if statement starting on line "<< fStartLineNumber << "." << eom );
                    std::exit(1);
                }

                //evalute the stack
                //pop off everything until we get back to the open parentheses
                //then evaluate it, and push the result back onto the stack

                std::list< int > tmp;
                while(eval_stack.size() != 0)
                {
                    tmp.push_front( eval_stack.top() );
                    eval_stack.pop();
                }
                condition_value = EvaluateBooleanOps(tmp);
            }
        }
    }
    return condition_value;
}

int
MHO_ControlConditionEvaluator::ProcessToken(token_iter& it, token_iter& it_end)
{
    if( *it == "true" ){return TRUE_STATE;}
    if( *it == "false" ){return FALSE_STATE;}
    if( *it == "(" ){return OPEN_PAR;}
    if( *it == ")" ){return CLOSED_PAR;}
    if( *it == "and" ){return AND_OP;}
    if( *it == "not" ){return NOT_OP;}
    if( *it == "or" ){return OR_OP;}

    if( std::next(it) != it_end )
    {
        if( *it == "station"){return EvaluateStation(++it);} //++it because we must consume the next token
        if( *it == "baseline"){return EvaluateBaseline(++it);}
        if( *it == "source"){return EvaluateSource(++it);}
        if( *it == "f_group"){return EvaluateFrequencyGroup(++it);}
        if( *it == "scan"){return EvaluateScan(++it, it_end);}
    }
    return FALSE_STATE;
}



int
MHO_ControlConditionEvaluator::EvaluateBooleanOps(std::list< int > states)
{
    //states vector consistes of NOT, AND, OR and TRUE/FALSE values
    //order of precedence is NOT, then AND, then OR

    int not_count = 0;
    int and_count = 0;
    int or_count = 0;

    do
    {
        not_count = 0;
        and_count = 0;
        or_count = 0;

        std::cout<<"printing states:"<<std::endl;
        for(auto s = states.begin(); s != states.end(); s++){std::cout<< *s <<std::endl;}
        std::cout<<"------"<<std::endl;

        //first loop over list evaluating NOTs
        for(auto it = states.begin(); it != states.end(); it++)
        {
            if(*it == NOT_OP)
            {
                not_count++;
                it = states.erase(it);
                if(it != states.end() )
                {
                    if(*it == TRUE_STATE){*it = FALSE_STATE;}
                    if(*it == FALSE_STATE){*it = TRUE_STATE;}
                }
            }
        }

        //now loop over the list evaluating the ANDs
        for(auto it = states.begin(); it != states.end(); it++)
        {
            if(*it == AND_OP)
            {
                and_count++;

                if(it == states.begin() )
                {
                    msg_fatal("control", "cannot parse 'and' condition with missing first argument for if statement starting on line "<< fStartLineNumber << "." << eom );
                    std::exit(1);
                }

                auto first_arg_it = std::prev(it);
                auto second_arg_it = std::next(it);

                if(second_arg_it == states.end() ) //can't have an and statment start the line
                {
                    msg_fatal("control", "cannot parse 'and' condition with missing second argument for if statement starting on line "<< fStartLineNumber << "." << eom );
                    std::exit(1);
                }

                int val1 = *first_arg_it;
                int val2 = *second_arg_it;
                int result = FALSE_STATE;
                if( val1 == TRUE_STATE && val2 == TRUE_STATE){result = TRUE_STATE;}
                *first_arg_it = result; //set the first argument equal to result
                it = states.erase(it); //erase AND_OP
                it = states.erase(it); //erase second arg
            }
        }


        //now loop over the list evaluating any ORs
        for(auto it = states.begin(); it != states.end(); it++)
        {
            if(*it == OR_OP)
            {
                or_count++;

                if(it == states.begin() )
                {
                    msg_fatal("control", "cannot parse 'or' condition with missing first argument for if statement starting on line "<< fStartLineNumber << "." << eom );
                    std::exit(1);
                }

                auto first_arg_it = std::prev(it);
                auto second_arg_it = std::next(it);

                if(second_arg_it == states.end() ) //can't have an and statment start the line
                {
                    msg_fatal("control", "cannot parse 'or' condition with missing second argument for if statement starting on line "<< fStartLineNumber << "." << eom );
                    std::exit(1);
                }

                int val1 = *first_arg_it;
                int val2 = *second_arg_it;
                int result = TRUE_STATE;
                if( val1 == FALSE_STATE && val2 == FALSE_STATE){result = FALSE_STATE;}
                *first_arg_it = result; //set the first argument equal to result
                it = states.erase(it); //erase OR_OP
                it = states.erase(it); //erase second arg
            }
        }

    }
    while( not_count || and_count || or_count); //keep looping until we have evaluated all of (not,and,or)

    //if we have more than one boolean state at the end, we by default and them all together
    //this could happen if someone wrote: "if (station A) (station B)", and failed to put an explicit 'and' in the middle
    if(states.size() > 1)
    {
        for(auto it = states.begin(); it != states.end(); it++){if( *it == FALSE_STATE){return FALSE_STATE;} }
        return TRUE_STATE;
    }
    else if(states.size() == 1)
    {
        return states.front();
    }
    else
    {
        //we have an error
        msg_error("control", "error parsing if statement on line "<< fStartLineNumber << ", assuming false." << eom );
        return FALSE_STATE;
    }

}


int
MHO_ControlConditionEvaluator::EvaluateStation(token_iter& it)
{
    //if( it == tokens.end() ){msg_error("control", "missing argument to station statement." << eom); return FALSE_STATE;}
    auto station = *it;
    if(station == fRefStation || station == fRemStation){return TRUE_STATE;}
    if(station == fWildcard){return TRUE_STATE;}
    return FALSE_STATE;
}

int
MHO_ControlConditionEvaluator::EvaluateBaseline(token_iter& it)
{
    //if( it == tokens.end() ){msg_error("control", "missing argument to baseline statement." << eom); return FALSE_STATE;}
    auto baseline = *it;
    std::string ref_station(1,baseline[0]);
    std::string rem_station(1,baseline[1]);
    if(baseline == fBaseline){return TRUE_STATE;}
    if(ref_station == fWildcard && rem_station == fRemStation){return TRUE_STATE;}
    if(rem_station == fWildcard && ref_station == fRefStation){return TRUE_STATE;}
    if(rem_station == fWildcard && ref_station == fWildcard){return TRUE_STATE;}
    return FALSE_STATE;
}

int
MHO_ControlConditionEvaluator::EvaluateSource(token_iter& it)
{
    //if( it == tokens.end() ){msg_error("control", "missing argument to source statement." << eom); return FALSE_STATE;}
    auto src = *it;
    if(src == fWildcard){return TRUE_STATE;}
    if(src == fSource){return TRUE_STATE;}
    return FALSE_STATE;
}

int
MHO_ControlConditionEvaluator::EvaluateFrequencyGroup(token_iter& it)
{
    //if( it == tokens.end() ){msg_error("control", "missing argument to f_group statement." << eom); return FALSE_STATE;}
    auto fgroup = *it;
    if(fgroup == fFGroup){return TRUE_STATE;}
    if(fgroup == fWildcard){return TRUE_STATE;}
    return FALSE_STATE;
}

int
MHO_ControlConditionEvaluator::EvaluateScan(token_iter& it, token_iter& it_end)
{
    //if( it == tokens.end() ){msg_error("control", "missing argument to scan statement." << eom); return FALSE_STATE;}

    if( *it == "<" )
    {
        ++it;
        std::string scan_value = *it;
        return ScanLessThan(scan_value);
    }
    else if( *it  == ">" )
    {
        ++it;
        std::string scan_value = *it;
        return ScanGreaterThan(scan_value);
    }
    else if( std::next(it) != it_end  && *( std::next(it) ) == "to" )
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
        else{return FALSE_STATE;}
    }
    else
    {
        //single scan only
        if(*it == fScanTime){return TRUE_STATE;}
    }

    return FALSE_STATE;
}




int
MHO_ControlConditionEvaluator::ScanLessThan(std::string scan)
{
    if( fScanTime < scan ){return TRUE_STATE;}
    return FALSE_STATE;
}

int
MHO_ControlConditionEvaluator::ScanGreaterThan(std::string scan)
{
    if( fScanTime > scan){return TRUE_STATE;}
    return FALSE_STATE;
}

int
MHO_ControlConditionEvaluator::ScanInBetween(std::string scan_low, std::string scan_high)
{
    if( ( scan_low < fScanTime ) && ( fScanTime < scan_high ) ){return TRUE_STATE;}
    return FALSE_STATE;
}



}
