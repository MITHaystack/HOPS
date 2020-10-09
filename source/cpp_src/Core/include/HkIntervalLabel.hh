#ifndef HkIntervalLabel_HH__
#define HkIntervalLabel_HH__

/*
*File: HkIntervalLabel.hh
*Class: HkIntervalLabel
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:
*/

#include <string>
#include <utility>

#include "HkInterval.hh"
#include "HkMultiTypeMap.hh"

namespace hops
{

//TODO: Make sure this set of types is complete for data-labelling.
//Consider what other types might be needed (float? short? dates?)
typedef HkMultiTypeMap< std::string, char, bool, int, double, std::string > HkIntervalLabelMap;

class HkIntervalLabel: public HkInterval< std::size_t >, public HkIntervalLabelMap
{
    public:

        HkIntervalLabel();
        HkIntervalLabel( std::size_t lower_bound, std::size_t upper_bound);
        HkIntervalLabel(const HkIntervalLabel& copy);
        virtual ~HkIntervalLabel();

        HkIntervalLabel& operator=(const HkIntervalLabel& rhs)
        {
            if(this != &rhs)
            {
                SetIntervalImpl(rhs.fLowerBound, rhs.fUpperBound );
                this->CopyFrom<char>(rhs);
                this->CopyFrom<bool>(rhs);
                this->CopyFrom<int>(rhs);
                this->CopyFrom<double>(rhs);
            }
            return *this;
        }

    private:



};

}

#endif /* end of include guard: HkIntervalLabel */
