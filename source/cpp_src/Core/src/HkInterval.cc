#include "HkInterval.hh"

namespace hops
{

HkInterval::HkInterval():
    fLowerBound(0),
    fUpperBound(0)
{};

HkInterval::HkInterval(std::size_t lower_bound, std::size_t upper_bound)
{
    SetIntervalImpl(lower_bound,upper_bound);
};

HkInterval::HkInterval(const HkInterval& copy)
{
    fLowerBound = copy.fLowerBound;
    fUpperBound = copy.fUpperBound;
}

HkInterval::~HkInterval(){};

void
HkInterval::SetInterval(std::size_t lower_bound, std::size_t upper_bound)
{
    SetIntervalImpl(lower_bound,upper_bound);
}

void
HkInterval::SetInterval(std::pair<std::size_t,std::size_t> lower_upper)
{
    SetIntervalImpl(lower_upper.first, lower_upper.second);
}

std::pair<std::size_t,std::size_t>
HkInterval::GetInterval() const
{
    return std::pair<std::size_t,std::size_t>(fLowerBound, fUpperBound);
}

void
HkInterval::SetLowerBound(std::size_t low) { SetIntervalImpl(low, fUpperBound); }

void
HkInterval::SetUpperBound(std::size_t up) { SetIntervalImpl(fLowerBound, up); }

std::size_t
HkInterval::GetLowerBound() const {return fLowerBound;}

std::size_t
HkInterval::GetUpperBound() const {return fUpperBound;}

void
HkInterval::SetIntervalImpl(std::size_t low, std::size_t up)
{
    if(low < up)
    {
        fLowerBound = low;
        fUpperBound = up;
    }
    else if (up < low)
    {
        fLowerBound = up;
        fUpperBound = low;
    }
    else if (low == up)
    {
        fLowerBound = low;
        fUpperBound = up;
    }
}


}//end of namespace
