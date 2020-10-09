#include "HkIntervalLabel.hh"


namespace hops
{


HkIntervalLabel::HkIntervalLabel():
    HkInterval(),
    HkIntervalLabelMap()
{}

HkIntervalLabel::HkIntervalLabel( std::size_t lower_bound, std::size_t upper_bound):
    HkInterval(lower_bound,upper_bound),
    HkIntervalLabelMap()
{};

HkIntervalLabel::HkIntervalLabel(const HkIntervalLabel& copy):
    HkInterval(copy)
{
    this->CopyFrom<char>(copy);
    this->CopyFrom<bool>(copy);
    this->CopyFrom<int>(copy);
    this->CopyFrom<double>(copy);
}

HkIntervalLabel::~HkIntervalLabel(){};


}
