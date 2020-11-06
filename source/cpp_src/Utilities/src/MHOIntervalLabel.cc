#include "MHOIntervalLabel.hh"


namespace hops
{


MHOIntervalLabel::MHOIntervalLabel():
    MHOInterval(),
    MHOIntervalLabelMap()
{}

MHOIntervalLabel::MHOIntervalLabel( std::size_t lower_bound, std::size_t upper_bound):
    MHOInterval(lower_bound,upper_bound),
    MHOIntervalLabelMap()
{};

bool
MHOIntervalLabel::HasKey(const std::string& key) const
{
    if(this->ContainsKey<char>(key)){return true;}
    if(this->ContainsKey<bool>(key)){return true;}
    if(this->ContainsKey<int>(key)){return true;}
    if(this->ContainsKey<double>(key)){return true;}
    if(this->ContainsKey<std::string>(key)){return true;}
    return false;
}

MHOIntervalLabel::MHOIntervalLabel(const MHOIntervalLabel& copy):
    MHOInterval(copy)
{
    this->CopyFrom<char>(copy);
    this->CopyFrom<bool>(copy);
    this->CopyFrom<int>(copy);
    this->CopyFrom<double>(copy);
    this->CopyFrom<std::string>(copy);
}

MHOIntervalLabel::~MHOIntervalLabel(){};


}//end of namespace
