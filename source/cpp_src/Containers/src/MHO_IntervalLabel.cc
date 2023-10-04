#include "MHO_IntervalLabel.hh"


namespace hops
{


MHO_IntervalLabel::MHO_IntervalLabel():
    MHO_Interval(),
    MHO_CommonLabelMap(),
    fIsValid(true)
{}

MHO_IntervalLabel::MHO_IntervalLabel( std::size_t lower_bound, std::size_t upper_bound):
    MHO_Interval(lower_bound,upper_bound),
    MHO_CommonLabelMap(),
    fIsValid(true)
{};

bool
MHO_IntervalLabel::HasKey(const std::string& key) const
{
    if(this->ContainsKey<char>(key)){return true;}
    if(this->ContainsKey<bool>(key)){return true;}
    if(this->ContainsKey<int>(key)){return true;}
    if(this->ContainsKey<double>(key)){return true;}
    if(this->ContainsKey<std::string>(key)){return true;}
    return false;
}

MHO_IntervalLabel::MHO_IntervalLabel(const MHO_IntervalLabel& copy):
    MHO_Interval(copy)
{
    this->CopyFrom<char>(copy);
    this->CopyFrom<bool>(copy);
    this->CopyFrom<int>(copy);
    this->CopyFrom<double>(copy);
    this->CopyFrom<std::string>(copy);
}

MHO_IntervalLabel::~MHO_IntervalLabel(){};


}//end of namespace
