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

MHOIntervalLabel::MHOIntervalLabel(const MHOIntervalLabel& copy):
    MHOInterval(copy)
{
    this->CopyFrom<char>(copy);
    this->CopyFrom<bool>(copy);
    this->CopyFrom<int>(copy);
    this->CopyFrom<double>(copy);
}

MHOIntervalLabel::~MHOIntervalLabel(){};


}//end of namespace
